#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (_trusted_memory == nullptr) {
        return; // Память уже освобождена или перемещена
    }

    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    logger* log = metadata->log;
    std::pmr::memory_resource* parent = metadata->parent_allocator != nullptr ?
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

    _trusted_memory = nullptr; // Предотвращаем повторное освобождение
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    if (other.get_logger() != nullptr) {
        other.get_logger()->log("allocator_boundary_tags::allocator_boundary_tags(move) called", 
                                logger::severity::debug);
    }
    
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
    
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::allocator_boundary_tags(move) finished", 
                          logger::severity::debug);
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (other.get_logger() != nullptr) {
        other.get_logger()->log("allocator_boundary_tags::operator=(move) called", 
                                logger::severity::debug);
    }
    
    if (this != &other) {
        this->~allocator_boundary_tags();
        
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::operator=(move) finished", 
                          logger::severity::debug);
    }
    
    return *this;
}


/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
// allocator_boundary_tags::allocator_boundary_tags(
//     size_t space_size,
//     std::pmr::memory_resource *parent_allocator,
//     logger *logger,
//     allocator_with_fit_mode::fit_mode allocate_fit_mode)
// {
//     if (logger != nullptr) {
//         logger->log("allocator_boundary_tags::allocator_boundary_tags() called", 
//                     logger::severity::debug);
//     }
    
//     std::pmr::memory_resource* actual_parent = 
//         parent_allocator != nullptr ? parent_allocator : std::pmr::get_default_resource();
    
//     _trusted_memory = actual_parent->allocate(space_size);
    
//     auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
//     metadata->log = logger;
//     metadata->parent_allocator = actual_parent;
//     metadata->mode = allocate_fit_mode;
//     metadata->space_size = space_size;
//     new (&metadata->mutex) std::mutex();
    
//     const size_t metadata_size = sizeof(allocator_metadata);
//     const size_t available_size = space_size 
//                                 - metadata_size 
//                                 - sizeof(block_header);
    
//     block_header* first_block = reinterpret_cast<block_header*>(
//         static_cast<char*>(_trusted_memory) + metadata_size
//     );
    
//     first_block->size = available_size;
//     first_block->occupied = false;
//     first_block->prev = nullptr;
//     first_block->next = nullptr;
    
//     metadata->first_block = first_block;
    
//     if (logger != nullptr) {
//         logger->log("Initialized allocator with " + std::to_string(available_size) + 
//                     " bytes available", 
//                     logger::severity::information);
        
//         logger->log("allocator_boundary_tags::allocator_boundary_tags() finished", 
//                     logger::severity::debug);
//     }
// }

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

    // Determine actual parent allocator
    std::pmr::memory_resource* actual_parent = 
        parent_allocator != nullptr ? parent_allocator : std::pmr::get_default_resource();

    // Calculate sizes: metadata lives before the block region
    const size_t metadata_size = sizeof(allocator_metadata);
    // The user-visible region size (excluding metadata)
    const size_t region_size = space_size;
    // Total bytes to allocate from parent = metadata + region
    const size_t total_size = metadata_size + region_size;

    // Allocate the combined region
    _trusted_memory = actual_parent->allocate(total_size);

    // Initialize metadata at the start of the allocation
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    metadata->log = logger;
    metadata->parent_allocator = actual_parent;
    metadata->mode = allocate_fit_mode;
    // Store total allocation size for deallocation later
    metadata->space_size = total_size;
    new (&metadata->mutex) std::mutex();

    // The first free block lives immediately after the metadata
    auto* first_block = reinterpret_cast<block_header*>(
        static_cast<char*>(_trusted_memory) + metadata_size
    );
    // Make the entire region available (subtract only the block header itself)
    first_block->size = region_size - sizeof(block_header);
    first_block->occupied = false;
    first_block->prev = nullptr;
    first_block->next = nullptr;

    // Point metadata at the first block
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
    const size_t min_block_size = sizeof(block_header) + 1; // Minimal usable size
    
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
    //     // Special case for zero size allocation - allocate minimum block
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
        throw std::bad_alloc();  // Throw std::bad_alloc when allocation fails
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
        info.block_size = it.size() + sizeof(block_header); // Добавляем размер заголовка
        result.push_back(info);
    }
    
    return result;
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
{
    if (other.get_logger() != nullptr) {
        other.get_logger()->log("allocator_boundary_tags::allocator_boundary_tags(copy) called", 
                                logger::severity::debug);
    }
    
    auto* other_metadata = static_cast<allocator_metadata*>(other._trusted_memory);
    
    std::pmr::memory_resource* parent = 
        other_metadata->parent_allocator != nullptr ? 
        other_metadata->parent_allocator : 
        std::pmr::get_default_resource();
    
    _trusted_memory = parent->allocate(other_metadata->space_size);
    std::memcpy(_trusted_memory, other._trusted_memory, other_metadata->space_size);
    
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    new (&metadata->mutex) std::mutex();
    
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::allocator_boundary_tags(copy) finished", 
                          logger::severity::debug);
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    if (this == &other) {
        return *this;
    }
    
    if (other.get_logger() != nullptr) {
        other.get_logger()->log("allocator_boundary_tags::operator=(copy) called", 
                                logger::severity::debug);
    }
    
    this->~allocator_boundary_tags();
    
    auto* other_metadata = static_cast<allocator_metadata*>(other._trusted_memory);
    
    std::pmr::memory_resource* parent = 
        other_metadata->parent_allocator != nullptr ? 
        other_metadata->parent_allocator : 
        std::pmr::get_default_resource();
    
    _trusted_memory = parent->allocate(other_metadata->space_size);
    std::memcpy(_trusted_memory, other._trusted_memory, other_metadata->space_size);
    
    auto* metadata = static_cast<allocator_metadata*>(_trusted_memory);
    new (&metadata->mutex) std::mutex();
    
    if (get_logger() != nullptr) {
        get_logger()->log("allocator_boundary_tags::operator=(copy) finished", 
                          logger::severity::debug);
    }
    
    return *this;
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