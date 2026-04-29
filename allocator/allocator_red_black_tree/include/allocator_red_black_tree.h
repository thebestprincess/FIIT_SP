#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H

#include <pp_allocator.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <mutex>


namespace rb_tree_utils
{
    constexpr size_t align_size(size_t size) noexcept
    { 
        return (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1); 
    }
}


class allocator_red_black_tree final:
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode
{
private:

    enum class block_color : unsigned char
    { RED, BLACK };

    struct block_data
    {
        bool occupied : 4;
        block_color color : 4;
    };

    void *_trusted_memory;

    static constexpr const size_t allocator_metadata_size = sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex) + sizeof(void*);
    static constexpr const size_t occupied_block_metadata_size = sizeof(block_data) + 3 * sizeof(void*);
    static constexpr const size_t free_block_metadata_size = sizeof(block_data) + 5 * sizeof(void*);

public:
    
    ~allocator_red_black_tree() override;
    
    allocator_red_black_tree(allocator_red_black_tree const &other);
    
    allocator_red_black_tree &operator=(allocator_red_black_tree const &other) = delete;
    
    allocator_red_black_tree(
        allocator_red_black_tree &&other) noexcept;
    
    allocator_red_black_tree &operator=(
        allocator_red_black_tree &&other) noexcept;

public:
    
    explicit allocator_red_black_tree(
            size_t space_size,
            std::pmr::memory_resource *parent_allocator = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);


private:

    static constexpr size_t alloc_parent_offset = 0;
    static constexpr size_t alloc_space_size_offset = rb_tree_utils::align_size(alloc_parent_offset + sizeof(std::pmr::memory_resource*));
    static constexpr size_t alloc_root_offset = rb_tree_utils::align_size(alloc_space_size_offset + sizeof(size_t));
    static constexpr size_t alloc_mutex_offset = rb_tree_utils::align_size(alloc_root_offset + sizeof(void*));
    static constexpr size_t alloc_fit_mode_offset = rb_tree_utils::align_size(alloc_mutex_offset + sizeof(std::mutex));
    
    static constexpr size_t ALIGNED_ALLOC_META_SIZE = rb_tree_utils::align_size(alloc_fit_mode_offset + sizeof(allocator_with_fit_mode::fit_mode));

    static constexpr size_t block_data_offset = 0;
    static constexpr size_t block_size_offset = rb_tree_utils::align_size(block_data_offset + sizeof(block_data));
    static constexpr size_t block_prev_phys_offset = rb_tree_utils::align_size(block_size_offset + sizeof(size_t));
    
    static constexpr size_t block_allocator_ptr_offset = rb_tree_utils::align_size(block_prev_phys_offset + sizeof(void*));
    static constexpr size_t ALIGNED_OCCUPIED_META_SIZE = rb_tree_utils::align_size(block_allocator_ptr_offset + sizeof(void*));

    static constexpr size_t block_parent_offset = rb_tree_utils::align_size(block_prev_phys_offset + sizeof(void*));
    static constexpr size_t block_left_offset = rb_tree_utils::align_size(block_parent_offset + sizeof(void*));
    static constexpr size_t block_right_offset = rb_tree_utils::align_size(block_left_offset + sizeof(void*));
    static constexpr size_t ALIGNED_FREE_META_SIZE = rb_tree_utils::align_size(block_right_offset + sizeof(void*));

    static std::pmr::memory_resource** get_parent_allocator_ptr(void* trusted) noexcept;
    static size_t* get_space_size_ptr(void* trusted) noexcept;
    static void** get_tree_root_ptr(void* trusted) noexcept;
    static std::mutex* get_mutex_ptr(void* trusted) noexcept;
    static allocator_with_fit_mode::fit_mode* get_fit_mode_ptr(void* trusted) noexcept;

    static block_data* get_block_data_ptr(void* header) noexcept;
    static size_t* get_block_size_ptr(void* header) noexcept;
    static void** get_prev_physical_ptr(void* header) noexcept;
    
    static void** get_allocator_ptr(void* header) noexcept;
    static void** get_parent_ptr(void* header) noexcept;
    static void** get_left_ptr(void* header) noexcept;
    static void** get_right_ptr(void* header) noexcept;
    
    static void* get_block_payload_ptr(void* header) noexcept;

private:

    static void set_color(void* node, block_color color) noexcept;
    static block_color get_color(void* node) noexcept;
    static void rotate_left(void* trusted, void* node) noexcept;
    static void rotate_right(void* trusted, void* node) noexcept;
    static void transplant(void* trusted, void* old_node, void* new_node) noexcept;
    static void* get_min_node(void* node) noexcept;
    static void insert_node(void* trusted, void* new_node) noexcept;
    static void insert_fixup(void* trusted, void* node) noexcept;
    static void remove_node(void* trusted, void* target_node) noexcept;
    static void remove_fixup(void* trusted, void* current_node, void* parent_node) noexcept;    

private:

    static void* find_first_fit(void* trusted, size_t size);
    static void* find_best_fit(void* trusted, size_t size);
    static void* find_worst_fit(void* trusted, size_t size);

private:
    
    [[nodiscard]] void *do_allocate_sm(
        size_t size) override;
    
    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource&) const noexcept override;

    std::vector<allocator_test_utils::block_info> get_blocks_info() const override;
    
    inline void set_fit_mode(allocator_with_fit_mode::fit_mode mode) override;

private:

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;

    class rb_iterator
    {
        void* _block_ptr;
        void* _trusted;

    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = void*;
        using reference = void*&;
        using pointer = void**;
        using difference_type = ptrdiff_t;

        bool operator==(const rb_iterator&) const noexcept;

        bool operator!=(const rb_iterator&) const noexcept;

        rb_iterator& operator++() & noexcept;

        rb_iterator operator++(int n);

        size_t size() const noexcept;

        void* operator*() const noexcept;

        bool occupied()const noexcept;

        rb_iterator();

        rb_iterator(void* trusted);
    };

    friend class rb_iterator;

    rb_iterator begin() const noexcept;
    rb_iterator end() const noexcept;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H