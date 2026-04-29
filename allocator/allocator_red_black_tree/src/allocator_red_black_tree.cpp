#include <not_implemented.h>

#include "../include/allocator_red_black_tree.h"


/**
 * get metadata
 */


std::pmr::memory_resource** allocator_red_black_tree::get_parent_allocator_ptr(void* trusted) noexcept
{
    return reinterpret_cast<std::pmr::memory_resource**>
    (
        static_cast<char*>(trusted) + alloc_parent_offset
    );
}

size_t* allocator_red_black_tree::get_space_size_ptr(void* trusted) noexcept
{
    return reinterpret_cast<size_t*>
    (
        static_cast<char*>(trusted) + alloc_space_size_offset
    );
}

void** allocator_red_black_tree::get_tree_root_ptr(void* trusted) noexcept
{
    return reinterpret_cast<void**>(static_cast<char*>(trusted) + alloc_root_offset);
}

std::mutex* allocator_red_black_tree::get_mutex_ptr(void* trusted) noexcept
{
    return reinterpret_cast<std::mutex*>(static_cast<char*>(trusted) + alloc_mutex_offset);
}
allocator_with_fit_mode::fit_mode* allocator_red_black_tree::get_fit_mode_ptr(void* trusted) noexcept
{
    return reinterpret_cast<allocator_with_fit_mode::fit_mode*>
    (
        static_cast<char*>(trusted) + alloc_fit_mode_offset
    );
}

allocator_red_black_tree::block_data* allocator_red_black_tree::get_block_data_ptr(void* header) noexcept
{
    return reinterpret_cast<block_data*>
    (
        static_cast<char*>(header) + block_data_offset
    );
}

size_t* allocator_red_black_tree::get_block_size_ptr(void* header) noexcept
{
    return reinterpret_cast<size_t*>(static_cast<char*>(header) + block_size_offset);
}

void** allocator_red_black_tree::get_prev_physical_ptr(void* header) noexcept
{
    return reinterpret_cast<void**>(static_cast<char*>(header) + block_prev_phys_offset);
}

void** allocator_red_black_tree::get_allocator_ptr(void* header) noexcept
{
    return reinterpret_cast<void**>(static_cast<char*>(header) + block_allocator_ptr_offset);
}

void** allocator_red_black_tree::get_parent_ptr(void* header) noexcept 
{
    return reinterpret_cast<void**>(static_cast<char*>(header) + block_parent_offset);
}

void** allocator_red_black_tree::get_left_ptr(void* header) noexcept
{
    return reinterpret_cast<void**>(static_cast<char*>(header) + block_left_offset);
}

void** allocator_red_black_tree::get_right_ptr(void* header) noexcept
{
    return reinterpret_cast<void**>(static_cast<char*>(header) + block_right_offset);
}

void* allocator_red_black_tree::get_block_payload_ptr(void* header) noexcept
{
    bool is_occupied = get_block_data_ptr(header)->occupied;
    size_t meta_size = is_occupied ? ALIGNED_OCCUPIED_META_SIZE : ALIGNED_FREE_META_SIZE;
    return static_cast<char*>(header) + meta_size;
}






/**
 * RB Tree
 */

void allocator_red_black_tree::set_color(void* node, block_color color) noexcept
{
    if (node) get_block_data_ptr(node)->color = color;
}

allocator_red_black_tree::block_color allocator_red_black_tree::get_color(void* node) noexcept
{
    return node ? get_block_data_ptr(node)->color : block_color::BLACK;
}

void* allocator_red_black_tree::get_min_node(void* node) noexcept
{
    while (*get_left_ptr(node) != nullptr) node = *get_left_ptr(node);
    return node;
}

void allocator_red_black_tree::transplant(void* trusted, void* old_node, void* new_node) noexcept {
    void* parent = *get_parent_ptr(old_node);
    
    if (parent == nullptr) {
        *get_tree_root_ptr(trusted) = new_node;
    } else if (old_node == *get_left_ptr(parent)) {
        *get_left_ptr(parent) = new_node;
    } else {
        *get_right_ptr(parent) = new_node;
    }
    
    if (new_node != nullptr) {
        *get_parent_ptr(new_node) = parent;
    }
}


