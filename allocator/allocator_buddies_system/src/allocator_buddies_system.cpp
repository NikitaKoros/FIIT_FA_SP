#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"

allocator_buddies_system::~allocator_buddies_system() 
{
    if (_trusted_memory == nullptr) {
        return;
    }

    char* base = static_cast<char*>(_trusted_memory);

    logger* log = *reinterpret_cast<logger**>(base);
    base += sizeof(logger*);

    std::pmr::memory_resource* parent = *reinterpret_cast<std::pmr::memory_resource**>(base);
    base += sizeof(std::pmr::memory_resource*);

    fit_mode mode = *reinterpret_cast<fit_mode*>(base);
    base += sizeof(fit_mode);

    unsigned char k = *reinterpret_cast<unsigned char*>(base);
    base += sizeof(unsigned char);

    std::mutex* mtx = reinterpret_cast<std::mutex*>(base);

    if (log) {
        log->log("allocator_buddies_system::~allocator_buddies_system() started",
                 logger::severity::trace);
    }

    mtx->~mutex();

    ::operator delete(_trusted_memory);

    if (log) {
        log->log("allocator_buddies_system::~allocator_buddies_system() completed",
                 logger::severity::trace);
    }

    _trusted_memory = nullptr;
}


allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system &&other) noexcept
    : smart_mem_resource(std::move(other))
    , allocator_with_fit_mode(std::move(other))
    , logger_guardant(std::move(other))
    , _trusted_memory(std::exchange(other._trusted_memory, nullptr))
{
    constexpr size_t offset_to_mutex = 
          sizeof(logger*) 
        + sizeof(std::pmr::memory_resource*) 
        + sizeof(fit_mode) 
        + sizeof(unsigned char);
    auto* other_mtx = reinterpret_cast<std::mutex*>(
        static_cast<char*>(other._trusted_memory) + offset_to_mutex
    );
    std::lock_guard<std::mutex> lock(*other_mtx);

    if (get_logger() != nullptr) {
        get_logger()->log("allocator_buddies_system(move) constructed via move-ctor", 
                          logger::severity::debug);
    }
}

allocator_buddies_system& allocator_buddies_system::operator=(allocator_buddies_system&& other) noexcept
{
    if (this != &other) {
        swap(*this, other);
    }
    return *this;
}


