#include <not_implemented.h>
#include "../include/allocator_sorted_list.h"
#include <new>     
#include <limits>  
#include <stdexcept>


/**
 * get metadata from void* trusted_memory
 */

static std::pmr::memory_resource** get_parent_allocator_ptr(void* trusted_memory)
{
    return reinterpret_cast<std::pmr::memory_resource**>(trusted_memory);
}

static allocator_with_fit_mode::fit_mode* get_fit_mode_ptr(void* trusted_memory)
{
    return reinterpret_cast<allocator_with_fit_mode::fit_mode*>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(size_t) + sizeof(void*) + sizeof(std::mutex)
    );
}

static size_t* get_space_size_ptr(void* trusted_memory)
{
    return reinterpret_cast<size_t*>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*)
    );
}

static std::mutex* get_mutex_ptr(void* trusted_memory)
{
    return reinterpret_cast<std::mutex*>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(size_t) + sizeof(void*)
    );
}

static void** get_first_free_block(void* trusted_memory)
{
    return reinterpret_cast<void**>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(size_t)
    );
}




/**
 * get metadata from block
 */

static size_t* get_block_size_ptr(void* block_header)
{
    return reinterpret_cast<size_t*>(block_header);
}

static void** get_next_free_block_ptr(void* block_header)
{
    return reinterpret_cast<void**>
    (
        static_cast<char*>(block_header) + sizeof(size_t)
    );
}

static void* get_block_payload_ptr(void* block_header)
{
    return reinterpret_cast<char*>(block_header) + sizeof(size_t) + sizeof(void*);
}

static void* get_block_header_from_payload(void* payload)
{
    return reinterpret_cast<char*>(payload) - (sizeof(size_t) + sizeof(void*));
}




allocator_sorted_list::allocator_sorted_list(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < allocator_metadata_size + block_metadata_size + 1)
        throw std::invalid_argument("Size too small");

    std::pmr::memory_resource* parent = parent_allocator
        ? parent_allocator
        : std::pmr::get_default_resource();

    try { _trusted_memory = parent->allocate(space_size); }
    catch(...) { throw std::bad_alloc(); } 

    *get_parent_allocator_ptr(_trusted_memory) = parent;
    *get_fit_mode_ptr(_trusted_memory) = allocate_fit_mode;
    *get_space_size_ptr(_trusted_memory) = space_size;

    new (get_mutex_ptr(_trusted_memory)) std::mutex();

    void* first_block = static_cast<char*>(_trusted_memory) + allocator_metadata_size;
    *get_first_free_block(_trusted_memory) = first_block;

    size_t usable_block_size = space_size - allocator_metadata_size - block_metadata_size;
    *get_block_size_ptr(first_block) = usable_block_size;
    *get_next_free_block_ptr(first_block) = nullptr;
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
    _trusted_memory  = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_sorted_list::~allocator_sorted_list()
{
    if (!_trusted_memory) return;

    std::pmr::memory_resource* parent = *get_parent_allocator_ptr(_trusted_memory);
    size_t space_size = *get_space_size_ptr(_trusted_memory);

    std::mutex* mtx = get_mutex_ptr(_trusted_memory);
    mtx->~mutex();

    parent->deallocate(_trusted_memory, space_size);
    _trusted_memory = nullptr;
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    if (this == &other) return *this;

    if (_trusted_memory != nullptr) this->~allocator_sorted_list();

    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    return *this;
}




/**
 * Search strategies
 */

allocator_sorted_list::search_result allocator_sorted_list::find_first_fit(size_t size) const
{
    void* prev = nullptr;
    void* current = *get_first_free_block(_trusted_memory);

    while (current)
    {
        if (*get_block_size_ptr(current) >= size) return {prev, current};

        prev = current;
        current = *get_next_free_block_ptr(current);
    }

    return {nullptr, nullptr};
}

