#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"


inline size_t get_aligned_size(size_t raw_size)
{
    return (raw_size + 7) & ~7;
}



/**
 * get allocator metadata
 */
static std::pmr::memory_resource** get_parent_allocator_ptr(void* trusted_memory)
{
    return reinterpret_cast<std::pmr::memory_resource**>(trusted_memory);
}

static size_t* get_space_size_ptr(void* trusted_memory)
{
    return reinterpret_cast<size_t*>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*)
    );
}

static void** get_first_free_block(void* trusted_memory)
{
    return reinterpret_cast<void**>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(size_t)
    );
}

static std::mutex* get_mutex_ptr(void* trusted_memory)
{
    return reinterpret_cast<std::mutex*>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(size_t) + sizeof(void*)
    );
}

static allocator_with_fit_mode::fit_mode* get_fit_mode_ptr(void* trusted_memory)
{
    return reinterpret_cast<allocator_with_fit_mode::fit_mode*>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*) \
            + sizeof(size_t) + sizeof(void*) + sizeof(std::mutex)
    );
}





/**
 * get block metadata
 */
static size_t* get_block_size_ptr(void* header)
{
    return reinterpret_cast<size_t*>(header);
}

static void** get_allocator_ptr(void* header)
{
    return reinterpret_cast<void**>(static_cast<char*>(header) + sizeof(size_t));
}

static void** get_next_free_ptr(void* header)
{
    return reinterpret_cast<void**>(static_cast<char*>(header) + sizeof(size_t) + sizeof(void*));
}

static void* get_block_payload_ptr(void* header)
{
    return static_cast<char*>(header) + sizeof(size_t) + sizeof(void*) + sizeof(void*); 
}

static void* get_header_from_payload(void* payload)
{
    return static_cast<char*>(payload) - (sizeof(size_t) + sizeof(void*) + sizeof(void*)); 
}

static void** get_footer_ptr(void* header)
{
    size_t size = *get_block_size_ptr(header);
    return reinterpret_cast<void**>(static_cast<char*>(header) + 24 + size);
}








allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    size_t aligned_meta = get_aligned_size(allocator_metadata_size);

    if (space_size < aligned_meta + occupied_block_metadata_size + 1)
        throw std::invalid_argument("Size too small");

    std::pmr::memory_resource* parent = parent_allocator ? parent_allocator : std::pmr::get_default_resource();

    try { _trusted_memory = parent->allocate(space_size + aligned_meta); }
    catch (...) { throw std::bad_alloc(); }

    *get_parent_allocator_ptr(_trusted_memory) = parent;
    *get_fit_mode_ptr(_trusted_memory) = allocate_fit_mode;
    *get_space_size_ptr(_trusted_memory) = space_size;

    new (get_mutex_ptr(_trusted_memory)) std::mutex();

    void* first_block_header = static_cast<char*>(_trusted_memory) + aligned_meta;
    *get_first_free_block(_trusted_memory) = first_block_header;

    size_t usable_block_size = space_size - occupied_block_metadata_size;
    
    *get_block_size_ptr(first_block_header) = usable_block_size;
    *get_allocator_ptr(first_block_header) = nullptr; 
    *get_next_free_ptr(first_block_header) = nullptr;

    *get_footer_ptr(first_block_header) = first_block_header;
}

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (!_trusted_memory) return;

    std::pmr::memory_resource* parent = *get_parent_allocator_ptr(_trusted_memory);
    size_t space_size = *get_space_size_ptr(_trusted_memory);
    size_t aligned_meta = get_aligned_size(allocator_metadata_size);

    std::mutex* mtx = get_mutex_ptr(_trusted_memory);
    mtx->~mutex();

    parent->deallocate(_trusted_memory, space_size + aligned_meta);
    _trusted_memory = nullptr;
}

allocator_boundary_tags &allocator_boundary_tags::operator=(allocator_boundary_tags &&other) noexcept
{
    if (this == &other) return *this;

    if (_trusted_memory != nullptr) this->~allocator_boundary_tags();

    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    return *this;
}





/**
 * Search strategies
 */