void allocator_red_black_tree::rotate_left(void* trusted, void* node) noexcept
{
    void* right_child = *get_right_ptr(node);
    *get_right_ptr(node) = *get_left_ptr(right_child);

    if (*get_left_ptr(right_child)) *get_parent_ptr(*get_left_ptr(right_child)) = node;

    void* parent = *get_parent_ptr(node);
    *get_parent_ptr(right_child) = parent;

    if (parent == nullptr) *get_tree_root_ptr(trusted) = right_child;
    else if (node == *get_left_ptr(parent)) *get_left_ptr(parent) = right_child;
    else *get_right_ptr(parent) = right_child;

    *get_left_ptr(right_child) = node;
    *get_parent_ptr(node) = right_child;
}

void allocator_red_black_tree::rotate_right(void* trusted, void* node) noexcept
{
    void* left_child = *get_left_ptr(node);
    *get_left_ptr(node) = *get_right_ptr(left_child);

    if (*get_right_ptr(left_child) != nullptr) *get_parent_ptr(*get_right_ptr(left_child)) = node;

    void* parent = *get_parent_ptr(node);
    *get_parent_ptr(left_child) = parent;

    if (parent == nullptr) *get_tree_root_ptr(trusted) = left_child;
    else if (node == *get_right_ptr(parent)) *get_right_ptr(parent) = left_child;
    else *get_left_ptr(parent) = left_child;

    *get_right_ptr(left_child) = node;
    *get_parent_ptr(node) = left_child;
}


void allocator_red_black_tree::insert_fixup(void* trusted, void* current) noexcept
{
    while (current != *get_tree_root_ptr(trusted) && get_color(*get_parent_ptr(current)) == block_color::RED)
    {
        void* parent = *get_parent_ptr(current);
        void* grandparent = *get_parent_ptr(parent);

        if (parent == *get_left_ptr(grandparent))
        {
            void* uncle = *get_right_ptr(grandparent);
            
            if (get_color(uncle) == block_color::RED)
            {
                set_color(parent, block_color::BLACK);
                set_color(uncle, block_color::BLACK);
                set_color(grandparent, block_color::RED);
                current = grandparent;
            }
            else
            {
                if (current == *get_right_ptr(parent))
                {
                    current = parent;
                    rotate_left(trusted, current);
                    parent = *get_parent_ptr(current);
                    grandparent = *get_parent_ptr(parent);
                }

                set_color(parent, block_color::BLACK);
                set_color(grandparent, block_color::RED);
                rotate_right(trusted, grandparent);
            }
        } 
        else
        {
            void* uncle = *get_left_ptr(grandparent);
            
            if (get_color(uncle) == block_color::RED) 
            {
                set_color(parent, block_color::BLACK);
                set_color(uncle, block_color::BLACK);
                set_color(grandparent, block_color::RED);
                current = grandparent;
            }
            else
            {
                if (current == *get_left_ptr(parent))
                {
                    current = parent;
                    rotate_right(trusted, current);
                    parent = *get_parent_ptr(current);
                    grandparent = *get_parent_ptr(parent);
                }

                set_color(parent, block_color::BLACK);
                set_color(grandparent, block_color::RED);
                rotate_left(trusted, grandparent);
            }
        }
    }

    set_color(*get_tree_root_ptr(trusted), block_color::BLACK);
}

void allocator_red_black_tree::insert_node(void* trusted, void* new_node) noexcept
{
    void* current = *get_tree_root_ptr(trusted);
    void* parent = nullptr;
    
    size_t new_size = *get_block_size_ptr(new_node);

    while (current != nullptr)
    {
        parent = current;
        size_t current_size = *get_block_size_ptr(current);
        
        if (new_size < current_size) current = *get_left_ptr(current);
        else if (new_size > current_size) current = *get_right_ptr(current);
        else
        { 
            if (new_node < current) current = *get_left_ptr(current);
            else current = *get_right_ptr(current);
        }
    }

    *get_parent_ptr(new_node) = parent;
    *get_left_ptr(new_node) = nullptr;
    *get_right_ptr(new_node) = nullptr;
    set_color(new_node, block_color::RED); 

    if (!parent)
    {
        *get_tree_root_ptr(trusted) = new_node;
    }
    else
    {
        size_t parent_size = *get_block_size_ptr(parent);
        if (new_size < parent_size || (new_size == parent_size && new_node < parent)) *get_left_ptr(parent) = new_node;
        else *get_right_ptr(parent) = new_node;
    }

    insert_fixup(trusted, new_node);
}


