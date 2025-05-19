#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags() {
    auto* log = get_logger();
    log->log("allocator_boundary_tags::~allocator_boundary_tags() called", logger::severity::debug);
    if (!_trusted_memory) {
        log->log("Nothing to deallocate", logger::severity::error);
        return;
    }

    get_mutex().~mutex();

    try {
        auto* parent_allocator = *reinterpret_cast<std::pmr::memory_resource**>(
            reinterpret_cast<char*>(_trusted_memory) + sizeof(logger*)
        );
        size_t user_space = *reinterpret_cast<size_t*>(
            reinterpret_cast<char*>(_trusted_memory)
          + sizeof(logger*)
          + sizeof(std::pmr::memory_resource*)
          + sizeof(allocator_with_fit_mode::fit_mode)
        );
        size_t total_size = allocator_metadata_size + user_space;

        if (parent_allocator) {
            parent_allocator->deallocate(_trusted_memory, total_size);
            log->log("Memory deallocated via parent allocator", logger::severity::debug);
        } else {
            ::operator delete(_trusted_memory);
            log->log("Memory deallocated via global operator delete", logger::severity::debug);
        }

        _trusted_memory = nullptr;
    } catch (const std::exception& ex) {
        log->log(std::string("Error while deleting allocator: ") + ex.what(), logger::severity::error);
        _trusted_memory = nullptr;
    }

    log->log("allocator_boundary_tags::~allocator_boundary_tags() finished", logger::severity::debug);
}

allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags&& other) noexcept
    : _trusted_memory(other._trusted_memory)
{
    auto* log = get_logger();
    log->log("allocator_boundary_tags(move) constructor called", logger::severity::debug);

    std::lock_guard<std::mutex> lock(other.get_mutex());
    other._trusted_memory = nullptr;

    log->log("allocator_boundary_tags(move) constructor finished", logger::severity::debug);
}

allocator_boundary_tags& allocator_boundary_tags::operator=(allocator_boundary_tags&& other) noexcept {
    auto* log = get_logger();
    log->log("allocator_boundary_tags::operator=(move) called", logger::severity::debug);

    if (this != &other) {
        std::unique_lock<std::mutex> lock_this(get_mutex(), std::defer_lock);
        std::unique_lock<std::mutex> lock_other(other.get_mutex(), std::defer_lock);
        std::lock(lock_this, lock_other);

        if (_trusted_memory) {
            try {
                size_t user_space = *reinterpret_cast<size_t*>(
                    reinterpret_cast<unsigned char*>(_trusted_memory)
                  + sizeof(logger*)
                  + sizeof(std::pmr::memory_resource*)
                  + sizeof(allocator_with_fit_mode::fit_mode)
                );
                size_t total_size = allocator_metadata_size + user_space;
                auto* parent_alloc = *reinterpret_cast<std::pmr::memory_resource**>(
                    reinterpret_cast<char*>(_trusted_memory) + sizeof(logger*)
                );
                if (parent_alloc) {
                    parent_alloc->deallocate(_trusted_memory, total_size);
                } else {
                    ::operator delete(_trusted_memory);
                }
            } catch (const std::exception& e) {
                log->log(std::string("Error in operator= : ") + e.what(), logger::severity::error);
            }
        }

        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
        log->log("Resources moved in operator=", logger::severity::debug);
    }

    log->log("allocator_boundary_tags::operator=(move) finished", logger::severity::debug);
    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    std::pmr::memory_resource* parent_allocator,
    logger* logger_ptr,
    allocator_with_fit_mode::fit_mode fit_mode)
{
    if (logger_ptr)
        logger_ptr->log("allocator_boundary_tags constructor started", logger::severity::debug);

    if (space_size == 0) {
        if (logger_ptr)
            logger_ptr->log("Size must be more than zero", logger::severity::error);
        throw std::invalid_argument("Size must be more than zero.");
    }

    try {

        parent_allocator = parent_allocator ? parent_allocator : std::pmr::get_default_resource();
        size_t total_size = allocator_metadata_size + space_size;
        if (logger_ptr) {
            logger_ptr->log("Allocator requires " + std::to_string(total_size) + " bytes", logger::severity::debug);
            logger_ptr->log("Available " + std::to_string(space_size) + " bytes", logger::severity::information);
        }

        _trusted_memory = parent_allocator->allocate(total_size);
        auto* mem = reinterpret_cast<unsigned char*>(_trusted_memory);
        *reinterpret_cast<logger**>(mem) = logger_ptr;
        mem += sizeof(logger*);
        *reinterpret_cast<std::pmr::memory_resource**>(mem) = parent_allocator;
        mem += sizeof(std::pmr::memory_resource*);

        *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(mem) = fit_mode;
        mem += sizeof(allocator_with_fit_mode::fit_mode);
        *reinterpret_cast<size_t*>(mem) = space_size;
        mem += sizeof(size_t);
        new (reinterpret_cast<std::mutex*>(mem)) std::mutex();
        mem += sizeof(std::mutex);
        *reinterpret_cast<void**>(mem) = nullptr;

    } catch (const std::exception& e) {
        if (logger_ptr)
            logger_ptr->log("Initiation of allocator failed: " + std::string(e.what()), logger::severity::error);
        throw std::iostream::failure("Initiation of allocator failed.");
    }

    get_logger()->log("allocator_boundary_tags constructor finished", logger::severity::debug);
}