allocator_boundary_tags::search_result allocator_boundary_tags::find_first_fit(size_t size) const
{
    void* prev = nullptr;
    void* current = *get_first_free_block(_trusted_memory);

    while (current)
    {
        if (*get_block_size_ptr(current) >= size) return {prev, current};
        prev = current;
        current = *get_next_free_ptr(current);
    }

    return {nullptr, nullptr};
}

allocator_boundary_tags::search_result allocator_boundary_tags::find_best_fit(size_t size) const
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
        current = *get_next_free_ptr(current);
    }

    return {best_prev, best_block};
}

allocator_boundary_tags::search_result allocator_boundary_tags::find_worst_fit(size_t size) const
{
    void* prev = nullptr;
    void* current = *get_first_free_block(_trusted_memory);

    void* worst_prev = nullptr;
    void* worst_block = nullptr;
    size_t max_diff = 0;

    while (current)
    {
        size_t block_size = *get_block_size_ptr(current);

        if (block_size >= size)
        {
            size_t diff = block_size - size;

            if (diff > max_diff)
            {
                max_diff = diff;
                worst_prev = prev;
                worst_block = current;
            }
        }

        prev = current;
        current = *get_next_free_ptr(current);
    }

    return {worst_prev, worst_block};
}






[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(size_t size)
{
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    auto mode = *get_fit_mode_ptr(_trusted_memory);
    search_result res = {nullptr, nullptr};

    switch (mode)
    {
        case allocator_with_fit_mode::fit_mode::first_fit:
            res = find_first_fit(size); break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            res = find_best_fit(size); break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            res = find_worst_fit(size); break;
    }

    if (!res.target_block) throw std::bad_alloc();

    size_t found_size = *get_block_size_ptr(res.target_block);
    void* next_free = *get_next_free_ptr(res.target_block);

    if (found_size >= size + occupied_block_metadata_size)
    {
        size_t new_free_size = found_size - size - occupied_block_metadata_size;
        
        void* new_free_block = static_cast<char*>(res.target_block) + occupied_block_metadata_size + size;

        *get_block_size_ptr(new_free_block) = new_free_size;
        *get_allocator_ptr(new_free_block) = nullptr; 
        *get_next_free_ptr(new_free_block) = next_free;
        *get_footer_ptr(new_free_block) = new_free_block;

        *get_block_size_ptr(res.target_block) = size;
        *get_allocator_ptr(res.target_block) = this;  
        *get_footer_ptr(res.target_block) = res.target_block;

        if (res.prev_block) *get_next_free_ptr(res.prev_block) = new_free_block;
        else *get_first_free_block(_trusted_memory) = new_free_block;
    }
    else
    {
        *get_allocator_ptr(res.target_block) = this; 
        
        if (res.prev_block) *get_next_free_ptr(res.prev_block) = next_free;
        else *get_first_free_block(_trusted_memory) = next_free;
    }

    return get_block_payload_ptr(res.target_block);
}

void allocator_boundary_tags::do_deallocate_sm(void *at)
{
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    void* current_header = get_header_from_payload(at);
    if (*get_allocator_ptr(current_header) != this)
        throw std::logic_error("Boundary Tags: Block does not belong to this allocator!");

    size_t current_size = *get_block_size_ptr(current_header);

    auto remove_from_free_list = [&](void* block_to_remove)
    {
        void** current_ptr = get_first_free_block(_trusted_memory);
        while (*current_ptr)
        {
            if (*current_ptr == block_to_remove)
            {
                *current_ptr = *get_next_free_ptr(block_to_remove);
                break;
            }

            current_ptr = get_next_free_ptr(*current_ptr);
        }
    };

    size_t total_space = *get_space_size_ptr(_trusted_memory);
    size_t aligned_meta = get_aligned_size(allocator_metadata_size);
    
    char* start_of_blocks = static_cast<char*>(_trusted_memory) + aligned_meta;
    char* end_of_memory = static_cast<char*>(_trusted_memory) + total_space;

    char* right_header_char = static_cast<char*>(current_header) + occupied_block_metadata_size + current_size;
    if (right_header_char < end_of_memory) {
        void* right_header = right_header_char;
        
        if (*get_allocator_ptr(right_header) == nullptr) {
            remove_from_free_list(right_header);
            
            current_size += occupied_block_metadata_size + *get_block_size_ptr(right_header);
            *get_block_size_ptr(current_header) = current_size;
            
            *get_footer_ptr(current_header) = current_header;
        }
    }

    char* left_footer_char = static_cast<char*>(current_header) - sizeof(void*);
    if (left_footer_char >= start_of_blocks) {
        void* left_header = *reinterpret_cast<void**>(left_footer_char);
        
        if (*get_allocator_ptr(left_header) == nullptr) {
            remove_from_free_list(left_header);
            
            size_t left_size = *get_block_size_ptr(left_header);
            left_size += occupied_block_metadata_size + current_size;
            
            current_header = left_header;
            current_size = left_size;
            
            *get_block_size_ptr(current_header) = current_size;
            *get_footer_ptr(current_header) = current_header;
        }
    }

    *get_allocator_ptr(current_header) = nullptr;
    
    void* old_head = *get_first_free_block(_trusted_memory);
    *get_next_free_ptr(current_header) = old_head;
    *get_first_free_block(_trusted_memory) = current_header;
}

inline void allocator_boundary_tags::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    if (!_trusted_memory) return;
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));
    *get_fit_mode_ptr(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    if (!_trusted_memory) return {};
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    return get_blocks_info_inner();
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> info;
    for (auto it = begin(); it != end(); ++it) 
    {
        info.push_back({ it.size(), it.occupied() });
    }

    return info;
}