void allocator_red_black_tree::remove_fixup(void* trusted, void* current, void* parent) noexcept
{
    while (current != *get_tree_root_ptr(trusted) && get_color(current) == block_color::BLACK)
    {
        if (current == *get_left_ptr(parent))
        {
            void* sibling = *get_right_ptr(parent);
            
            if (get_color(sibling) == block_color::RED)
            {
                set_color(sibling, block_color::BLACK);
                set_color(parent, block_color::RED);
                rotate_left(trusted, parent);
                sibling = *get_right_ptr(parent);
            }
            
            if (get_color(*get_left_ptr(sibling)) == block_color::BLACK &&
                get_color(*get_right_ptr(sibling)) == block_color::BLACK)
            {
                set_color(sibling, block_color::RED);
                current = parent;
                parent = *get_parent_ptr(current);
            }
            else
            {
                if (get_color(*get_right_ptr(sibling)) == block_color::BLACK)
                {
                    set_color(*get_left_ptr(sibling), block_color::BLACK);
                    set_color(sibling, block_color::RED);
                    rotate_right(trusted, sibling);
                    sibling = *get_right_ptr(parent);
                }

                set_color(sibling, get_color(parent));
                set_color(parent, block_color::BLACK);
                set_color(*get_right_ptr(sibling), block_color::BLACK);
                rotate_left(trusted, parent);
                current = *get_tree_root_ptr(trusted);
            }
        }
        else
        {
            void* sibling = *get_left_ptr(parent);
            
            if (get_color(sibling) == block_color::RED)
            {
                set_color(sibling, block_color::BLACK);
                set_color(parent, block_color::RED);
                rotate_right(trusted, parent);
                sibling = *get_left_ptr(parent);
            }
            
            if (get_color(*get_right_ptr(sibling)) == block_color::BLACK &&
                get_color(*get_left_ptr(sibling)) == block_color::BLACK)
            {
                set_color(sibling, block_color::RED);
                current = parent;
                parent = *get_parent_ptr(current);
            }
            else
            {
                if (get_color(*get_left_ptr(sibling)) == block_color::BLACK)
                {
                    set_color(*get_right_ptr(sibling), block_color::BLACK);
                    set_color(sibling, block_color::RED);
                    rotate_left(trusted, sibling);
                    sibling = *get_left_ptr(parent);
                }

                set_color(sibling, get_color(parent));
                set_color(parent, block_color::BLACK);
                set_color(*get_left_ptr(sibling), block_color::BLACK);
                rotate_right(trusted, parent);
                current = *get_tree_root_ptr(trusted);
            }
        }
    }
    set_color(current, block_color::BLACK);
}

void allocator_red_black_tree::remove_node(void* trusted, void* target_node) noexcept
{
    void* replacement = nullptr;
    void* replacement_parent = nullptr;
    block_color original_color = get_color(target_node);

    if (*get_left_ptr(target_node) == nullptr)
    {
        replacement = *get_right_ptr(target_node);
        replacement_parent = *get_parent_ptr(target_node);
        transplant(trusted, target_node, replacement);
    }
    else if (*get_right_ptr(target_node) == nullptr)
    {
        replacement = *get_left_ptr(target_node);
        replacement_parent = *get_parent_ptr(target_node);
        transplant(trusted, target_node, replacement);
    }
    else
    {
        void* min_right_node = get_min_node(*get_right_ptr(target_node));
        original_color = get_color(min_right_node);
        replacement = *get_right_ptr(min_right_node);

        if (*get_parent_ptr(min_right_node) == target_node) 
        {
            replacement_parent = min_right_node;
        }
        else
        {
            replacement_parent = *get_parent_ptr(min_right_node);
            transplant(trusted, min_right_node, replacement);
            *get_right_ptr(min_right_node) = *get_right_ptr(target_node);
            *get_parent_ptr(*get_right_ptr(min_right_node)) = min_right_node;
        }

        transplant(trusted, target_node, min_right_node);
        *get_left_ptr(min_right_node) = *get_left_ptr(target_node);
        *get_parent_ptr(*get_left_ptr(min_right_node)) = min_right_node;
        set_color(min_right_node, get_color(target_node));
    }

    if (original_color == block_color::BLACK)
    {
        remove_fixup(trusted, replacement, replacement_parent);
    }
}






