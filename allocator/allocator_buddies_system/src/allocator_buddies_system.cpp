#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"



inline size_t get_aligned_size(size_t raw_size)
{
    return (raw_size + 7) & ~7;
}



/**
 * get allocator metadata
 */
std::pmr::memory_resource** allocator_buddies_system::get_parent_allocator_ptr(void* trusted_memory)
{
    return reinterpret_cast<std::pmr::memory_resource**>(trusted_memory);
}

std::mutex* allocator_buddies_system::get_mutex_ptr(void* trusted_memory)
{
    return reinterpret_cast<std::mutex*>(static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*));
}

allocator_with_fit_mode::fit_mode* allocator_buddies_system::get_fit_mode_ptr(void* trusted_memory)
{
    return reinterpret_cast<allocator_with_fit_mode::fit_mode*>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(std::mutex)
    );
}

unsigned char* allocator_buddies_system::get_total_k_ptr(void* trusted_memory)
{
    return reinterpret_cast<unsigned char*>
    (
        static_cast<char*>(trusted_memory) + sizeof(std::pmr::memory_resource*) \
            + sizeof(std::mutex) + sizeof(allocator_with_fit_mode::fit_mode)
    );
}





/**
 * get block metadata
 */

void* allocator_buddies_system::get_block_payload_ptr(void* block_header)
{
    return static_cast<char*>(block_header) + get_aligned_size(occupied_block_metadata_size);
}

void* allocator_buddies_system::get_header_from_payload(void* payload)
{
    return static_cast<char*>(payload) - get_aligned_size(occupied_block_metadata_size);
}

void** allocator_buddies_system::get_allocator_ptr(void* block_header)
{
    return reinterpret_cast<void**>(static_cast<char*>(block_header) + sizeof(void*));
}








allocator_buddies_system::allocator_buddies_system(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    size_t total_k = __detail::nearest_greater_k_of_2(space_size);
    if (total_k < min_k) throw std::invalid_argument("Size too small");

    size_t buddy_area_size = 1ULL << total_k;
    size_t aligned_meta = get_aligned_size(allocator_metadata_size);

    std::pmr::memory_resource* parent = parent_allocator ? parent_allocator : std::pmr::get_default_resource();

    try { _trusted_memory = parent->allocate(aligned_meta + buddy_area_size); }
    catch ( ... ) { throw std::bad_alloc(); }

    *get_parent_allocator_ptr(_trusted_memory) = parent;
    *get_fit_mode_ptr(_trusted_memory) = allocate_fit_mode;
    *get_total_k_ptr(_trusted_memory) = static_cast<unsigned char>(total_k);

    new (get_mutex_ptr(_trusted_memory)) std::mutex();

    void* first_block = static_cast<char*>(_trusted_memory) + aligned_meta;
    
    auto* meta = reinterpret_cast<block_metadata*>(first_block);
    meta->occupied = false;
    meta->size = static_cast<unsigned char>(total_k);
}