[[nodiscard]] void* allocator_boundary_tags::do_allocate_sm(size_t bytes) {
    auto* log = get_logger();
    log->log("do_allocate_sm() called", logger::severity::debug);

    size_t total = bytes + occupied_block_metadata_size;
    size_t space = *reinterpret_cast<size_t*>(
        reinterpret_cast<char*>(_trusted_memory)
      + sizeof(logger*)
      + sizeof(std::pmr::memory_resource*)
      + sizeof(allocator_with_fit_mode::fit_mode));
    if (total > space) {
        log->log("Too much size for allocation", logger::severity::error);
        throw std::bad_alloc();
    }

    fit_result fr;
    switch (get_fit_mode()) {
        case fit_mode::first_fit:   fr = find_first_fit(bytes); break;
        case fit_mode::the_best_fit: fr = find_best_fit(bytes); break;
        case fit_mode::the_worst_fit:fr = find_worst_fit(bytes); break;
        default:
            log->log("Unknown fit mode", logger::severity::error);
            throw std::invalid_argument("Unknown fit mode");
    }

    if (!fr.address) {
        log->log("Allocation failed for size " + std::to_string(bytes), logger::severity::error);
        throw std::bad_alloc();
    }

    void* result = make_allocation(fr, bytes);
    log->log("do_allocate_sm() finished", logger::severity::debug);
    return result;
}

void* allocator_boundary_tags::make_allocation(const fit_result& fr, size_t size) {
    size_t left = fr.free_size - size - occupied_block_metadata_size;
    size_t use_size = size;
    if (left < occupied_block_metadata_size) {
        use_size = fr.free_size - occupied_block_metadata_size;
    }

    char* ptr = fr.address;
    // Записываем размер без флага (LSB = 0)
    *reinterpret_cast<size_t*>(ptr) = use_size & ~size_t(1);
    *reinterpret_cast<void**>(ptr + sizeof(size_t)) = fr.next;
    *reinterpret_cast<void**>(ptr + sizeof(size_t) + sizeof(void*)) = fr.prev;
    *reinterpret_cast<void**>(ptr + sizeof(size_t) + 2*sizeof(void*)) = _trusted_memory;

    if (fr.prev) {
        *reinterpret_cast<void**>(
            reinterpret_cast<char*>(fr.prev) + sizeof(size_t)
        ) = ptr;
    } else {
        char* heap_start = reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
        *reinterpret_cast<void**>(heap_start - sizeof(void*)) = ptr;
    }
    if (fr.next) {
        *reinterpret_cast<void**>(
            reinterpret_cast<char*>(fr.next) + sizeof(size_t) + sizeof(void*)
        ) = ptr;
    }

    return ptr + occupied_block_metadata_size;
}