/**
 * Search strategies
 */

void* allocator_red_black_tree::find_best_fit(void* trusted, size_t size)
{
    void* target = nullptr;
    void* current = *get_tree_root_ptr(trusted);

    while (current)
    {
        size_t current_size = *get_block_size_ptr(current);
        
        if (current_size >= size)
        {
            target = current;
            current = *get_left_ptr(current);
        }
        else
        {
            current = *get_right_ptr(current);
        }
    }

    return target;
}

void* allocator_red_black_tree::find_worst_fit(void* trusted, size_t size)
{
    void* target = nullptr;
    void* current = *get_tree_root_ptr(trusted);

    while (current && *get_right_ptr(current)) current = *get_right_ptr(current);
    if (current && *get_block_size_ptr(current) >= size) target = current;

    return target;
}

void* allocator_red_black_tree::find_first_fit(void* trusted, size_t physical_size)
{
    return find_best_fit(trusted, physical_size);
}











/**
 * RB Allocator implementation
 */

allocator_red_black_tree::allocator_red_black_tree(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < ALIGNED_ALLOC_META_SIZE + ALIGNED_FREE_META_SIZE + 1)
        throw std::invalid_argument("Size is too small");

    std::pmr::memory_resource* parent = parent_allocator ? parent_allocator : std::pmr::get_default_resource();

    try { _trusted_memory = parent->allocate(space_size + ALIGNED_ALLOC_META_SIZE); }
    catch (...) { throw std::bad_alloc(); }

    *get_parent_allocator_ptr(_trusted_memory) = parent;
    *get_fit_mode_ptr(_trusted_memory) = allocate_fit_mode;
    *get_space_size_ptr(_trusted_memory) = space_size;

    new (get_mutex_ptr(_trusted_memory)) std::mutex();

    void* first_block = static_cast<char*>(_trusted_memory) + ALIGNED_ALLOC_META_SIZE;
    *get_tree_root_ptr(_trusted_memory) = first_block; 

    size_t usable_block_size = space_size - ALIGNED_FREE_META_SIZE;
    
    auto* data = get_block_data_ptr(first_block);
    data->occupied = false;
    data->color = block_color::BLACK;
    
    *get_block_size_ptr(first_block) = usable_block_size;
    *get_prev_physical_ptr(first_block) = nullptr;
    
    *get_parent_ptr(first_block) = nullptr;
    *get_left_ptr(first_block) = nullptr;
    *get_right_ptr(first_block) = nullptr;
}

allocator_red_black_tree::~allocator_red_black_tree()
{
    if (!_trusted_memory) return;
    std::pmr::memory_resource* parent = *get_parent_allocator_ptr(_trusted_memory);
    size_t space_size = *get_space_size_ptr(_trusted_memory);
    
    std::mutex* mtx = get_mutex_ptr(_trusted_memory);
    mtx->~mutex();
    
    parent->deallocate(_trusted_memory, space_size + ALIGNED_ALLOC_META_SIZE);
    _trusted_memory = nullptr;
}