allocator_buddies_system::allocator_buddies_system(
    size_t                             space_size,
    std::pmr::memory_resource         *parent_allocator,
    logger                            *log,
    allocator_with_fit_mode::fit_mode  allocate_fit_mode)
{
    if (space_size < static_cast<size_t>(min_k)) {
        throw std::logic_error("allocator_buddies_system: space_size too small");
    }
    if (log) {
        log->log("allocator_buddies_system::allocator_buddies_system() called",
                 logger::severity::debug);
    }

    std::pmr::memory_resource *actual_parent =
        parent_allocator != nullptr
          ? parent_allocator
          : std::pmr::get_default_resource();

          const size_t region_size = 1ULL << space_size;
          const size_t raw_meta_size = meta_mutex_offset + sizeof(std::mutex);
          const size_t total_size = raw_meta_size + region_size;


    _trusted_memory = actual_parent->allocate(total_size);

    char *raw = static_cast<char*>(_trusted_memory);
    std::memset(raw, 0, total_size);

    {
        uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
        size_t mis = (raw_addr + raw_meta_size) % region_size;
        size_t padding = mis == 0 ? 0 : (region_size - mis);
        _meta_size = raw_meta_size + padding;
    }

    char *meta = raw;

    // *reinterpret_cast<logger**>(meta) = log;
    // meta += sizeof(logger*);

    // *reinterpret_cast<std::pmr::memory_resource**>(meta) = actual_parent;
    // meta += sizeof(std::pmr::memory_resource*);

    // *reinterpret_cast<fit_mode*>(meta) = allocate_fit_mode;
    // meta += sizeof(fit_mode);

    // *reinterpret_cast<unsigned char*>(meta) = static_cast<unsigned char>(space_size);
    // meta += sizeof(unsigned char);

    // new (meta) std::mutex();

    *reinterpret_cast<logger**>(meta) = log;            
    meta += sizeof(logger*);
    *reinterpret_cast<std::pmr::memory_resource**>(meta) = actual_parent; 
    meta += sizeof(std::pmr::memory_resource*);
    *reinterpret_cast<fit_mode*>(meta) = allocate_fit_mode;  
    meta += sizeof(fit_mode);
    *reinterpret_cast<unsigned char*>(meta) = static_cast<unsigned char>(space_size);

    void* mutex_addr = raw + meta_mutex_offset;
    new (mutex_addr) std::mutex();

    auto *first_blk = reinterpret_cast<block_metadata*>(
        static_cast<char*>(_trusted_memory) + _meta_size
    );
    first_blk->occupied = false;
    first_blk->size     = static_cast<unsigned char>(space_size);

    if (log) {
        log->log("Initialized allocator with "
                 + std::to_string(region_size - free_block_metadata_size)
                 + " bytes available",
                 logger::severity::information);
        log->log("allocator_buddies_system::allocator_buddies_system() finished",
                 logger::severity::debug);
    }
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(
    size_t size)
{
    if (get_logger() != nullptr)
    {
        std::string log_msg = "allocator_buddies_system::do_allocate_sm(";
        log_msg += std::to_string(size);
        log_msg += ") started";
        get_logger()->log(log_msg, logger::severity::debug);
    }
    
    //void* mutex_ptr = static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(unsigned char);
    void* mutex_ptr = static_cast<char*>(_trusted_memory) + meta_mutex_offset;
    std::mutex* mtx = static_cast<std::mutex*>(mutex_ptr);
    
    std::lock_guard<std::mutex> lock(*mtx);
    
    void* memory_start = static_cast<char*>(_trusted_memory) + _meta_size;

    size_t requested_size = size + occupied_block_metadata_size;
    size_t min_power_required = __detail::nearest_greater_k_of_2(requested_size);
    min_power_required = std::max(min_power_required, min_k);
    
    unsigned char max_power = *reinterpret_cast<unsigned char*>(
        static_cast<char*>(_trusted_memory)
      + sizeof(logger*)
      + sizeof(std::pmr::memory_resource*)
      + sizeof(fit_mode)
    );
    if (min_power_required > max_power)
    {
        if (get_logger() != nullptr)
        {
            std::string error_msg = "Cannot allocate ";
            error_msg += std::to_string(size);
            error_msg += " bytes (requires 2^";
            error_msg += std::to_string(min_power_required);
            error_msg += " bytes)";
            get_logger()->log(error_msg, logger::severity::error);
        }
        throw std::bad_alloc();
    }
    
    fit_mode mode = *reinterpret_cast<fit_mode*>(
        static_cast<char*>(_trusted_memory)
      + sizeof(logger*)
      + sizeof(std::pmr::memory_resource*)
    );
    buddy_iterator best_fit = end();
    size_t best_fit_size = SIZE_MAX;
    buddy_iterator worst_fit = end();
    size_t worst_fit_size = 0;
    
    for (auto it = begin(); it != end(); ++it)
    {
        if (!it.occupied() && it.size() >= min_power_required)
        {
            if (mode == fit_mode::first_fit)
            {
                best_fit = it;
                break;
            }
            else if (mode == fit_mode::the_best_fit)
            {
                if (it.size() < best_fit_size)
                {
                    best_fit = it;
                    best_fit_size = it.size();
                }
            }
            else if (mode == fit_mode::the_worst_fit)
            {
                if (it.size() > worst_fit_size)
                {
                    worst_fit = it;
                    worst_fit_size = it.size();
                }
            }
        }
    }
    
    buddy_iterator selected_block = (mode == fit_mode::the_worst_fit) ? worst_fit : best_fit;
    
    if (selected_block == end())
    {
        if (get_logger() != nullptr)
        {
            std::string error_msg = "Failed to allocate ";
            error_msg += std::to_string(size);
            error_msg += " bytes: no suitable block found";
            get_logger()->log(error_msg, logger::severity::error);
        }
        throw std::bad_alloc();
    }
    
    void* block_ptr = *selected_block;
    unsigned char original_size = static_cast<block_metadata*>(block_ptr)->size;
    size_t block_size = 1ULL << static_cast<block_metadata*>(block_ptr)->size;
    if ((reinterpret_cast<uintptr_t>(block_ptr) - reinterpret_cast<uintptr_t>(memory_start)) % block_size != 0) {
        throw std::invalid_argument("Invalid block address");
    }
    while (static_cast<block_metadata*>(block_ptr)->size > min_power_required)
    {
        unsigned char current_size = static_cast<block_metadata*>(block_ptr)->size;
        
        unsigned char new_size = current_size - 1;
        
        size_t block_size = 1ULL << current_size;
        void* buddy_ptr = static_cast<char*>(block_ptr) + (block_size >> 1);
        
        static_cast<block_metadata*>(block_ptr)->size = new_size;
        
        block_metadata* buddy_block = static_cast<block_metadata*>(buddy_ptr);
        buddy_block->occupied = false;
        buddy_block->size = new_size;
    }
    
    static_cast<block_metadata*>(block_ptr)->occupied = true;
    
    if (get_logger() != nullptr)
    {
        size_t available_memory = 0;
        for (auto it = begin(); it != end(); ++it)
        {
            if (!it.occupied())
            {
                available_memory += (1ULL << it.size()) - free_block_metadata_size;
            }
        }
        
        std::string success_msg = "Successfully allocated ";
        success_msg += std::to_string(size);
        success_msg += " bytes (block size: ";
        success_msg += std::to_string(1ULL << static_cast<block_metadata*>(block_ptr)->size);
        success_msg += " bytes)";
        get_logger()->log(success_msg, logger::severity::debug);
        
        std::string avail_msg = "Available memory: ";
        avail_msg += std::to_string(available_memory);
        avail_msg += " bytes";
        get_logger()->log(avail_msg, logger::severity::information);
        
        std::string memory_state;
        for (auto it = begin(); it != end(); ++it)
        {
            memory_state += (it.occupied() ? "occup " : "avail ");
            size_t block_data_size = (1ULL << it.size()) - (it.occupied() ? occupied_block_metadata_size : free_block_metadata_size);
            memory_state += std::to_string(block_data_size);
            memory_state += "|";
        }
        get_logger()->log("Memory state: " + memory_state, logger::severity::debug);
        
        if (original_size > min_power_required)
        {
            size_t requested = 1ULL << min_power_required;
            size_t allocated = 1ULL << static_cast<block_metadata*>(block_ptr)->size;
            std::string warning_msg = "User requested ";
            warning_msg += std::to_string(size);
            warning_msg += " bytes, but allocated ";
            warning_msg += std::to_string(allocated - occupied_block_metadata_size);
            warning_msg += " bytes due to block size constraints";
            get_logger()->log(warning_msg, logger::severity::warning);
        }
        
        std::string complete_msg = "allocator_buddies_system::do_allocate_sm(";
        complete_msg += std::to_string(size);
        complete_msg += ") completed";
        get_logger()->log(complete_msg, logger::severity::debug);
    }
    
    return static_cast<char*>(block_ptr) + sizeof(block_metadata);
}

void allocator_buddies_system::do_deallocate_sm(void *at)
{
    if (get_logger() != nullptr)
    {
        std::string log_msg = "allocator_buddies_system::do_deallocate_sm(";
        log_msg += std::to_string(reinterpret_cast<uintptr_t>(at));
        log_msg += ") started";
        get_logger()->log(log_msg, logger::severity::debug);
    }
    
    if (at == nullptr)
    {
        if (get_logger() != nullptr)
            get_logger()->log("Attempt to deallocate nullptr", logger::severity::warning);
        return;
    }
    
    char* trusted_mem = static_cast<char*>(_trusted_memory);
    std::mutex* mtx = reinterpret_cast<std::mutex*>(
        trusted_mem + meta_mutex_offset
    );
    
    std::lock_guard<std::mutex> lock(*mtx);
    
    block_metadata* block = reinterpret_cast<block_metadata*>(
        static_cast<char*>(at) - sizeof(block_metadata)
    );

    char* memory_start = trusted_mem + _meta_size;
    unsigned char max_power = *reinterpret_cast<unsigned char*>(
        trusted_mem + 
        sizeof(logger*) + 
        sizeof(std::pmr::memory_resource*) + 
        sizeof(fit_mode)
    );
    char* memory_end = memory_start + (1ULL << max_power);
    
    if (reinterpret_cast<char*>(block) < memory_start || 
        reinterpret_cast<char*>(block) >= memory_end)
    {
        if (get_logger() != nullptr)
        {
            std::string error_msg = "Attempt to deallocate memory not owned by this allocator: ";
            error_msg += std::to_string(reinterpret_cast<uintptr_t>(at));
            get_logger()->log(error_msg, logger::severity::error);
        }
        throw std::invalid_argument("Memory not owned by this allocator");
    }

    if (!block->occupied)
    {
        if (get_logger() != nullptr)
        {
            std::string error_msg = "Attempt to deallocate already freed memory: ";
            error_msg += std::to_string(reinterpret_cast<uintptr_t>(at));
            get_logger()->log(error_msg, logger::severity::error);
        }
        throw std::invalid_argument("Double free detected");
    }
    
    block->occupied = false;
    bool merged;
    do
    {
        merged = false;
        size_t block_size = 1ULL << block->size;
        auto block_addr = reinterpret_cast<std::uintptr_t>(block);
        auto buddy_addr = block_addr ^ block_size;
        char* buddy_ptr = reinterpret_cast<char*>(buddy_addr);
        if (buddy_ptr < memory_start || buddy_ptr >= memory_end)
            break;
        
        block_metadata* buddy = reinterpret_cast<block_metadata*>(buddy_ptr);
        
        if (!buddy->occupied && buddy->size == block->size)
        {
            block_metadata* merged_block = (block < buddy) ? block : buddy;
            
            merged_block->size++;
            
            block = merged_block;
            merged = true;
        }
    } while (merged);
    
    if (get_logger() != nullptr)
    {
        size_t available_memory = 0;
        for (auto it = begin(); it != end(); ++it)
        {
            if (!it.occupied())
            {
                available_memory += (1ULL << it.size()) - free_block_metadata_size;
            }
        }
        
        std::string success_msg = "Successfully deallocated memory at ";
        success_msg += std::to_string(reinterpret_cast<uintptr_t>(at));
        get_logger()->log(success_msg, logger::severity::debug);
        
        std::string avail_msg = "Available memory: ";
        avail_msg += std::to_string(available_memory);
        avail_msg += " bytes";
        get_logger()->log(avail_msg, logger::severity::information);
        
        std::string memory_state;
        for (auto it = begin(); it != end(); ++it)
        {
            memory_state += (it.occupied() ? "occup " : "avail ");
            size_t block_data_size = (1ULL << it.size()) - (it.occupied() ? occupied_block_metadata_size : free_block_metadata_size);
            memory_state += std::to_string(block_data_size);
            memory_state += "|";
        }
        get_logger()->log("Memory state: " + memory_state, logger::severity::debug);
        
        std::string complete_msg = "allocator_buddies_system::do_deallocate_sm(";
        complete_msg += std::to_string(reinterpret_cast<uintptr_t>(at));
        complete_msg += ") completed";
        get_logger()->log(complete_msg, logger::severity::debug);
    }
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other)
    : smart_mem_resource(other),
      allocator_with_fit_mode(other),
      logger_guardant(static_cast<const logger_guardant&>(other))
{
    if (get_logger() != nullptr)
    {
        get_logger()->log("allocator_buddies_system::allocator_buddies_system(copy) started", 
                         logger::severity::trace);
    }

    unsigned char k = *reinterpret_cast<unsigned char*>(
        static_cast<char*>(other._trusted_memory) + 
        sizeof(logger*) + 
        sizeof(std::pmr::memory_resource*) + 
        sizeof(fit_mode)
    );
    size_t total_memory_size = (1ULL << k) + _meta_size;

    std::pmr::memory_resource* parent = std::pmr::get_default_resource();
    _trusted_memory = parent->allocate(total_memory_size);

    std::memcpy(
        _trusted_memory,
        other._trusted_memory,
        sizeof(logger*) + 
        sizeof(std::pmr::memory_resource*) + 
        sizeof(fit_mode) + 
        sizeof(unsigned char)
    );

    new (static_cast<char*>(_trusted_memory) + 
        sizeof(logger*) + 
        sizeof(std::pmr::memory_resource*) + 
        sizeof(fit_mode) + 
        sizeof(unsigned char)) std::mutex();

    void* src = static_cast<char*>(other._trusted_memory) + _meta_size;
    void* dst = static_cast<char*>(_trusted_memory) + _meta_size;
    
    std::memcpy(dst, src, (1ULL << k));

    if (get_logger() != nullptr)
    {
        get_logger()->log("allocator_buddies_system::allocator_buddies_system(copy) completed", 
                         logger::severity::trace);
    }
}

allocator_buddies_system& allocator_buddies_system::operator=(const allocator_buddies_system& other)
{
    if (this != &other)
    {
        if (get_logger() != nullptr)
        {
            get_logger()->log("allocator_buddies_system::operator=(copy) started", logger::severity::trace);
        }

        allocator_buddies_system temp(other);

        swap(*this, temp);

        if (get_logger() != nullptr)
        {
            get_logger()->log("allocator_buddies_system::operator=(copy) completed", logger::severity::trace);
        }
    }
    return *this;
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    const allocator_buddies_system* other_allocator = dynamic_cast<const allocator_buddies_system*>(&other);
    if (other_allocator == nullptr)
        return false;
    
    return _trusted_memory == other_allocator->_trusted_memory;
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    if (get_logger() != nullptr)
    {
        std::string log_msg = "allocator_buddies_system::set_fit_mode(";
        log_msg += std::to_string(static_cast<int>(mode));
        log_msg += ") started";
        get_logger()->log(log_msg, logger::severity::trace);
    }
    
    //void* mutex_ptr = static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(unsigned char);
    void* mutex_ptr = static_cast<char*>(_trusted_memory) + meta_mutex_offset;
    std::mutex* mtx = static_cast<std::mutex*>(mutex_ptr);
    
    std::lock_guard<std::mutex> lock(*mtx);
    
    void* fit_mode_ptr = static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*);
    *(fit_mode*)(fit_mode_ptr) = mode;
    
    if (get_logger() != nullptr)
    {
        get_logger()->log("allocator_buddies_system::set_fit_mode() completed", logger::severity::trace);
    }
}