allocator_boundary_tags::fit_result
allocator_boundary_tags::find_first_fit(size_t size) {
    std::lock_guard<std::mutex> guard(get_mutex());
    size_t total = size + occupied_block_metadata_size;

    char* heap = reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
    size_t space = *reinterpret_cast<size_t*>(
        reinterpret_cast<char*>(_trusted_memory)
      + sizeof(logger*)
      + sizeof(std::pmr::memory_resource*)
      + sizeof(allocator_with_fit_mode::fit_mode));
    char* end = heap + space;
    void** first = reinterpret_cast<void**>(heap - sizeof(void*));

    if (!*first) {
        if (end - heap >= total)
            return {heap, nullptr, nullptr, size_t(end - heap)};
        return {nullptr, nullptr, nullptr, 0};
    }
    size_t front = reinterpret_cast<char*>(*first) - heap;
    if (front >= total)
        return {heap, nullptr, *first, front};

    void* cur = *first;
    while (cur) {
        char* cur_end = reinterpret_cast<char*>(cur)
                       + occupied_block_metadata_size
                       + *reinterpret_cast<size_t*>(cur);
        void* nx = *reinterpret_cast<void**>(
            reinterpret_cast<char*>(cur) + sizeof(size_t));
        size_t hole = (nx ? reinterpret_cast<char*>(nx) : end) - cur_end;
        if (hole >= total)
            return {cur_end, cur, nx, hole};
        cur = nx;
    }
    return {nullptr, nullptr, nullptr, 0};
}

allocator_boundary_tags::fit_result
allocator_boundary_tags::find_best_fit(size_t size) {
    std::lock_guard<std::mutex> guard(get_mutex());
    size_t total = size + occupied_block_metadata_size;

    char* heap = reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
    size_t space = *reinterpret_cast<size_t*>(
        reinterpret_cast<char*>(_trusted_memory)
      + sizeof(logger*)
      + sizeof(std::pmr::memory_resource*)
      + sizeof(allocator_with_fit_mode::fit_mode));
    char* end = heap + space;
    void** first = reinterpret_cast<void**>(heap - sizeof(void*));

    void* best_addr = nullptr;
    size_t best_diff = SIZE_MAX;
    void* best_prev = nullptr;
    void* best_next = nullptr;

    if (*first) {
        size_t front = reinterpret_cast<char*>(*first) - heap;
        if (front >= total) {
            best_addr = heap;
            best_prev = nullptr;
            best_next = *first;
            best_diff = front - total;
        }
    } else if (end - heap >= total) {
        return {heap, nullptr, nullptr, size_t(end - heap)};
    }

    void* cur = *first;
    while (cur) {
        char* cur_end = reinterpret_cast<char*>(cur)
                       + occupied_block_metadata_size
                       + *reinterpret_cast<size_t*>(cur);
        void* nx = *reinterpret_cast<void**>(
            reinterpret_cast<char*>(cur) + sizeof(size_t));
        size_t hole = (nx ? reinterpret_cast<char*>(nx) : end) - cur_end;
        if (hole >= total) {
            size_t diff = hole - total;
            if (diff < best_diff) {
                best_diff = diff;
                best_addr = cur_end;
                best_prev = cur;
                best_next = nx;
                if (diff == 0) break;
            }
        }
        cur = nx;
    }

    if (!best_addr)
        return {nullptr, nullptr, nullptr, 0};
    return {reinterpret_cast<char*>(best_addr), best_prev, best_next, best_diff + total};
}

allocator_boundary_tags::fit_result
allocator_boundary_tags::find_worst_fit(size_t size) {
    std::lock_guard<std::mutex> guard(get_mutex());
    size_t total = size + occupied_block_metadata_size;

    char* heap = reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
    size_t space = *reinterpret_cast<size_t*>(
        reinterpret_cast<char*>(_trusted_memory)
      + sizeof(logger*)
      + sizeof(std::pmr::memory_resource*)
      + sizeof(allocator_with_fit_mode::fit_mode));
    char* end = heap + space;
    void** first = reinterpret_cast<void**>(heap - sizeof(void*));

    void* worst_addr = nullptr;
    size_t worst_size = 0;
    void* worst_prev = nullptr;
    void* worst_next = nullptr;

    if (!*first) {
        if (end - heap >= total)
            return {heap, nullptr, nullptr, size_t(end - heap)};
        return {nullptr, nullptr, nullptr, 0};
    }
    size_t front = reinterpret_cast<char*>(*first) - heap;
    if (front >= total) {
        worst_addr = heap;
        worst_prev = nullptr;
        worst_next = *first;
        worst_size = front;
    }

    void* cur = *first;
    while (cur) {
        char* cur_end = reinterpret_cast<char*>(cur)
                       + occupied_block_metadata_size
                       + *reinterpret_cast<size_t*>(cur);
        void* nx = *reinterpret_cast<void**>(
            reinterpret_cast<char*>(cur) + sizeof(size_t));
        size_t hole = (nx ? reinterpret_cast<char*>(nx) : end) - cur_end;
        if (hole >= total && hole > worst_size) {
            worst_size = hole;
            worst_addr = cur_end;
            worst_prev = cur;
            worst_next = nx;
        }
        cur = nx;
    }

    if (!worst_addr)
        return {nullptr, nullptr, nullptr, 0};
    return {reinterpret_cast<char*>(worst_addr), worst_prev, worst_next, worst_size};
}