allocator_red_black_tree::allocator_red_black_tree(allocator_red_black_tree &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_red_black_tree &allocator_red_black_tree::operator=(allocator_red_black_tree &&other) noexcept
{
    if (this == &other) return *this;
    
    if (_trusted_memory) this->~allocator_red_black_tree();
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
    
    return *this;
}

bool allocator_red_black_tree::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

[[nodiscard]] void *allocator_red_black_tree::do_allocate_sm(size_t size)
{
    size_t aligned_size = rb_tree_utils::align_size(size);

    if (aligned_size < ALIGNED_FREE_META_SIZE - ALIGNED_OCCUPIED_META_SIZE)
        aligned_size = ALIGNED_FREE_META_SIZE - ALIGNED_OCCUPIED_META_SIZE;
    
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    auto mode = *get_fit_mode_ptr(_trusted_memory);
    void* target_block = nullptr;
    void* root = *get_tree_root_ptr(_trusted_memory);

    size_t meta_diff = ALIGNED_FREE_META_SIZE - ALIGNED_OCCUPIED_META_SIZE; 
    if (aligned_size < meta_diff) aligned_size = meta_diff;
    
    size_t required_physical = aligned_size + ALIGNED_OCCUPIED_META_SIZE;
    size_t target_payload = required_physical - ALIGNED_FREE_META_SIZE;

    switch (mode)
    {
        case allocator_with_fit_mode::fit_mode::the_best_fit:
        {
            target_block = find_best_fit(_trusted_memory, target_payload);
            break;
        }
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
        {
            target_block = find_worst_fit(_trusted_memory, target_payload);
            break;
        }
        case allocator_with_fit_mode::fit_mode::first_fit:
        {
            target_block = find_first_fit(_trusted_memory, required_physical);
            break;
        }
    }

    if (!target_block) throw std::bad_alloc();

    remove_node(_trusted_memory, target_block);

    size_t found_size = *get_block_size_ptr(target_block);
    size_t available_physical = found_size + ALIGNED_FREE_META_SIZE;

    if (available_physical >= required_physical + ALIGNED_FREE_META_SIZE)
    {
        size_t remaining_physical = available_physical - required_physical;
        size_t new_free_size = remaining_physical - ALIGNED_FREE_META_SIZE;
        
        void* new_free = static_cast<char*>(target_block) + required_physical;

        get_block_data_ptr(new_free)->occupied = false;
        *get_block_size_ptr(new_free) = new_free_size;
        *get_prev_physical_ptr(new_free) = target_block;

        get_block_data_ptr(target_block)->occupied = true;
        *get_block_size_ptr(target_block) = aligned_size;
        *get_allocator_ptr(target_block) = this;

        char* next_phys = static_cast<char*>(new_free) + remaining_physical;
        size_t space_size = *get_space_size_ptr(_trusted_memory);
        char* end_of_memory = static_cast<char*>(_trusted_memory) + ALIGNED_ALLOC_META_SIZE + space_size;
        if (next_phys < end_of_memory) *get_prev_physical_ptr(next_phys) = new_free;
        
        insert_node(_trusted_memory, new_free);
    }
    else
    {
        get_block_data_ptr(target_block)->occupied = true;
        *get_block_size_ptr(target_block) = available_physical - ALIGNED_OCCUPIED_META_SIZE;
        *get_allocator_ptr(target_block) = this;
    }

    return get_block_payload_ptr(target_block);
}
    
    
void allocator_red_black_tree::do_deallocate_sm(void *at)
{
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    void* target_block = static_cast<char*>(at) - ALIGNED_OCCUPIED_META_SIZE;

    if (*get_allocator_ptr(target_block) != this)
    {
        throw std::logic_error("RB Tree: Block does not belong to this allocator");
    }

    size_t current_size = *get_block_size_ptr(target_block);
    size_t current_physical = current_size + ALIGNED_OCCUPIED_META_SIZE;

    size_t space_size = *get_space_size_ptr(_trusted_memory);
    char* end_of_memory = static_cast<char*>(_trusted_memory) + ALIGNED_ALLOC_META_SIZE + space_size;

    char* right_phys = static_cast<char*>(target_block) + current_physical;
    if (right_phys < end_of_memory)
    {
        if (!get_block_data_ptr(right_phys)->occupied)
        {
            remove_node(_trusted_memory, right_phys);
            
            size_t right_size = *get_block_size_ptr(right_phys);
            size_t right_physical = right_size + ALIGNED_FREE_META_SIZE;
            
            current_physical += right_physical;
            
            char* next_of_right = right_phys + right_physical;
            if (next_of_right < end_of_memory) *get_prev_physical_ptr(next_of_right) = target_block;
        }
    }

    void* left_phys = *get_prev_physical_ptr(target_block);
    if (left_phys != nullptr)
    {
        if (!get_block_data_ptr(left_phys)->occupied)
        {
            remove_node(_trusted_memory, left_phys);
            
            size_t left_size = *get_block_size_ptr(left_phys);
            size_t left_physical = left_size + ALIGNED_FREE_META_SIZE;
            
            current_physical += left_physical;
            
            target_block = left_phys;
            
            char* next_of_target = static_cast<char*>(target_block) + current_physical;
            if (next_of_target < end_of_memory) *get_prev_physical_ptr(next_of_target) = target_block;
        }
    }

    get_block_data_ptr(target_block)->occupied = false;
    *get_block_size_ptr(target_block) = current_physical - ALIGNED_FREE_META_SIZE;

    insert_node(_trusted_memory, target_block);
}

void allocator_red_black_tree::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    if (!_trusted_memory) return;
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));
    *get_fit_mode_ptr(_trusted_memory) = mode;
}
        
