#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (_trusted_memory == nullptr) {
        return;
    }

    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    logger* log = metadata->log;
    std::pmr::memory_resource* parent = metadata->parent_allocator != nullptr ? // абстрактный класс
                                        metadata->parent_allocator :
                                        std::pmr::get_default_resource();
    size_t space_size = metadata->space_size;

    if (log != nullptr) {
        log->log("allocator_boundary_tags::~allocator_boundary_tags() called",
                 logger::severity::debug);
    }

    parent->deallocate(_trusted_memory, space_size);

    if (log != nullptr) {
        log->log("allocator_boundary_tags::~allocator_boundary_tags() finished",
                 logger::severity::debug);
    }

    _trusted_memory = nullptr;
}

// allocator_boundary_tags::allocator_boundary_tags(
//     allocator_boundary_tags&& other) noexcept
//     : smart_mem_resource(std::move(other)),
//       allocator_with_fit_mode(std::move(other)),
//       logger_guardant(std::move(other)),
//       _trusted_memory(std::exchange(other._trusted_memory, nullptr))
// {
//     std::lock_guard lock(other.get_mutex());
    
//     auto* other_meta = static_cast<allocator_metadata*>(other._trusted_memory);
//     auto* this_meta = static_cast<allocator_metadata*>(_trusted_memory);
    
//     this_meta->first_block = std::exchange(other_meta->first_block, nullptr);
    
//     if (other_meta->log) {
//         other_meta->log->log("Moved to new allocator", logger::severity::debug);
//     }
// }

allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags&& other) noexcept
: smart_mem_resource(std::move(other)),
allocator_with_fit_mode(std::move(other)),
logger_guardant(std::move(other))
{
    auto* other_meta = static_cast<allocator_metadata*>(other._trusted_memory);

    _trusted_memory = std::exchange(other._trusted_memory, nullptr);

    std::lock_guard lock(other_meta->mutex);

    auto* this_meta = static_cast<allocator_metadata*>(_trusted_memory);
    this_meta->first_block = std::exchange(other_meta->first_block, nullptr);

    if (other_meta->log) {
        other_meta->log->log("Moved to new allocator", logger::severity::debug);
    }
}


allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (this != &other) {
        std::unique_lock lock_this(get_mutex(), std::defer_lock);
        std::unique_lock lock_other(other.get_mutex(), std::defer_lock);
        std::lock(lock_this, lock_other);

        if (_trusted_memory) {
            auto* meta = static_cast<allocator_metadata*>(_trusted_memory);
            meta->parent_allocator->deallocate(
                _trusted_memory, 
                meta->space_size
            );
        }

        _trusted_memory = std::exchange(other._trusted_memory, nullptr);

        static_cast<allocator_with_fit_mode&>(*this) = std::move(other);
        static_cast<logger_guardant&>(*this) = std::move(other);
    }
    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    std::pmr::memory_resource *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (logger != nullptr) {
        logger->log("allocator_boundary_tags::allocator_boundary_tags() called", 
                    logger::severity::debug);
    }

    std::pmr::memory_resource* actual_parent = 
        parent_allocator != nullptr ? parent_allocator : std::pmr::get_default_resource();

    const size_t metadata_size = sizeof(allocator_metadata);
    const size_t region_size = space_size;
    const size_t total_size = metadata_size + region_size;

    _trusted_memory = actual_parent->allocate(total_size);

    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    metadata->log = logger;
    metadata->parent_allocator = actual_parent;
    metadata->mode = allocate_fit_mode;
    metadata->space_size = total_size;
    new (&metadata->mutex) std::mutex();

    auto* first_block = reinterpret_cast<block_header*>(
        static_cast<char*>(_trusted_memory) + metadata_size
    );
    first_block->size = region_size - sizeof(block_header);
    first_block->occupied = false;
    first_block->prev = nullptr;
    first_block->next = nullptr;

    metadata->first_block = first_block;

    if (logger != nullptr) {
        logger->log("Initialized allocator with " + std::to_string(first_block->size) + 
                    " bytes available", 
                    logger::severity::information);
        logger->log("allocator_boundary_tags::allocator_boundary_tags() finished", 
                    logger::severity::debug);
    }
}