void allocator_boundary_tags::do_deallocate_sm(void* at) {
    if (!at) return;
    // вычисляем начало метаданных
    auto* meta_start = reinterpret_cast<char*>(at) - occupied_block_metadata_size;

    auto* log = get_logger();
    log->log("do_deallocate_sm() called", logger::severity::debug);
    std::lock_guard<std::mutex> guard(get_mutex());

    // помечаем блок как свободный (LSB = 1)
    *reinterpret_cast<size_t*>(meta_start) |= 1;

    // читаем next и prev из метаданных
    void* next_blk = *reinterpret_cast<void**>(meta_start + sizeof(size_t));
    void* prev_blk = *reinterpret_cast<void**>(meta_start + sizeof(size_t) + sizeof(void*));
    char* heap_start = reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
    void** head_ptr = reinterpret_cast<void**>(heap_start - sizeof(void*));

    // удаляем из списка занятых
    if (prev_blk) {
        *reinterpret_cast<void**>(reinterpret_cast<char*>(prev_blk) + sizeof(size_t))
          = next_blk;
    } else {
        *head_ptr = next_blk;
    }
    if (next_blk) {
        *reinterpret_cast<void**>(reinterpret_cast<char*>(next_blk)
          + sizeof(size_t) + sizeof(void*))
          = prev_blk;
    }

    // теперь сливаем соседние свободные блоки
    merge_blocks(meta_start);
    log->log("do_deallocate_sm() finished", logger::severity::debug);
}

allocator_with_fit_mode::fit_mode allocator_boundary_tags::get_fit_mode() const {
    if (!_trusted_memory) {
        throw std::logic_error("Allocator not initialized: no trusted memory");
    }

    auto* base = reinterpret_cast<unsigned char*>(_trusted_memory);

    constexpr size_t offset_logger = sizeof(logger*);
    constexpr size_t offset_parent = offset_logger + sizeof(std::pmr::memory_resource*);
    constexpr size_t offset_mode   = offset_parent + 0; // fit_mode идёт сразу после parent

    auto* mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(base + offset_mode);
    return *mode_ptr;
}

