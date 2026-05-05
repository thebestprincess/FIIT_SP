#ifndef SYS_PROG_B_TREE_H
#define SYS_PROG_B_TREE_H

#include <iterator>
#include <utility>
#include <boost/container/static_vector.hpp>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class B_tree final : private compare // EBCO
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration


    struct btree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<btree_node*, maximum_keys_in_node + 2> _pointers;
        btree_node() noexcept;
    };

    pp_allocator<value_type> _allocator;
    btree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit B_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit B_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit B_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    B_tree(const B_tree& other);

    B_tree(B_tree&& other) noexcept;

    B_tree& operator=(const B_tree& other);

    B_tree& operator=(B_tree&& other) noexcept;

    ~B_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class btree_iterator;
    class btree_reverse_iterator;
    class btree_const_iterator;
    class btree_const_reverse_iterator;

    class btree_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);

    };

    class btree_const_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_iterator;
        friend class btree_const_reverse_iterator;

        btree_const_iterator(const btree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_const_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    class btree_reverse_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_reverse_iterator;

        friend class B_tree;
        friend class btree_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

        btree_reverse_iterator(const btree_iterator& it) noexcept;
        operator btree_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_reverse_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);
    };

    class btree_const_reverse_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_reverse_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_iterator;

        btree_const_reverse_iterator(const btree_reverse_iterator& it) noexcept;
        operator btree_const_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_const_reverse_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;
    friend class btree_reverse_iterator;
    friend class btree_const_reverse_iterator;

    // endregion iterators declaration

    // region element access declaration

    /*
     * Returns a reference to the mapped value of the element with specified key. If no such element exists, an exception of type std::out_of_range is thrown.
     */
    tvalue& at(const tkey&);
    const tvalue& at(const tkey&) const;

    /*
     * If key not exists, makes default initialization of value
     */
    tvalue& operator[](const tkey& key);
    tvalue& operator[](tkey&& key);

    // endregion element access declaration
    // region iterator begins declaration

    btree_iterator begin();
    btree_iterator end();

    btree_const_iterator begin() const;
    btree_const_iterator end() const;

    btree_const_iterator cbegin() const;
    btree_const_iterator cend() const;

    btree_reverse_iterator rbegin();
    btree_reverse_iterator rend();

    btree_const_reverse_iterator rbegin() const;
    btree_const_reverse_iterator rend() const;

    btree_const_reverse_iterator crbegin() const;
    btree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    btree_iterator find(const tkey& key);
    btree_const_iterator find(const tkey& key) const;

    btree_iterator lower_bound(const tkey& key);
    btree_const_iterator lower_bound(const tkey& key) const;

    btree_iterator upper_bound(const tkey& key);
    btree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<btree_iterator, bool> insert(const tree_data_type& data);
    std::pair<btree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<btree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    btree_iterator insert_or_assign(const tree_data_type& data);
    btree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    btree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    btree_iterator erase(btree_iterator pos);
    btree_iterator erase(btree_const_iterator pos);

    btree_iterator erase(btree_iterator beg, btree_iterator en);
    btree_iterator erase(btree_const_iterator beg, btree_const_iterator en);


    btree_iterator erase(const tkey& key);

    // endregion modifiers declaration


public: 
    
    // region nested exceptions

    class btree_exception: public std::exception
    {
    protected:
        std::string _msg;
    public:
        explicit btree_exception(std::string msg): _msg(std::move(msg)) { }
        const char* what() const noexcept override { return _msg.c_str(); }
    };

    class key_not_found_exception final : public btree_exception
    {
    public:
        explicit key_not_found_exception(): btree_exception("B-Tree: key not found") { }
    };

    class memory_allocation_exception: public btree_exception
    {
    public:
        explicit memory_allocation_exception(): btree_exception("B-Tree: failed to allocate memory") { } 
    };

    class duplicate_key_exception: public btree_exception
    {
    public:
        explicit duplicate_key_exception(): btree_exception("B-Tree: key already exists") { }
    };

    // endregion nested exceptions
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
B_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_pairs(const B_tree::tree_data_type &lhs,
                                                     const B_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_node::btree_node() noexcept: _keys(), _pointers() {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename B_tree<tkey, tvalue, compare, t>::value_type> B_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept
{
    return _allocator;
}

// region constructors implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        const compare& cmp,
        pp_allocator<value_type> alloc)
    : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        pp_allocator<value_type> alloc,\
        const compare& cmp)
        : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