block_header* allocator_boundary_tags::find_first_fit(block_header* first_block, size_t size) {
    block_header* current = first_block;
    while (current != nullptr) {
        if (!current->occupied && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

block_header* allocator_boundary_tags::find_best_fit(block_header* first_block, size_t size) {
    block_header* best = nullptr;
    block_header* current = first_block;
    size_t min_diff = SIZE_MAX;
    
    while (current != nullptr) {
        if (!current->occupied && current->size >= size) {
            size_t diff = current->size - size;
            if (diff < min_diff) {
                min_diff = diff;
                best = current;
            }
        }
        current = current->next;
    }
    
    return best;
}

block_header* allocator_boundary_tags::find_worst_fit(block_header* first_block, size_t size) {
    block_header* worst = nullptr;
    block_header* current = first_block;
    size_t max_size = 0;
    
    while (current != nullptr) 
    {
        if (!current->occupied && current->size >= size) 
        {
            if (current->size > max_size) 
            {
                max_size = current->size;
                worst = current;
            }
        }
        current = current->next;
    }
    
    return worst;
}

void allocator_boundary_tags::split_block(block_header* block, size_t size) 
{
    const size_t min_block_size = sizeof(block_header) + 1;
    
    if (block->size > size + min_block_size) 
    {
        char* new_block_start = reinterpret_cast<char*>(block) + sizeof(block_header) + size;
        
        block_header* new_block = reinterpret_cast<block_header*>(new_block_start);
        new_block->size = block->size - size - sizeof(block_header);
        new_block->occupied = false;
        new_block->prev = block;
        new_block->next = block->next;
        
        if (block->next != nullptr) 
        {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
    }
}

void allocator_boundary_tags::merge_blocks(block_header* block) 
{
    if (block->prev != nullptr && !block->prev->occupied) 
    {
        block->prev->size += sizeof(block_header) + block->size;
        block->prev->next = block->next;
        if (block->next != nullptr) 
        {
            block->next->prev = block->prev;
        }
        block = block->prev;
    }
    
    if (block->next != nullptr && !block->next->occupied) 
    {
        block->size += sizeof(block_header) + block->next->size;
        block->next = block->next->next;
        if (block->next != nullptr) 
        {
            block->next->prev = block;
        }
    }
}

bool allocator_boundary_tags::is_pointer_in_trusted_memory(void* ptr, void* trusted_memory, size_t space_size) {
    char* start = static_cast<char*>(trusted_memory);
    char* end = start + space_size;
    return ptr >= start && ptr < end;
}

std::string allocator_boundary_tags::get_memory_state_string(block_header* first_block) {
    std::string result;
    block_header* current = first_block;
    
    while (current != nullptr) {
        result += (current->occupied ? "occup " : "avail ");
        result += std::to_string(current->size);
        
        if (current->next != nullptr) {
            result += "|";
        }
        
        current = current->next;
    }
    
    return result;
}

size_t allocator_boundary_tags::get_available_memory(block_header* first_block) {
    size_t available = 0;
    block_header* current = first_block;
    
    while (current != nullptr) {
        if (!current->occupied) {
            available += current->size;
        }
        current = current->next;
    }
    
    return available;
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    // if (size == 0) {
    //     size = 1;
    // }
    
    std::lock_guard<std::mutex> lock(get_mutex());
    
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    block_header* block = nullptr;

    if (get_logger() != nullptr) {
        std::string state_before = get_memory_state_string(metadata->first_block);
        get_logger()->log("Before allocation, memory state: " + state_before, logger::severity::debug);
    }
    
    switch (metadata->mode) {
        case allocator_with_fit_mode::fit_mode::first_fit:
            block = find_first_fit(metadata->first_block, size);
            break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            block = find_best_fit(metadata->first_block, size);
            break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            block = find_best_fit(metadata->first_block, size);
            break;
    }
    
    if (block == nullptr) {
        if (get_logger() != nullptr) {
            get_logger()->log("Failed to allocate memory: no suitable block found", 
                              logger::severity::error);
            get_logger()->log("allocator_boundary_tags::do_allocate_sm() finished", 
                              logger::severity::debug);
        }
        throw std::bad_alloc();
    }
    
    split_block(block, size);
    
    block->occupied = true;
    
    void* user_ptr = reinterpret_cast<char*>(block) + sizeof(block_header);
    
    if (get_logger() != nullptr) {
        size_t available_memory = get_available_memory(metadata->first_block);
        std::string memory_state = get_memory_state_string(metadata->first_block);
        
        get_logger()->log("Available memory after allocation: " + std::to_string(available_memory) + " bytes", 
                          logger::severity::information);
        get_logger()->log("Memory state: " + memory_state, 
                          logger::severity::debug);
        get_logger()->log("allocator_boundary_tags::do_allocate_sm() finished", 
                          logger::severity::debug);
    }
    
    return user_ptr;
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::do_deallocate_sm() called", 
                          logger::severity::debug);
    }
    
    std::lock_guard<std::mutex> lock(get_mutex());
    
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    
    if (!is_pointer_in_trusted_memory(at, _trusted_memory, metadata->space_size)) {
        if (get_logger() != nullptr) {
            get_logger()->log("Attempted to deallocate memory outside trusted region", 
                              logger::severity::error);
            get_logger()->log("allocator_boundary_tags::do_deallocate_sm() finished", 
                              logger::severity::debug);
        }
        throw std::invalid_argument("Pointer is not within trusted memory region");
    }
    
    block_header* block = reinterpret_cast<block_header*>(
        static_cast<char*>(at) - sizeof(block_header)
    );
    
    if (!block->occupied) {
        if (get_logger() != nullptr) {
            get_logger()->log("Attempted to deallocate already freed memory", 
                              logger::severity::error);
            get_logger()->log("allocator_boundary_tags::do_deallocate_sm() finished", 
                              logger::severity::debug);
        }
        throw std::invalid_argument("Memory already deallocated");
    }
    
    block->occupied = false;
    
    merge_blocks(block);
    
    if (get_logger() != nullptr) {
        size_t available_memory = get_available_memory(metadata->first_block);
        std::string memory_state = get_memory_state_string(metadata->first_block);
        
        get_logger()->log("Available memory after deallocation: " + std::to_string(available_memory) + " bytes", 
                          logger::severity::information);
        get_logger()->log("Memory state: " + memory_state, 
                          logger::severity::debug);
        get_logger()->log("allocator_boundary_tags::do_deallocate_sm() finished", 
                          logger::severity::debug);
    }
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::set_fit_mode() called", 
                          logger::severity::debug);
    }
    
    std::lock_guard<std::mutex> lock(get_mutex());
    
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    metadata->mode = mode;
    
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::set_fit_mode() finished", 
                          logger::severity::debug);
    }
}


