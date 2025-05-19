#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <pp_allocator.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <iterator>
#include <mutex>
#include <cstring>
#include <string>

struct block_header {
    size_t size;       
    bool occupied;     
    block_header* prev;
    block_header* next; 
    
    //alignas(alignof(void*)) char _padding[7];
    //alignas(8) char _padding[4];

};
    
struct allocator_metadata {
    logger* log;
    std::pmr::memory_resource* parent_allocator;
    allocator_with_fit_mode::fit_mode mode;
    size_t space_size;       
    std::mutex mutex;         
    block_header* first_block; 
};

class allocator_boundary_tags final :
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:

    static block_header* find_first_fit(block_header* first_block, size_t size);
    static block_header* find_best_fit(block_header* first_block, size_t size);
    static block_header* find_worst_fit(block_header* first_block, size_t size);
    static void split_block(block_header* block, size_t size);
    static void merge_blocks(block_header* block);
    static bool is_pointer_in_trusted_memory(void* ptr, void* trusted_memory, size_t space_size);
    static std::string get_memory_state_string(block_header* first_block);
    static size_t get_available_memory(block_header* first_block);


    /**
     * TODO: You must improve it for alignment support
     */
    static constexpr const size_t allocator_metadata_size = sizeof(allocator_metadata);

    static constexpr const size_t occupied_block_metadata_size = sizeof(block_header);

    static constexpr const size_t free_block_metadata_size = 0;

    void *_trusted_memory;

public:
    
    ~allocator_boundary_tags() override;
    
    allocator_boundary_tags(allocator_boundary_tags const &other);
    
    allocator_boundary_tags &operator=(allocator_boundary_tags const &other);
    
    allocator_boundary_tags(
        allocator_boundary_tags &&other) noexcept;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags &&other) noexcept;

public:
    
    explicit allocator_boundary_tags(
            size_t space_size,
            std::pmr::memory_resource *parent_allocator = nullptr,
            logger *logger = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:
    
    [[nodiscard]] void *do_allocate_sm(
        size_t bytes) override;
    
    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

public:
    
    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;

public:
    
    std::vector<allocator_test_utils::block_info> get_blocks_info() const override;

private:

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;

/** TODO: Highly recommended for helper functions to return references */

    inline logger *get_logger() const override;

    std::mutex& get_mutex() const noexcept;

    inline std::string get_typename() const noexcept override;

    void swap(allocator_boundary_tags& a, allocator_boundary_tags& b) noexcept;
    
    class boundary_iterator
    {
        void* _occupied_ptr;
        bool _occupied;
        void* _trusted_memory;

    public:

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = void*;
        using reference = void*&;
        using pointer = void**;
        using difference_type = ptrdiff_t;

        bool operator==(const boundary_iterator&) const noexcept;

        bool operator!=(const boundary_iterator&) const noexcept;

        boundary_iterator& operator++() & noexcept;

        boundary_iterator& operator--() & noexcept;

        boundary_iterator operator++(int n);

        boundary_iterator operator--(int n);

        size_t size() const noexcept;

        bool occupied() const noexcept;

        void* operator*() const noexcept;

        void* get_ptr() const noexcept;

        boundary_iterator();

        boundary_iterator(void* trusted);
    };

    friend class boundary_iterator;

    boundary_iterator begin() const noexcept;

    boundary_iterator end() const noexcept;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H