allocator_sorted_list::search_result allocator_sorted_list::find_best_fit(size_t size) const
{
    void* prev = nullptr;
    void* current = *get_first_free_block(_trusted_memory);

    void* best_prev = nullptr;
    void* best_block = nullptr;
    size_t min_diff = std::numeric_limits<size_t>::max();

    while (current)
    {
        size_t block_size = *get_block_size_ptr(current);
        if (block_size >= size)
        {
            size_t diff = block_size - size;
            if (diff < min_diff)
            {
                min_diff = diff;
                best_prev = prev;
                best_block = current;
            }
        }

        prev = current;
        current = *get_next_free_block_ptr(current);
    }

    return {best_prev, best_block};
} 

allocator_sorted_list::search_result allocator_sorted_list::find_worst_fit(size_t size) const
{
    void* prev = nullptr;
    void* current = *get_first_free_block(_trusted_memory);

    void* worst_block = nullptr;
    void* worst_prev = nullptr;
    size_t max_diff = std::numeric_limits<size_t>::min();

    while (current)
    {
        size_t block_size = *get_block_size_ptr(current);
        if (block_size >= size)
        {
            size_t diff = block_size - size;
            if (diff > max_diff)
            {
                max_diff = diff;
                worst_block = current;
                worst_prev = prev;
            }
        }

        prev = current;
        current = *get_next_free_block_ptr(current);
    }

    return {worst_prev, worst_block};
}






[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(size_t size)
{
    std::lock_guard <std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    auto mode = *get_fit_mode_ptr(_trusted_memory);
    search_result res = {nullptr, nullptr};

    switch (mode)
    {
        case allocator_sorted_list::fit_mode::first_fit:
            res = find_first_fit(size); break;
        case allocator_sorted_list::fit_mode::the_best_fit:
            res = find_best_fit(size); break;
        case allocator_sorted_list::fit_mode::the_worst_fit:
            res = find_worst_fit(size); break;
    }

    if (!res.target_block) throw std::bad_alloc();

    size_t found_size = *get_block_size_ptr(res.target_block);
    void* next_free = *get_next_free_block_ptr(res.target_block);

    if (found_size >= size + block_metadata_size + 1)
    {
        void* new_free_block = static_cast<char*>(res.target_block) + block_metadata_size + size;
        
        *get_block_size_ptr(new_free_block) = found_size - size - block_metadata_size;
        *get_next_free_block_ptr(new_free_block) = next_free;
        *get_block_size_ptr(res.target_block) = size;

        if (res.prev_block) *get_next_free_block_ptr(res.prev_block) = new_free_block;
        else *get_first_free_block(_trusted_memory) = new_free_block;
    }
    else
    {
        if (res.prev_block) *get_next_free_block_ptr(res.prev_block) = next_free;
        else *get_first_free_block(_trusted_memory) = next_free;
    }

    return get_block_payload_ptr(res.target_block);
}

bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

void allocator_sorted_list::do_deallocate_sm(void* at)
{
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    void* target_block = get_block_header_from_payload(at);
    size_t target_size = *get_block_size_ptr(target_block);

    void** head_ptr = get_first_free_block(_trusted_memory);
    void* current = *head_ptr;
    void* prev = nullptr;

    while (current && current < target_block)
    {
        prev = current;
        current = *get_next_free_block_ptr(current);
    }

    if (!prev) *head_ptr = target_block;
    else *get_next_free_block_ptr(prev) = target_block;
    *get_next_free_block_ptr(target_block) = current;

    if (current)
    {
        char* target_end = static_cast<char*>(target_block) + block_metadata_size + target_size;
        
        if (target_end == static_cast<char*>(current))
        {
            *get_block_size_ptr(target_block) += block_metadata_size + *get_block_size_ptr(current);
            *get_next_free_block_ptr(target_block) = *get_next_free_block_ptr(current);
        }
    }

    if (prev)
    {
        char* prev_end = static_cast<char*>(prev) + block_metadata_size + *get_block_size_ptr(prev);
        
        if (prev_end == static_cast<char*>(target_block))
        {
            *get_block_size_ptr(prev) += block_metadata_size + *get_block_size_ptr(target_block);
            *get_next_free_block_ptr(prev) = *get_next_free_block_ptr(target_block);
        }
    }
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    if (!_trusted_memory) return;

    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));
    *get_fit_mode_ptr(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    if (!_trusted_memory) return {};
    
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> info;
    for (auto it = begin(); it != end(); ++it) info.push_back({ it.size(), it.occupied() });
    
    return info;
}