/**
 * boundary iterator
 */

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    if (!_occupied_ptr) return 0;
    
    return *get_block_size_ptr(_occupied_ptr) + occupied_block_metadata_size;
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    return boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    return boundary_iterator(nullptr);
}

bool allocator_boundary_tags::boundary_iterator::operator==(const boundary_iterator& other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(const boundary_iterator& other) const noexcept
{
    return _occupied_ptr != other._occupied_ptr;
}

allocator_boundary_tags::boundary_iterator& allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (!_occupied_ptr) return *this;

    size_t payload_size = *get_block_size_ptr(_occupied_ptr);
    char* next_header = static_cast<char*>(_occupied_ptr) + occupied_block_metadata_size + payload_size;

    size_t space_size = *get_space_size_ptr(_trusted_memory);
    size_t aligned_meta = get_aligned_size(allocator_metadata_size);
    char* end_of_memory = static_cast<char*>(_trusted_memory) + aligned_meta + space_size;

    if (next_header >= end_of_memory)
    {
        _occupied_ptr = nullptr;
    }
    else
    {
        _occupied_ptr = next_header;
        _occupied = (*get_allocator_ptr(_occupied_ptr) != nullptr);
    }

    return *this;
}

allocator_boundary_tags::boundary_iterator& allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    if (!_occupied_ptr) return *this;

    size_t aligned_meta = get_aligned_size(allocator_metadata_size);
    char* start_of_blocks = static_cast<char*>(_trusted_memory) + aligned_meta;

    if (static_cast<char*>(_occupied_ptr) <= start_of_blocks)
    {
        _occupied_ptr = nullptr; 
    }
    else
    {
        void* prev_footer = *reinterpret_cast<void**>(static_cast<char*>(_occupied_ptr) - sizeof(void*));
        
        _occupied_ptr = prev_footer;
        _occupied = (*get_allocator_ptr(_occupied_ptr) != nullptr);
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

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    if (_occupied_ptr) return get_block_payload_ptr(_occupied_ptr);

    return nullptr;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr) {}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void* trusted): _trusted_memory(trusted)
{
    if (trusted)
    {
        size_t aligned_meta = get_aligned_size(allocator_metadata_size);
        _occupied_ptr = static_cast<char*>(trusted) + aligned_meta;
        _occupied = (*get_allocator_ptr(_occupied_ptr) != nullptr);
    }
    else 
    {
        _occupied_ptr = nullptr;
        _occupied = false;
    }
}

void* allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}