allocator_buddies_system::allocator_buddies_system(allocator_buddies_system &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_buddies_system::~allocator_buddies_system()
{
    if (!_trusted_memory) return;

    std::pmr::memory_resource* parent = *get_parent_allocator_ptr(_trusted_memory);
    unsigned char total_k = *get_total_k_ptr(_trusted_memory);
    size_t aligned_meta = get_aligned_size(allocator_metadata_size);
    
    std::mutex* mtx = get_mutex_ptr(_trusted_memory);
    mtx->~mutex();

    parent->deallocate(_trusted_memory, aligned_meta + (1ULL << total_k));
    _trusted_memory = nullptr;
}

allocator_buddies_system &allocator_buddies_system::operator=(allocator_buddies_system &&other) noexcept
{
    if (this == &other) return *this;

    if (_trusted_memory != nullptr) this->~allocator_buddies_system();
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    return *this;
}







[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(size_t size)
{
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    size_t required_bytes = size + get_aligned_size(occupied_block_metadata_size);
    unsigned char required_k = static_cast<unsigned char>(__detail::nearest_greater_k_of_2(required_bytes));

    if (required_k < min_k) required_k = min_k;

    auto mode = *get_fit_mode_ptr(_trusted_memory);
    size_t aligned_meta = get_aligned_size(allocator_metadata_size);
    char* start_of_blocks = static_cast<char*>(_trusted_memory) + aligned_meta;
    unsigned char total_k = *get_total_k_ptr(_trusted_memory);
    char* end_of_memory = start_of_blocks + (1ULL << total_k);

    char* current = start_of_blocks;
    void* target_block = nullptr;
    unsigned char min_k = 255;
    unsigned char max_k = 0;

    while (current < end_of_memory)
    {
        auto* meta = reinterpret_cast<block_metadata*>(current);
        unsigned char k = meta->size;

        if (!meta->occupied && k >= required_k)
        {
            if (mode == allocator_with_fit_mode::fit_mode::first_fit)
            {
                target_block = current;
                break;
            }
            else if (mode == allocator_with_fit_mode::fit_mode::the_best_fit)
            {
                if (k < min_k)
                {
                    min_k = k;
                    target_block = current;
                }
            }
            else if (mode == allocator_with_fit_mode::fit_mode::the_worst_fit)
            {
                if (k > max_k)
                {
                    max_k = k;
                    target_block = current;
                }
            }
        }
        
        current += (1ULL << k);
    }

    if (!target_block) throw std::bad_alloc();

    auto* meta = reinterpret_cast<block_metadata*>(target_block);

    while (meta->size > required_k)
    {
        meta->size--;
        unsigned char new_k = meta->size;

        char* buddy = static_cast<char*>(target_block) + (1ULL << new_k);
        auto* buddy_meta = reinterpret_cast<block_metadata*>(buddy);
        
        buddy_meta->occupied = false;
        buddy_meta->size = new_k;
    }

    meta->occupied = true;
    *get_allocator_ptr(target_block) = this;

    return get_block_payload_ptr(target_block);
}

void allocator_buddies_system::do_deallocate_sm(void *at)
{
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));

    void* block = get_header_from_payload(at);
    if (*get_allocator_ptr(block) != this)
    {
        throw std::logic_error("Block does not belong to this allocator!");
    }

    auto* meta = reinterpret_cast<block_metadata*>(block);
    meta->occupied = false;

    size_t aligned_meta = get_aligned_size(allocator_metadata_size);
    char* start_of_blocks = static_cast<char*>(_trusted_memory) + aligned_meta;
    unsigned char total_k = *get_total_k_ptr(_trusted_memory);

    char* current_ptr = static_cast<char*>(block);
    unsigned char current_k = meta->size;

    while (current_k < total_k)
    {
        size_t offset = current_ptr - start_of_blocks;
        
        size_t buddy_offset = offset ^ (1ULL << current_k);
        char* buddy_ptr = start_of_blocks + buddy_offset;

        auto* buddy_meta = reinterpret_cast<block_metadata*>(buddy_ptr);
        if (!buddy_meta->occupied && buddy_meta->size == current_k)
        {
            current_k++;
            
            current_ptr = (offset < buddy_offset) ? current_ptr : buddy_ptr;
            
            auto* merged_meta = reinterpret_cast<block_metadata*>(current_ptr);
            merged_meta->size = current_k;
            merged_meta->occupied = false; 
        }
        else break; 
    }
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

inline void allocator_buddies_system::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    if (!_trusted_memory) return;
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));
    *get_fit_mode_ptr(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    if (!_trusted_memory) return {};
    std::lock_guard<std::mutex> lock(*get_mutex_ptr(_trusted_memory));
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> info;
    
    size_t total_k = *get_total_k_ptr(_trusted_memory);
    size_t total_size = 1ULL << total_k;
    size_t traversed = 0;

    auto it = begin();
    while (traversed < total_size)
    {
        info.push_back({ it.size(), it.occupied() });
        traversed += it.size();
        ++it;
    }

    return info;
}








/**
 * Buddy iterator implementation
 */

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept
{
    return buddy_iterator(_trusted_memory);
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept
{
    return buddy_iterator(nullptr);
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
    if (!_block) return *this;

    auto* meta = reinterpret_cast<block_metadata*>(_block);
    unsigned char k = meta->size;

    char* next_block = static_cast<char*>(_block) + (1ULL << k);
    _block = next_block;
    
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
    if (!_block) return 0;
    auto* meta = reinterpret_cast<block_metadata*>(_block);
    return 1ULL << (meta->size); 
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept
{
    if (!_block) return false;
    return reinterpret_cast<block_metadata*>(_block)->occupied;
}

void *allocator_buddies_system::buddy_iterator::operator*() const noexcept
{
    if (!_block) return nullptr;
    return allocator_buddies_system::get_block_payload_ptr(_block);
}

allocator_buddies_system::buddy_iterator::buddy_iterator(void *start)
{
    if (start)
    {
        size_t aligned_meta = get_aligned_size(allocator_metadata_size);
        _block = static_cast<char*>(start) + aligned_meta;
    }
    else
    {
        _block = nullptr;
    }
}

allocator_buddies_system::buddy_iterator::buddy_iterator(): _block(nullptr) { }