std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::get_blocks_info() called", 
                          logger::severity::debug);
    }
    
    std::lock_guard<std::mutex> lock(get_mutex());
    
    std::vector<allocator_test_utils::block_info> result = get_blocks_info_inner();
    
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::get_blocks_info() finished", 
                          logger::severity::debug);
    }
    
    return result;
}

inline logger *allocator_boundary_tags::get_logger() const
{
    if (_trusted_memory == nullptr) {
        return nullptr;
    }
    
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    return metadata->log;
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "allocator_boundary_tags";
}

std::mutex& allocator_boundary_tags::get_mutex() const noexcept {
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    return metadata->mutex;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    if (_trusted_memory == nullptr) {
        return boundary_iterator();
    }
    
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    return boundary_iterator(metadata->first_block);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    return boundary_iterator();
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> result;
    
    for (auto it = begin(); it != end(); ++it) 
    {
        allocator_test_utils::block_info info;
        info.is_block_occupied = it.occupied(); 
        info.block_size = it.size() + sizeof(block_header);
        result.push_back(info);
    }
    
    return result;
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
    : smart_mem_resource(other),
      allocator_with_fit_mode(other),
      logger_guardant(static_cast<const logger_guardant&>(other))
{
    std::lock_guard lock(other.get_mutex());
    
    auto* other_meta = static_cast<allocator_metadata*>(other._trusted_memory);
    const size_t total_size = other_meta->space_size;
    
    std::pmr::memory_resource* parent = other_meta->parent_allocator;
    if (!parent) {
        parent = std::pmr::get_default_resource();
    }
    
    _trusted_memory = parent->allocate(total_size);
    
    auto* this_meta = static_cast<allocator_metadata*>(_trusted_memory);
    this_meta->log = other_meta->log;
    this_meta->parent_allocator = parent;
    this_meta->mode = other_meta->mode;
    this_meta->space_size = other_meta->space_size;
    new (&this_meta->mutex) std::mutex();
    
    block_header* prev = nullptr;
    block_header* curr = other_meta->first_block;
    block_header* this_curr = reinterpret_cast<block_header*>(
        static_cast<char*>(_trusted_memory) + sizeof(allocator_metadata)
    );
    
    while (curr) {
        *this_curr = *curr;
        this_curr->prev = prev;
        this_curr->next = nullptr;
        
        if (prev) {
            prev->next = this_curr;
        }
        
        prev = this_curr;
        curr = curr->next;
        
        this_curr = reinterpret_cast<block_header*>(
            reinterpret_cast<char*>(this_curr) + 
            sizeof(block_header) + 
            this_curr->size
        );
    }
    
    this_meta->first_block = (prev) ? 
        reinterpret_cast<block_header*>(
            static_cast<char*>(_trusted_memory) + sizeof(allocator_metadata)
        ) : 
        nullptr;
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    if (this != &other) {
        std::unique_lock lock_this(get_mutex(), std::defer_lock);
        std::unique_lock lock_other(other.get_mutex(), std::defer_lock);
        std::lock(lock_this, lock_other);

        allocator_boundary_tags temp(other);
        
        swap(*this, temp);
    }
    return *this;
}

void allocator_boundary_tags::swap(allocator_boundary_tags& a, allocator_boundary_tags& b) noexcept {
    using std::swap;
    swap(a._trusted_memory, b._trusted_memory);
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::do_is_equal() called", 
                          logger::severity::trace);
    }
    
    auto* other_allocator = dynamic_cast<const allocator_boundary_tags*>(&other);
    if (other_allocator == nullptr) {
        return false;
    }
    
    bool result = _trusted_memory == other_allocator->_trusted_memory;
    
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::do_is_equal() finished", 
                          logger::severity::trace);
    }
    
    return result;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
        const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (_occupied_ptr != nullptr) {
        auto* block = static_cast<block_header*>(_occupied_ptr);
        _occupied_ptr = block->next;
        if (_occupied_ptr != nullptr) {
            auto* next_block = static_cast<block_header*>(_occupied_ptr);
            _occupied = next_block->occupied;
        }
    }
    return *this;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    if (_occupied_ptr != nullptr) {
        auto* block = static_cast<block_header*>(_occupied_ptr);
        _occupied_ptr = block->prev;
        if (_occupied_ptr != nullptr) {
            auto* prev_block = static_cast<block_header*>(_occupied_ptr);
            _occupied = prev_block->occupied;
        }
    }
    return *this;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    boundary_iterator temp = *this;
    ++(*this);
    return temp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    boundary_iterator temp = *this;
    --(*this);
    return temp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    if (_occupied_ptr == nullptr) {
        return 0;
    }
    auto* block = static_cast<block_header*>(_occupied_ptr);
    return block->size;
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    if (_occupied_ptr == nullptr) {
        return false;
    }
    auto* block = static_cast<block_header*>(_occupied_ptr);
    return block->occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    if (_occupied_ptr == nullptr) {
        return nullptr;
    }
    return static_cast<char*>(_occupied_ptr) + sizeof(block_header);
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
: _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr)
{}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted): _occupied_ptr(trusted), _trusted_memory(trusted) {
    if (_occupied_ptr != nullptr) {
        auto* block = static_cast<block_header*>(_occupied_ptr);
        _occupied = block->occupied;
    } else {
        _occupied = false;
    }
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}