inline void allocator_boundary_tags::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
	if (!_trusted_memory) {
        throw std::logic_error("Allocator not initialized: no trusted memory");
    }

    auto* base = reinterpret_cast<unsigned char*>(_trusted_memory);

    constexpr size_t offset_logger = sizeof(logger*);
    constexpr size_t offset_parent = offset_logger + sizeof(std::pmr::memory_resource*);
    constexpr size_t offset_mode   = offset_parent + 0;

    auto* mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(base + offset_mode);
    *mode_ptr = mode;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const {
	logger *logger = get_logger();
	logger->trace("allocator_boundary_tags::get_blocks_info_inner() started.");
	std::vector<allocator_test_utils::block_info> blocks_info;

	if (!_trusted_memory) {
		logger->error("No trusted memory.");
		return blocks_info;
	}

	try {
		char *heap_start = reinterpret_cast<char *>(_trusted_memory) + allocator_metadata_size;
		size_t heap_size = *reinterpret_cast<size_t *>(
				reinterpret_cast<char *>(_trusted_memory) +
				sizeof(class logger *) + sizeof(memory_resource *) +
				sizeof(allocator_with_fit_mode::fit_mode));
		char *heap_end = heap_start + heap_size;

		void **first_block_ptr = reinterpret_cast<void **>(heap_start - sizeof(void *));
		char *current_block = reinterpret_cast<char *>(*first_block_ptr);
		while (current_block && current_block < heap_end) {
			size_t block_size = *reinterpret_cast<size_t *>(current_block);
			void *next_block = *reinterpret_cast<void **>(current_block + sizeof(size_t));
			bool is_occupied = true;
			blocks_info.push_back({.block_size = block_size + occupied_block_metadata_size,
								   .is_block_occupied = is_occupied});
			current_block = reinterpret_cast<char *>(next_block);
		}

		current_block = reinterpret_cast<char *>(*first_block_ptr);
		char *prev_block_end = heap_start;
		while (current_block && current_block < heap_end) {
			if (current_block > prev_block_end) {
				size_t hole_size = current_block - prev_block_end;
				blocks_info.push_back({.block_size = hole_size, .is_block_occupied = false});
			}

			size_t block_size = *reinterpret_cast<size_t *>(current_block);
			prev_block_end = current_block + occupied_block_metadata_size + block_size;
			void *next_block = *reinterpret_cast<void **>(current_block + sizeof(size_t));
			current_block = reinterpret_cast<char *>(next_block);
		}

		if (prev_block_end < heap_end) {
			size_t hole_size = heap_end - prev_block_end;
			blocks_info.push_back({.block_size = hole_size, .is_block_occupied = false});
		}
	} catch (...) {
		logger->error("Iteration failed.");
		throw;
	}

	return blocks_info;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const {
    auto* log = get_logger();
    log->log("get_blocks_info() called", logger::severity::debug);

    std::lock_guard<std::mutex> guard(get_mutex());
    auto result = get_blocks_info_inner();

    log->log("Retrieved " + std::to_string(result.size()) + " blocks", logger::severity::information);
    log->log("get_blocks_info() finished", logger::severity::debug);

    return result;
}

inline logger *allocator_boundary_tags::get_logger() const {
	if (!_trusted_memory) return nullptr;

	return *reinterpret_cast<logger **>(_trusted_memory);
}

inline std::string allocator_boundary_tags::get_typename() const noexcept {
	return "allocator_boundary_tags";
}


allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept {
	return {reinterpret_cast<char *>(_trusted_memory) + allocator_metadata_size};
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept {
	size_t total_size = *reinterpret_cast<size_t *>(reinterpret_cast<char *>(_trusted_memory) +
													sizeof(logger *) + sizeof(memory_resource *) +
													sizeof(allocator_with_fit_mode::fit_mode));

	return {reinterpret_cast<char *>(_trusted_memory) + total_size};
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept {
	if (this == &other) return true;

	const auto *derived = dynamic_cast<const allocator_boundary_tags *>(&other);

	if (!derived) return false;

	return this->_trusted_memory == derived->_trusted_memory;
}

bool allocator_boundary_tags::boundary_iterator::operator==(const allocator_boundary_tags::boundary_iterator &other) const noexcept {
	return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(const allocator_boundary_tags::boundary_iterator &other) const noexcept {
	return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &
allocator_boundary_tags::boundary_iterator::operator++() & noexcept {
	auto *current_tag_ptr = reinterpret_cast<size_t *>(_occupied_ptr);

	size_t raw_tag = *current_tag_ptr;
	size_t block_size = raw_tag & ~size_t(1);

	auto *next_block_ptr = reinterpret_cast<unsigned char *>(_occupied_ptr) + block_size;
	_occupied_ptr = static_cast<void *>(next_block_ptr);

	auto *new_tag_ptr = reinterpret_cast<size_t *>(_occupied_ptr);
	size_t new_tag = *new_tag_ptr;
	_occupied = static_cast<bool>(new_tag & 1);

	return *this;
}


allocator_boundary_tags::boundary_iterator &
allocator_boundary_tags::boundary_iterator::operator--() & noexcept {
	auto *size_tag_ptr = reinterpret_cast<size_t *>(
		reinterpret_cast<unsigned char *>(_occupied_ptr) - sizeof(size_t));

	size_t raw_tag = *size_tag_ptr;
	size_t block_size = raw_tag & ~size_t(1);

	auto *prev_block_ptr = reinterpret_cast<unsigned char *>(_occupied_ptr) - block_size;
	_occupied_ptr = static_cast<void *>(prev_block_ptr);

	auto *tag_ptr = reinterpret_cast<size_t *>(_occupied_ptr);
	size_t tag = *tag_ptr;
	_occupied = static_cast<bool>(tag & 1);

	return *this;
}


allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int) {
	boundary_iterator tmp = *this;
	++(*this);
	return tmp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int) {
	boundary_iterator tmp = *this;
	--(*this);
	return tmp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept {
	return *reinterpret_cast<size_t *>(_occupied_ptr) & ~size_t(1);
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept {
	return _occupied;
}

void *allocator_boundary_tags::boundary_iterator::operator*() const noexcept {
	return reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(_occupied_ptr) + sizeof(size_t));
}

allocator_boundary_tags::boundary_iterator::boundary_iterator() : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr) {}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted) : _trusted_memory(trusted) {
	_occupied_ptr = trusted;
	size_t tag = *reinterpret_cast<size_t *>(_occupied_ptr);
	_occupied = tag & 1;
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept {
	return _occupied_ptr;
}

inline std::mutex& allocator_boundary_tags::get_mutex() const {
    if (!_trusted_memory) {
        throw std::logic_error("Mutex doesn't exist.");
    }
    auto* ptr = reinterpret_cast<unsigned char*>(_trusted_memory)
              + sizeof(logger*)
              + sizeof(std::pmr::memory_resource*)
              + sizeof(allocator_with_fit_mode::fit_mode)
              + sizeof(size_t);
    return *reinterpret_cast<std::mutex*>(ptr);
}

void allocator_boundary_tags::merge_blocks(void* freed_meta_ptr) {
    auto* log = get_logger();
    log->log("merge_blocks() called", logger::severity::debug);

    char* blk_start = reinterpret_cast<char*>(freed_meta_ptr);
    size_t blk_raw = *reinterpret_cast<size_t*>(blk_start);
    size_t blk_size = blk_raw & ~size_t(1);  // размер без флага
    char* blk_end = blk_start + occupied_block_metadata_size + blk_size;

    char* heap_base = reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
    void** head_ptr = reinterpret_cast<void**>(heap_base - sizeof(void*));

    // Попытка слияния с next, только если он свободен
    void* next_blk = *reinterpret_cast<void**>(blk_start + sizeof(size_t));
    if (next_blk) {
        char* next_start = reinterpret_cast<char*>(next_blk);
        size_t next_raw = *reinterpret_cast<size_t*>(next_start);
        if ((next_raw & 1) && blk_end == next_start) {
            size_t next_size = next_raw & ~size_t(1);
            void* next_next = *reinterpret_cast<void**>(next_start + sizeof(size_t));

            // объединяем размеры и перенаправляем указатели
            *reinterpret_cast<size_t*>(blk_start) = (blk_size + occupied_block_metadata_size + next_size) | 1;
            *reinterpret_cast<void**>(blk_start + sizeof(size_t)) = next_next;
            if (next_next) {
                char* nn = reinterpret_cast<char*>(next_next);
                *reinterpret_cast<void**>(nn + sizeof(size_t) + sizeof(void*)) = blk_start;
            }
            log->log("merge_blocks() merged with next block", logger::severity::debug);

            // обновляем blk_end и blk_size для последующего слияния с prev
            blk_size = (blk_size + occupied_block_metadata_size + next_size);
            blk_end = blk_start + occupied_block_metadata_size + blk_size;
        }
    }

    // Попытка слияния с prev, только если он свободен
    void* prev_blk = *reinterpret_cast<void**>(blk_start + sizeof(size_t) + sizeof(void*));
    if (prev_blk) {
        char* prev_start = reinterpret_cast<char*>(prev_blk);
        size_t prev_raw = *reinterpret_cast<size_t*>(prev_start);
        size_t prev_size = prev_raw & ~size_t(1);
        char* prev_end = prev_start + occupied_block_metadata_size + prev_size;
        if ((prev_raw & 1) && prev_end == blk_start) {
            void* next_after = *reinterpret_cast<void**>(blk_start + sizeof(size_t));
            size_t combined = prev_size + occupied_block_metadata_size + blk_size;

            *reinterpret_cast<size_t*>(prev_start) = combined | 1;
            *reinterpret_cast<void**>(prev_start + sizeof(size_t)) = next_after;
            if (next_after) {
                char* na = reinterpret_cast<char*>(next_after);
                *reinterpret_cast<void**>(na + sizeof(size_t) + sizeof(void*)) = prev_start;
            }
            // если голова списка указывала на blk_start, переназначаем её
            if (blk_start == *head_ptr) {
                *head_ptr = prev_start;
            }
            log->log("merge_blocks() merged with previous block", logger::severity::debug);
        }
    }

    log->log("merge_blocks() finished", logger::severity::debug);
}