std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    if (get_logger() != nullptr)
    {
        get_logger()->log("allocator_buddies_system::get_blocks_info() started", logger::severity::trace);
    }
    
    std::vector<allocator_test_utils::block_info> result = get_blocks_info_inner();
    
    if (get_logger() != nullptr)
    {
        get_logger()->log("allocator_buddies_system::get_blocks_info() completed", logger::severity::trace);
    }
    
    return result;
}

inline logger *allocator_buddies_system::get_logger() const
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }
    return *(logger**)(_trusted_memory);
}

inline std::string allocator_buddies_system::get_typename() const
{
    return "allocator_buddies_system";
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> blocks;
    
    if (_trusted_memory == nullptr)
    {
        return blocks;
    }
    
    void* memory_start = static_cast<char*>(_trusted_memory) + _meta_size;
    size_t total_space = 1ULL << *reinterpret_cast<unsigned char*>(
        static_cast<char*>(_trusted_memory) + 
        sizeof(logger*) + 
        sizeof(std::pmr::memory_resource*) + 
        sizeof(fit_mode)
    );

    for (auto it = begin(); it != end(); ++it)
    {
        block_info info;
        const size_t block_total_size = 1ULL << it.size();
        info.block_size = block_total_size;
        info.is_block_occupied = it.occupied();
        
        blocks.push_back(info);
    }
    
    return blocks;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept

{
    void* memory_start = static_cast<char*>(_trusted_memory) + _meta_size;
    return buddy_iterator(memory_start);
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept
{
    if (_trusted_memory == nullptr) {
        return buddy_iterator(nullptr);
    }

    char* metadata_start = static_cast<char*>(_trusted_memory);
    
    char* max_power_ptr = metadata_start + 
                         sizeof(logger*) + 
                         sizeof(std::pmr::memory_resource*) + 
                         sizeof(fit_mode);
    
    unsigned char max_power = *reinterpret_cast<unsigned char*>(max_power_ptr);
    
    void* memory_start = metadata_start + _meta_size;
    void* memory_end = static_cast<char*>(memory_start) + (1ULL << max_power);
    
    return buddy_iterator(memory_end);
}

bool allocator_buddies_system::buddy_iterator::operator==(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return _block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return _block != other._block;
}

allocator_buddies_system::buddy_iterator &allocator_buddies_system::buddy_iterator::operator++() & noexcept
{
    if (_block == nullptr) return *this;
    block_metadata* metadata = static_cast<block_metadata*>(_block);
    size_t block_size = 1ULL << metadata->size;
    _block = static_cast<char*>(_block) + block_size;
    return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int n)
{
    buddy_iterator temp = *this;
    ++(*this); 
    return temp;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept
{
    if (_block == nullptr)
        return 0;
    
    block_metadata* metadata = static_cast<block_metadata*>(_block);
    return metadata->size;
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept
{
    if (_block == nullptr)
        return false;
    
    block_metadata* metadata = static_cast<block_metadata*>(_block);
    return metadata->occupied;
}

void *allocator_buddies_system::buddy_iterator::operator*() const noexcept
{
    return _block;
}

allocator_buddies_system::buddy_iterator::buddy_iterator()
: _block(nullptr)
{}

void swap(allocator_buddies_system& a, allocator_buddies_system& b) noexcept
{
    using std::swap;
    
    swap(a._trusted_memory, b._trusted_memory);
    
    static_cast<smart_mem_resource&>(a) = static_cast<smart_mem_resource&&>(b);
    static_cast<smart_mem_resource&>(b) = static_cast<smart_mem_resource&&>(a);
    
    static_cast<allocator_with_fit_mode&>(a) = static_cast<allocator_with_fit_mode&&>(b);
    static_cast<allocator_with_fit_mode&>(b) = static_cast<allocator_with_fit_mode&&>(a);
    
    static_cast<logger_guardant&>(a) = static_cast<logger_guardant&&>(b);
    static_cast<logger_guardant&>(b) = static_cast<logger_guardant&&>(a);
}