B_tree<tkey, tvalue, compare, t>::B_tree(
        iterator begin,
        iterator end,
        const compare& cmp,
        pp_allocator<value_type> alloc)
        : B_tree(cmp, alloc)
{
    for (auto it = begin; it != end; ++it)
    {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        std::initializer_list<std::pair<tkey, tvalue>> data,
        const compare& cmp,
        pp_allocator<value_type> alloc)
    : B_tree(cmp, alloc)
{
    for (const auto& item : data) {
        insert(item);
    }
}

// endregion constructors implementation

// region five implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::~B_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(const B_tree& other)
    : compare(other), 
      _allocator(other._allocator.select_on_container_copy_construction()), 
      _root(nullptr), 
      _size(0)
{
    if (!other._root) return;

    std::vector<btree_node*> allocated_nodes;
    allocated_nodes.reserve(other._size);

    auto copy_node = [&](auto& self, btree_node* other_node) -> btree_node*
    {
        if (!other_node) return nullptr;

        btree_node* new_node;
        try { new_node = _allocator.template new_object<btree_node>(); }
        catch (...) { throw memory_allocation_exception(); }
        allocated_nodes.push_back(new_node); 

        new_node->_keys = other_node->_keys;

        for (btree_node* child : other_node->_pointers)
        {
            new_node->_pointers.push_back(self(self, child));
        }

        return new_node;
    };

    try
    {
        _root = copy_node(copy_node, other._root);
        _size = other._size;
    }
    catch (...)
    {
        for (auto it = allocated_nodes.rbegin(); it != allocated_nodes.rend(); ++it)
        {
            _allocator.template delete_object<btree_node>(*it);
        }
        _root = nullptr;
        _size = 0;
        throw;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(const B_tree& other)
{
    if (this == &other) return *this;

    std::vector<btree_node*> allocated_nodes;
    allocated_nodes.reserve(other._size);

    auto copy_node = [&](auto& self, btree_node* other_node) -> btree_node*
    {
        if (!other_node) return nullptr;

        btree_node* new_node;
        try { new_node = _allocator.template new_object<btree_node>(); }
        catch (...) { throw memory_allocation_exception(); }
        allocated_nodes.push_back(new_node);

        new_node->_keys = other_node->_keys;
        for (btree_node* child : other_node->_pointers)
        {
            new_node->_pointers.push_back(self(self, child));
        }

        return new_node;
    };

    btree_node* new_root = nullptr;
    try
    {
        if (other._root)
        {
            new_root = copy_node(copy_node, other._root);
        }
    }
    catch (...)
    {
        for (auto it = allocated_nodes.rbegin(); it != allocated_nodes.rend(); ++it)
        {
            _allocator.template delete_object<btree_node>(*it);
        }
        throw;
    }

    clear(); 

    static_cast<compare&>(*this) = static_cast<const compare&>(other);
    _root = new_root;
    _size = other._size;

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(B_tree&& other) noexcept
    : compare(std::move(static_cast<compare&>(other))), 
      _allocator(std::move(other._allocator)), 
      _root(other._root), 
      _size(other._size) 
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(B_tree&& other) noexcept
{
    if (this == &other) return *this;

    clear();

    static_cast<compare&>(*this) = std::move(static_cast<compare&>(other));

    if constexpr (pp_allocator<value_type>::propagate_on_container_move_assignment::value)
    {
        _allocator = std::move(other._allocator);
    }

    _root = other._root;
    _size = other._size;

    other._root = nullptr;
    other._size = 0;

    return *this;
}
// endregion five implementation

// region iterators implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_iterator::btree_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)
        : _path(path), _index(index) 
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++()
{
    if (_path.empty()) return *this;

    btree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index + 1;
        btree_node** child_ptr = &current->_pointers[_index + 1];
        
        while (!(*child_ptr)->_pointers.empty())
        {
            _path.push({child_ptr, 0});
            child_ptr = &(*child_ptr)->_pointers[0];
        }

        _path.push({child_ptr, 0});
        _index = 0; 
        return *this;        
    }

    _index++;
    if (_index < current->_keys.size())
    {
        _path.top().second = _index; 
        return *this;
    }

    while (!_path.empty())
    {
        _path.pop(); 
        
        if (_path.empty())
        {
            _index = 0;
            return *this;
        }

        btree_node* parent = *_path.top().first;
        size_t child_idx = _path.top().second; 

        if (child_idx < parent->_keys.size())
        {
            _index = child_idx;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--()
{
    if (_path.empty()) return *this;

    btree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index;
        btree_node** child_ptr = &current->_pointers[_index];

        while (!(*child_ptr)->_pointers.empty())
        {
            size_t last_child_idx = (*child_ptr)->_pointers.size() - 1;
            _path.push({ child_ptr, last_child_idx });
            child_ptr = &(*child_ptr)->_pointers[last_child_idx];
        }

        size_t last_key_idx = (*child_ptr)->_keys.size() - 1;
        _path.push({ child_ptr, last_key_idx });
        _index = last_key_idx;
        return *this;
    }

    if (_index > 0)
    {
        --_index;
        _path.top().second = _index;
        return *this;
    }

    while(!_path.empty())
    {
        _path.pop();

        if (_path.empty()) return *this;

        size_t child_idx = _path.top().second;

        if (child_idx > 0)
        {
            _index = child_idx - 1;
            _path.top().second = _index;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator==(const self& other) const noexcept
{
    if (_path.empty() && other._path.empty()) return true;
    if (_path.empty() != other._path.empty()) return false;

    return _path.top().first == other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) return 0;
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) return false;
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index)
    : _path(path), _index(index)
{   
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(const btree_iterator& it) noexcept
    : _path(it._path), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++()
{
    if (_path.empty()) return *this; 

    btree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index + 1; 
        btree_node* const* child_ptr = &current->_pointers[_index + 1];
        
        while (!(*child_ptr)->_pointers.empty())
        {
            _path.push({child_ptr, 0});
            child_ptr = &(*child_ptr)->_pointers[0];
        }
        _path.push({child_ptr, 0});
        _index = 0; 
        return *this;
    }

    _index++;
    if (_index < current->_keys.size())
    {
        _path.top().second = _index; 
        return *this;
    }

    while (!_path.empty())
    {
        _path.pop(); 
        
        if (_path.empty())
        {
            _index = 0;
            return *this;
        }

        btree_node* parent = *_path.top().first;
        size_t child_idx = _path.top().second; 

        if (child_idx < parent->_keys.size())
        {
            _index = child_idx;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++(int)
{
    self tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--()
{
    if (_path.empty()) return *this; 

    btree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index; 
        btree_node** child_ptr = &current->_pointers[_index];
        
        while (!(*child_ptr)->_pointers.empty())
        {
            size_t last_child_idx = (*child_ptr)->_pointers.size() - 1;
            _path.push({child_ptr, last_child_idx});
            child_ptr = &(*child_ptr)->_pointers[last_child_idx];
        }
        size_t last_key_idx = (*child_ptr)->_keys.size() - 1;
        _path.push({child_ptr, last_key_idx});
        _index = last_key_idx;
        return *this;
    }

    if (_index > 0)
    {
        _index--;
        _path.top().second = _index;
        return *this;
    }

    while (!_path.empty())
    {
        _path.pop(); 
        
        if (_path.empty()) return *this; 

        size_t child_idx = _path.top().second;
        
        if (child_idx > 0)
        {
            _index = child_idx - 1;
            _path.top().second = _index;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int)
{
    self tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator==(const self& other) const noexcept
{
    if (_path.empty() && other._path.empty()) return true;
    if (_path.empty() != other._path.empty()) return false;

    return _path.top().first == other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) return 0;
    return (*_path.top().first)->_keys.size(); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) return false;
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)
    : _path(path), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(const btree_iterator& it) noexcept
    : _path(it._path), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_iterator() const noexcept
{
    return btree_reverse_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++()
{
    if (_path.empty()) return *this; 

    btree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index; 
        btree_node** child_ptr = &current->_pointers[_index];
        
        while (!(*child_ptr)->_pointers.empty())
        {
            size_t last_child_idx = (*child_ptr)->_pointers.size() - 1;
            _path.push({child_ptr, last_child_idx});
            child_ptr = &(*child_ptr)->_pointers[last_child_idx];
        }

        size_t last_key_idx = (*child_ptr)->_keys.size() - 1;
        _path.push({child_ptr, last_key_idx});
        _index = last_key_idx;
        return *this;
    }

    if (_index > 0)
    {
        _index--;
        _path.top().second = _index;
        return *this;
    }

    while (!_path.empty())
    {
        _path.pop(); 
        
        if (_path.empty()) return *this; 

        size_t child_idx = _path.top().second;
        
        if (child_idx > 0)
        {
            _index = child_idx - 1;
            _path.top().second = _index;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++(int)
{
    self tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--()
{
    if (_path.empty()) return *this; 

    btree_node* current = *_path.top().first;

    if (!current->_pointers.empty()) 
    {
        _path.top().second = _index + 1; 
        btree_node** child_ptr = &current->_pointers[_index + 1];
        
        while (!(*child_ptr)->_pointers.empty())
        {
            _path.push({child_ptr, 0});
            child_ptr = &(*child_ptr)->_pointers[0];
        }
        _path.push({child_ptr, 0});
        _index = 0; 
        return *this;
    }

    _index++;
    if (_index < current->_keys.size())
    {
        _path.top().second = _index; 
        return *this;
    }

    while (!_path.empty()) {
        _path.pop(); 
        
        if (_path.empty())
        {
            _index = 0;
            return *this;
        }

        btree_node* parent = *_path.top().first;
        size_t child_idx = _path.top().second; 

        if (child_idx < parent->_keys.size())
        {
            _index = child_idx;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--(int)
{
    self tmp = *this;
    --(*this);
    return *tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator==(const self& other) const noexcept
{
    if (_path.empty() && other._path.empty()) return true;
    if (_path.empty() != other._path.empty()) return false;

    return _path.top().first == other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) return 0;
    return (*_path.top().first)->_keys.size(); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) return false;
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index)
    : _path(path), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const btree_reverse_iterator& it) noexcept
    : _path(it._path), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_const_iterator() const noexcept
{
    return btree_const_reverse_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++()
{
    if (_path.empty()) return *this; 

    btree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index; 
        btree_node** child_ptr = &current->_pointers[_index];
        
        while (!(*child_ptr)->_pointers.empty())
        {
            size_t last_child_idx = (*child_ptr)->_pointers.size() - 1;
            _path.push({child_ptr, last_child_idx});
            child_ptr = &(*child_ptr)->_pointers[last_child_idx];
        }

        size_t last_key_idx = (*child_ptr)->_keys.size() - 1;
        _path.push({child_ptr, last_key_idx});
        _index = last_key_idx;
        return *this;
    }

    if (_index > 0) 
    {
        _index--;
        _path.top().second = _index;
        return *this;
    }

    while (!_path.empty())
    {
        _path.pop(); 
        
        if (_path.empty()) return *this; 

        size_t child_idx = _path.top().second;
        
        if (child_idx > 0)
        {
            _index = child_idx - 1;
            _path.top().second = _index;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++(int)
{
    self tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--()
{
    if (_path.empty()) return *this; 

    btree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index + 1; 
        btree_node** child_ptr = &current->_pointers[_index + 1];
        
        while (!(*child_ptr)->_pointers.empty())
        {
            _path.push({child_ptr, 0});
            child_ptr = &(*child_ptr)->_pointers[0];
        }
        _path.push({child_ptr, 0});
        _index = 0; 
        return *this;
    }

    _index++;
    if (_index < current->_keys.size())
    {
        _path.top().second = _index; 
        return *this;
    }

    while (!_path.empty())
    {
        _path.pop(); 
        
        if (_path.empty())
        {
            _index = 0;
            return *this;
        }

        btree_node* parent = *_path.top().first;
        size_t child_idx = _path.top().second; 

        if (child_idx < parent->_keys.size())
        {
            _index = child_idx;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--(int)
{
    self tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator==(const self& other) const noexcept
{
    if (_path.empty() && other._path.empty()) return true;
    if (_path.empty() != other._path.empty()) return false;

    return _path.top().first == other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) return 0;
    
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) return false;
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::index() const noexcept
{
    return _index;
}

// endregion iterators implementation

// region element access implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) throw key_not_found_exception();

    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) throw key_not_found_exception();

    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    auto it = find(key);
    if (it != end()) return it->second;

    return emplace(key, tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    auto it = find(key);
    if (it != end()) return it->second;

    return emplace(std::move(key), tvalue()).first->second;
}

// endregion element access implementation

// region iterator begins implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::begin()
{
    if (!_root) return end();

    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node** current_ptr = &_root;

    while (!(*current_ptr)->_pointers.empty())
    {
        path.push({current_ptr, 0});
        current_ptr = &(*current_ptr)->_pointers[0];
    }

    path.push({current_ptr, 0});
    return btree_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::end()
{
    return btree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::begin() const
{
    if (!_root) return btree_const_iterator();

    std::stack<std::pair<btree_node* const*, size_t>> path;
    btree_node* const* current_ptr = &_root;

    while(!(*current_ptr)->_pointers.empty())
    {
        path.push({current_ptr, 0});
        current_ptr = &(*current_ptr)->_pointers[0];
    }

    path.push({current_ptr, 0});
    return btree_const_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::end() const
{
    return btree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin()
{
    if (!_root) return rend();

    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node** current_ptr = &_root;
    
    while (!(*current_ptr)->_pointers.empty())
    {
        size_t last_child = (*current_ptr)->_pointers.size() - 1;
        path.push({current_ptr, last_child});
        current_ptr = &(*current_ptr)->_pointers[last_child];
    }
    
    size_t last_key = (*current_ptr)->_keys.size() - 1;
    path.push({current_ptr, last_key});
    return btree_reverse_iterator(path, last_key);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend()
{
    return btree_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return crbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend() const
{
    return crend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crbegin() const
{
    return rbegin();    
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crend() const
{
    return rend();
}

// endregion iterator begins implementation

// region lookup implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (!_root) return end();
    
    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node** current_ptr = &_root;

    while(*current_ptr)
    {
        btree_node* current = *current_ptr;

        auto it = std::lower_bound
        (
            current->_keys.begin(), current->_keys.end(), key,[this](const tree_data_type& data, const tkey& k)
            {
                return compare_keys(data.first, k);
            }
        );
            
        size_t idx = std::distance(current->_keys.begin(), it);
        
        if (idx < current->_keys.size() && !compare_keys(key, current->_keys[idx].first))
        {
            path.push({current_ptr, idx});
            return btree_iterator(path, idx);
        }
        
        if (current->_pointers.empty()) break; 
        
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }
    
    return end(); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    if (!_root) return end();

    std::stack<std::pair<btree_node* const*, size_t>> path;
    btree_node* const* current_ptr = &_root;

    while(*current_ptr)
    {
        btree_node* current = *current_ptr;
        
        auto it = std::lower_bound
        (
            current->_keys.begin(), current->_keys.end(), key, [this](const tree_data_type_const& data, const tkey& key)
            {
                return compare_keys(data.first, key);
            }
        );

        size_t idx = std::distance(current->_keys.begin(), it);

        if (idx < current->_keys.size() && !compare_keys(current->_keys[idx].first, key))
        {
            path.push({ current_ptr, idx });
            return btree_const_iterator(path, idx);
        }
        
        if (current->_keys.empty()) break;

        path.push({ current_ptr, idx });
        current_ptr = &current->_pointers[idx];
    }

    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    std::stack<std::pair<btree_node**, size_t>> path;

    std::stack<std::pair<btree_node**, size_t>> best_path;
    size_t best_index = 0;

    btree_node** current_ptr = &_root;
    while (*current_ptr)
    {
        btree_node* current = *current_ptr;
        
        auto it = std::lower_bound
        (
            current->_keys.begin(), current->_keys.end(), key, [this](const tree_data_type& data, const tkey& k)
            {
                return compare_keys(data.first, k);
            }
        );
            
        size_t idx = std::distance(current->_keys.begin(), it);
        
        if (idx < current->_keys.size())
        {
            best_path = path;
            best_path.push({current_ptr, idx});
            best_index = idx;
            
            if (!compare_keys(key, current->_keys[idx].first))
            {
                return btree_iterator(best_path, best_index);
            }
        }
        
        if (current->_pointers.empty()) break;
        
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }
    
    if (best_path.empty()) return end();
    return btree_iterator(best_path, best_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    std::stack<std::pair<btree_node**, size_t>> path;

    std::stack<std::pair<btree_node**, size_t>> best_path;
    size_t best_index = 0;

    btree_node** current_ptr = &_root;
    while (*current_ptr)
    {
        btree_node* current = *current_ptr;
        
        auto it = std::lower_bound
        (
            current->_keys.begin(), current->_keys.end(), key, [this](const tree_data_type_const& data, const tkey& k)
            {
                return compare_keys(data.first, k);
            }
        );
            
        size_t idx = std::distance(current->_keys.begin(), it);
        
        if (idx < current->_keys.size())
        {
            best_path = path;
            best_path.push({current_ptr, idx});
            best_index = idx;
            
            if (!compare_keys(key, current->_keys[idx].first))
            {
                return btree_const_iterator(best_path, best_index);
            }
        }
        
        if (current->_pointers.empty()) break;
        
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }
    
    if (best_path.empty()) return end();
    return btree_const_iterator(best_path, best_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    std::stack<std::pair<btree_node**, size_t>> path;
    std::stack<std::pair<btree_node**, size_t>> best_path;
    size_t best_index = 0;
    
    btree_node** current_ptr = &_root;
    
    while (*current_ptr) {
        btree_node* current = *current_ptr;
        
        auto it = std::upper_bound
        (
            current->_keys.begin(), current->_keys.end(), key, 
            [this](const tkey& k, const tree_data_type& data)
            {
                return compare_keys(k, data.first);
            });
            
        size_t idx = std::distance(current->_keys.begin(), it);
        
        if (idx < current->_keys.size())
        {
            best_path = path;
            best_path.push({current_ptr, idx});
            best_index = idx;

            if (!compare_keys(key, current->_keys[idx].first))
            {
                return btree_iterator(best_path, best_index);
            }
        }
        
        if (current->_pointers.empty()) break;
        
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }
    
    if (best_path.empty()) return end();
    return btree_iterator(best_path, best_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    std::stack<std::pair<btree_node* const*, size_t>> path;
    std::stack<std::pair<btree_node* const*, size_t>> best_path;
    size_t best_index = 0;
    
    btree_node** current_ptr = &_root;
    
    while (*current_ptr) {
        btree_node* current = *current_ptr;
        
        auto it = std::upper_bound
        (
            current->_keys.begin(), current->_keys.end(), key, 
            [this](const tkey& k, const tree_data_type& data)
            {
                return compare_keys(k, data.first);
            });
            
        size_t idx = std::distance(current->_keys.begin(), it);
        
        if (idx < current->_keys.size())
        {
            best_path = path;
            best_path.push({current_ptr, idx});
            best_index = idx;

            if (!compare_keys(key, current->_keys[idx].first))
            {
                return btree_const_iterator(best_path, best_index);
            }
        }
        
        if (current->_pointers.empty()) break;
        
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }
    
    if (best_path.empty()) return end();
    return btree_const_iterator(best_path, best_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion lookup implementation

// region modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (!_root) return;

    try
    {
        std::vector<btree_node*> nodes_to_delete;
        nodes_to_delete.reserve(_size);
        nodes_to_delete.push_back(_root);

        while(!nodes_to_delete.empty())
        {
            btree_node* current = nodes_to_delete.back();
            nodes_to_delete.pop_back();

            if (!current->_pointers.empty())
            {
                for (btree_node* child : current->_pointers)
                {
                    if (child) nodes_to_delete.push_back(child);
                }
            }

            _allocator.template delete_object<btree_node>(current);
        }
    }
    catch(...) { }

    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);

    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node** current_ptr = &_root;

    if (!_root)
    {
        try { _root = _allocator.template new_object<btree_node>(); }
        catch(...) { throw memory_allocation_exception(); }

        _root->_keys.push_back(std::move(new_data));
        ++_size;
        path.push({&_root, 0});
        return {btree_iterator(path, 0), true};
    }

    while(*current_ptr)
    {
        btree_node* current = *current_ptr;

        auto it = std::lower_bound
        (
            current->_keys.begin(), current->_keys.end(), new_data.first, [this](const tree_data_type& data, const tkey& key)
            {
                return compare_keys(data.first, key);
            }
        );

        size_t idx = std::distance(current->_keys.begin(), it);

        if (idx < current->_keys.size() && !compare_keys(new_data.first, current->_keys[idx].first))
        {
            path.push({current_ptr, idx});
            return {btree_iterator(path, idx), false};
        }

        path.push({current_ptr, idx});
        if (current->_pointers.empty()) break;
        current_ptr = &(*current_ptr)->_pointers[idx];
    }

    btree_node* leaf = *path.top().first;
    size_t idx = path.top().second;
    leaf->_keys.insert(leaf->_keys.begin() + idx, std::move(new_data));
    ++_size;

    btree_node* target_node = leaf;
    size_t target_idx = idx;

    std::stack<std::pair<btree_node**, size_t>> split_path = path;
    while(!split_path.empty())
    {
        btree_node* node = *split_path.top().first;
        
        if (node->_keys.size() <= maximum_keys_in_node) break;

        split_path.pop();

        btree_node* right;
        try { right = _allocator.template new_object<btree_node>(); }
        catch (...) { throw memory_allocation_exception(); }

        size_t median_idx = t;
        tree_data_type median = std::move(node->_keys[median_idx]);

        right->_keys.insert
        (
            right->_keys.begin(),
            std::make_move_iterator(node->_keys.begin() + median_idx + 1),
            std::make_move_iterator(node->_keys.end())
        );

        if (!node->_pointers.empty())
        {
            right->_pointers.insert
            (
                right->_pointers.begin(),
                std::make_move_iterator(node->_pointers.begin() + median_idx + 1),
                std::make_move_iterator(node->_pointers.end())
            );
        }

        node->_keys.erase(node->_keys.begin() + median_idx, node->_keys.end());
        if (!node->_pointers.empty()) node->_pointers.erase(node->_pointers.begin() + median_idx + 1, node->_pointers.end());

        if (target_node == node)
        {
            if (target_idx == median_idx) target_node = nullptr; 
            else if (target_idx > median_idx)
            {
                target_node = right;
                target_idx = target_idx - median_idx - 1;
            }
        }

        if (split_path.empty())
        {
            btree_node* new_root;
            try { new_root = _allocator.template new_object<btree_node>(); }
            catch (...) { throw memory_allocation_exception(); }

            new_root->_keys.push_back(std::move(median));
            new_root->_pointers.push_back(node);
            new_root->_pointers.push_back(right);
            _root = new_root;

            if (target_node == nullptr)
            {
                target_node = new_root;
                target_idx = 0;
            }
            break;
        }
        else
        {
            btree_node* parent = *split_path.top().first;
            size_t child_idx = split_path.top().second;

            parent->_keys.insert(parent->_keys.begin() + child_idx, std::move(median));
            parent->_pointers.insert(parent->_pointers.begin() + child_idx + 1, right);

            if (target_node == nullptr)
            {
                target_node = parent;
                target_idx = child_idx;
            }
            else if (target_node == parent && target_idx >= child_idx)
            {
                target_idx++;
            }
        }
    }

    const tkey& final_key = target_node->_keys[target_idx].first;
    return {find(final_key), true};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    auto it = find(data.first);
    if (it != end())
    {
        it->second = data.second;
        return it;
    }

    return emplace(data).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    auto it = find(data.first);
    if (it != end())
    {
        it->second = std::move(data.second);
        return it;
    }

    return emplace(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);
    return insert_or_assign(std::move(new_data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos)
{
    if (pos == end()) return end();

    btree_iterator next_it = pos;
    ++next_it;
    std::optional<tkey> next_key = std::nullopt; 
    if (next_it != end()) next_key = next_it->first;

    std::stack<std::pair<btree_node**, size_t>> path = pos._path;
    btree_node* current = *path.top().first;
    size_t idx = path.top().second;

    if (!current->_pointers.empty())
    {
        btree_node** pred_ptr = &current->_pointers[idx];
        std::stack<std::pair<btree_node**, size_t>> pred_path = path;
        pred_path.top().second = idx; 

        while (!(*pred_ptr)->_pointers.empty())
        {
            size_t last_child = (*pred_ptr)->_pointers.size() - 1;
            pred_path.push({pred_ptr, last_child});
            pred_ptr = &(*pred_ptr)->_pointers[last_child];
        }

        btree_node* leaf = *pred_ptr;
        size_t last_key_idx = leaf->_keys.size() - 1;
        pred_path.push({pred_ptr, last_key_idx});

        current->_keys[idx] = std::move(leaf->_keys[last_key_idx]);

        path = std::move(pred_path);
        current = leaf;
        idx = last_key_idx;
    }

    current->_keys.erase(current->_keys.begin() + idx);
    _size--;

    while (path.size() > 1)
    {
        btree_node* node = *path.top().first;
        if (node->_keys.size() >= minimum_keys_in_node) break; 

        path.pop();
        btree_node* parent = *path.top().first;
        size_t child_idx = path.top().second;

        btree_node* left_sibling = (child_idx > 0) ? parent->_pointers[child_idx - 1] : nullptr;
        btree_node* right_sibling = (child_idx < parent->_pointers.size() - 1) ? parent->_pointers[child_idx + 1] : nullptr;

        if (left_sibling && left_sibling->_keys.size() > minimum_keys_in_node)
        {
            node->_keys.insert(node->_keys.begin(), std::move(parent->_keys[child_idx - 1]));
            parent->_keys[child_idx - 1] = std::move(left_sibling->_keys.back());
            left_sibling->_keys.pop_back();

            if (!left_sibling->_pointers.empty())
            {
                node->_pointers.insert(node->_pointers.begin(), left_sibling->_pointers.back());
                left_sibling->_pointers.pop_back();
            }
            break; 
        }
        else if (right_sibling && right_sibling->_keys.size() > minimum_keys_in_node)
        {
            node->_keys.push_back(std::move(parent->_keys[child_idx]));
            parent->_keys[child_idx] = std::move(right_sibling->_keys.front());
            right_sibling->_keys.erase(right_sibling->_keys.begin());

            if (!right_sibling->_pointers.empty())
            {
                node->_pointers.push_back(right_sibling->_pointers.front());
                right_sibling->_pointers.erase(right_sibling->_pointers.begin());
            }
            break; 
        }
        else
        {
            if (left_sibling)
            {
                left_sibling->_keys.push_back(std::move(parent->_keys[child_idx - 1]));
                
                left_sibling->_keys.insert
                (
                    left_sibling->_keys.end(),
                    std::make_move_iterator(node->_keys.begin()),
                    std::make_move_iterator(node->_keys.end())
                );

                if (!node->_pointers.empty())
                {
                    left_sibling->_pointers.insert
                    (
                        left_sibling->_pointers.end(),
                        std::make_move_iterator(node->_pointers.begin()),
                        std::make_move_iterator(node->_pointers.end())
                    );
                }

                parent->_keys.erase(parent->_keys.begin() + child_idx - 1);
                parent->_pointers.erase(parent->_pointers.begin() + child_idx);
                
                _allocator.template delete_object<btree_node>(node);
            }
            else if (right_sibling)
            {
                node->_keys.push_back(std::move(parent->_keys[child_idx]));

                node->_keys.insert
                (
                    node->_keys.end(),
                    std::make_move_iterator(right_sibling->_keys.begin()),
                    std::make_move_iterator(right_sibling->_keys.end())
                );

                if (!right_sibling->_pointers.empty()) 
                {
                    node->_pointers.insert
                    (
                        node->_pointers.end(),
                        std::make_move_iterator(right_sibling->_pointers.begin()),
                        std::make_move_iterator(right_sibling->_pointers.end())
                    );
                }

                parent->_keys.erase(parent->_keys.begin() + child_idx);
                parent->_pointers.erase(parent->_pointers.begin() + child_idx + 1);
                
                _allocator.template delete_object<btree_node>(right_sibling);
            }
        }
    }

    if (_root && _root->_keys.empty())
    {
        if (!_root->_pointers.empty())
        {
            btree_node* old_root = _root;
            _root = _root->_pointers[0];
            _allocator.template delete_object<btree_node>(old_root);
        }
        else
        {
            _allocator.template delete_object<btree_node>(_root);
            _root = nullptr;
        }
    }

    if (next_key.has_value())
    {
        return find(next_key.value());
    }
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos)
{
    return erase(find(pos->first));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, btree_iterator en)
{
    std::vector<tkey> keys_to_delete;
    for (auto it = beg; it != en; ++it)
    {
        keys_to_delete.push_back(it->first);
    }
    
    for (const auto& k : keys_to_delete)
    {
        erase(k);
    }
    
    if (keys_to_delete.empty()) return en;
    return lower_bound(keys_to_delete.back()); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg, btree_const_iterator en)
{
    btree_iterator non_const_beg = (beg != cend()) ? find(beg->first) : end();
    btree_iterator non_const_en = (en != cend()) ? find(en->first) : end();
    return erase(non_const_beg, non_const_en);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    btree_iterator it = find(key);
    if (it != end())
    {
        return erase(it);
    }

    return end();
}

// endregion modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_pairs(const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &lhs,
                   const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &rhs)
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_keys(const tkey &lhs, const tkey &rhs)
{
    return compare::operator()(lhs, rhs);
}


#endif