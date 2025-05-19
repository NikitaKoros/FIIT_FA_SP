#include <not_implemented.h>
#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap(
    logger *logger) : _logger(logger) 
{}

[[nodiscard]] void* allocator_global_heap::do_allocate_sm(size_t size)
{
    if (auto log = get_logger())
    {
        log->log("Entering do_allocate_sm", logger::severity::trace);
    }

    try 
    {
        if (size == 0)
        {
            if (auto log = get_logger())
            {
                log->log("Zero-size allocation request", logger::severity::debug);
            }
            return nullptr;
        }

        size_t total_size = size + sizeof(size_t);
        void* block = ::operator new(total_size); // глобальная функция, только выделяет память, не вызывая конструкторов
        *reinterpret_cast<size_t*>(block) = size;
        void* user_ptr = static_cast<char*>(block) + sizeof(size_t);
        
        if (auto log = get_logger())
        {
            log->log(
                "Allocated " + std::to_string(size) + " bytes at " + 
                std::to_string(reinterpret_cast<uintptr_t>(user_ptr)),
                logger::severity::trace
            );
        }
        
        return user_ptr;
    }
    catch (const std::bad_alloc& ex)
    {
        if (auto log = get_logger())
        {
            log->log(
                "Allocation failed: " + std::string(ex.what()),
                logger::severity::error
            );
        }
        throw;
    }
    
    if (auto log = get_logger())
    {
        log->log("Exiting do_allocate_sm", logger::severity::trace);
    }
}

void allocator_global_heap::do_deallocate_sm(
    void *at)
{
    if (auto log = get_logger())
    {
        log->log("Entering do_deallocate_sm", logger::severity::trace);
    }

    if (at == nullptr)
    {
        if (auto log = get_logger())
        {
            log->log("Attempt to deallocate nullptr", logger::severity::debug);
        }
        return;
    }
    
    void* block = static_cast<char*>(at) - sizeof(size_t);
    size_t size = *reinterpret_cast<size_t*>(block);

    if (auto log = get_logger())
    {
        log->log(
            "Deallocating " + std::to_string(size) + " bytes at " + 
            std::to_string(reinterpret_cast<uintptr_t>(at)),
            logger::severity::trace
        );
    }
    
    ::operator delete(block);
    
    if (auto log = get_logger())
    {
        log->log("Exiting do_deallocate_sm", logger::severity::trace);
    }
}

inline logger *allocator_global_heap::get_logger() const
{
    return _logger;
}

inline std::string allocator_global_heap::get_typename() const
{
    return "allocator_global_heap";
}

allocator_global_heap::~allocator_global_heap() = default;

allocator_global_heap::allocator_global_heap(const allocator_global_heap &other) 
    : _logger(other._logger)
{}

allocator_global_heap &allocator_global_heap::operator=(const allocator_global_heap &other)
{
    if (this != &other) {
        _logger = other._logger;
    }
    return *this;
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    if (auto log = get_logger())
    {
        log->log("Checking equality with another memory resource", logger::severity::trace);
    }
    return dynamic_cast<const allocator_global_heap*>(&other) != nullptr;
}

allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept
    : _logger(other._logger)
{
    other._logger = nullptr;
}

allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept
{
    if (this != &other) {
        _logger = other._logger;
        other._logger = nullptr;
    }
    return *this;
}