std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info() const
{
    if (!_trusted_memory) return {};

    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> info;
    for (auto it = begin(); it != end(); ++it) info.push_back({ it.size(), it.occupied() });

    return info;
}
        
        
        
        
        
        
        
        
        
        
/**
 * RB Iterator
 */

allocator_red_black_tree::rb_iterator allocator_red_black_tree::begin() const noexcept
{
    return rb_iterator(_trusted_memory);
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::end() const noexcept
{
    return rb_iterator(nullptr);
}

bool allocator_red_black_tree::rb_iterator::operator==(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return _block_ptr == other._block_ptr;
}

bool allocator_red_black_tree::rb_iterator::operator!=(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return _block_ptr != other._block_ptr;
}

allocator_red_black_tree::rb_iterator &allocator_red_black_tree::rb_iterator::operator++() & noexcept
{
    if (!_block_ptr) return *this;

    bool is_occupied = occupied();
    size_t payload = *get_block_size_ptr(_block_ptr);
    size_t meta = is_occupied ? ALIGNED_OCCUPIED_META_SIZE : ALIGNED_FREE_META_SIZE;

    char* next_block = static_cast<char*>(_block_ptr) + meta + payload;

    size_t space_size = *get_space_size_ptr(_trusted);
    char* end_of_memory = static_cast<char*>(_trusted) + space_size + ALIGNED_ALLOC_META_SIZE;

    if (next_block >= end_of_memory) _block_ptr = nullptr;
    else _block_ptr = next_block;

    return *this;
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::rb_iterator::operator++(int n)
{
    rb_iterator temp = *this;
    ++(*this);
    return temp;
}

size_t allocator_red_black_tree::rb_iterator::size() const noexcept
{
    if (!_block_ptr) return 0;

    bool is_occupied = occupied();
    size_t payload = *get_block_size_ptr(_block_ptr);
    return payload + (is_occupied ? ALIGNED_OCCUPIED_META_SIZE : ALIGNED_FREE_META_SIZE);
}

void* allocator_red_black_tree::rb_iterator::operator*() const noexcept
{
    if (!_block_ptr) return nullptr;
    return get_block_payload_ptr(_block_ptr);
}

allocator_red_black_tree::rb_iterator::rb_iterator(): _block_ptr(nullptr), _trusted(nullptr) { }

allocator_red_black_tree::rb_iterator::rb_iterator(void *trusted)
{
    if (trusted) _block_ptr = static_cast<char*>(trusted) + ALIGNED_ALLOC_META_SIZE;
    else _block_ptr = nullptr;
}

bool allocator_red_black_tree::rb_iterator::occupied() const noexcept
{
    if(!_block_ptr) return false;
    return get_block_data_ptr(_block_ptr)->occupied;
}