/**
 * sorted_free_iterarot methods
 */

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept
{
    return sorted_free_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept
{
    return sorted_free_iterator(nullptr);
}

bool allocator_sorted_list::sorted_free_iterator::operator==(
    const allocator_sorted_list::sorted_free_iterator & other) const noexcept
{
    return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
    const allocator_sorted_list::sorted_free_iterator &other) const noexcept
{
    return _free_ptr != other._free_ptr;
}

allocator_sorted_list::sorted_free_iterator &allocator_sorted_list::sorted_free_iterator::operator++() & noexcept
{
    if (_free_ptr) _free_ptr = *get_next_free_block_ptr(_free_ptr);
    return *this;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int n)
{
    sorted_free_iterator temp = *this;
    ++(*this);
    return temp;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept
{
    if (_free_ptr) return *get_block_size_ptr(_free_ptr);
    return 0;
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept
{
    if (_free_ptr) return get_block_payload_ptr(_free_ptr);
    return nullptr;
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(): _free_ptr(nullptr) { }

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void *trusted)
{
    if (trusted) _free_ptr = *get_first_free_block(trusted);
    else _free_ptr = nullptr;
}






/**
 * sorted_iterator methods
 */

allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept
{
    return sorted_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept
{
    return sorted_iterator(nullptr);
}

bool allocator_sorted_list::sorted_iterator::operator==(const allocator_sorted_list::sorted_iterator & other) const noexcept
{
    return _current_ptr == other._current_ptr;
}

bool allocator_sorted_list::sorted_iterator::operator!=(const allocator_sorted_list::sorted_iterator &other) const noexcept
{
    return _current_ptr != other._current_ptr;
}

allocator_sorted_list::sorted_iterator &allocator_sorted_list::sorted_iterator::operator++() & noexcept
{
    if (!_current_ptr) return *this;

    if (_current_ptr == _free_ptr) _free_ptr = *get_next_free_block_ptr(_free_ptr);

    size_t current_size = *get_block_size_ptr(_current_ptr);
    char* next_block_ptr = static_cast<char*>(_current_ptr) + block_metadata_size + current_size;

    size_t total_space = *get_space_size_ptr(_trusted_memory);
    char* end_of_memory = static_cast<char*>(_trusted_memory) + total_space;

    if (next_block_ptr >= end_of_memory)
    {
        _current_ptr = nullptr;
        _free_ptr = nullptr;
    }
    else
    {
        _current_ptr = next_block_ptr;
    }

    return *this;
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int n)
{
    sorted_iterator temp = *this;
    ++(*this);
    return temp;
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept
{
    if (_current_ptr) return *get_block_size_ptr(_current_ptr);
    return 0;
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept
{
    if (_current_ptr) return get_block_payload_ptr(_current_ptr);
    return nullptr;
}

allocator_sorted_list::sorted_iterator::sorted_iterator(): _free_ptr(nullptr), _current_ptr(nullptr), _trusted_memory(nullptr) { }

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted): _trusted_memory(trusted)
{
    if (trusted)
    {
        _current_ptr = static_cast<char*>(trusted) + allocator_metadata_size;
        _free_ptr = *get_first_free_block(trusted);
    }
    else
    {
        _current_ptr = nullptr;
        _free_ptr = nullptr;
    }
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept
{
    return _current_ptr != _free_ptr;
}
