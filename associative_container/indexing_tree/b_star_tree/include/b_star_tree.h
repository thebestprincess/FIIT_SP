#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

#ifndef SYS_PROG_BS_TREE_H
#define SYS_PROG_BS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BS_tree final : private compare
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = 2 * t - 1;
    static constexpr const size_t maximum_keys_in_node = 3 * t;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

    struct bstree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bstree_node*, maximum_keys_in_node + 2> _pointers;
        bstree_node() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bstree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BS_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BS_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BS_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BS_tree(const BS_tree& other);

    BS_tree(BS_tree&& other) noexcept;

    BS_tree& operator=(const BS_tree& other);

    BS_tree& operator=(BS_tree&& other) noexcept;

    ~BS_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bstree_iterator;
    class bstree_reverse_iterator;
    class bstree_const_iterator;
    class bstree_const_reverse_iterator;

    class bstree_iterator final
    {
        std::stack<std::pair<bstree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_const_iterator;
        friend class bstree_const_reverse_iterator;

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

        explicit bstree_iterator(const std::stack<std::pair<bstree_node**, size_t>>& path = std::stack<std::pair<bstree_node**, size_t>>(), size_t index = 0);

    };

    class bstree_const_iterator final
    {
        std::stack<std::pair<bstree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_const_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_iterator;
        friend class bstree_const_reverse_iterator;

        bstree_const_iterator(const bstree_iterator& it) noexcept;

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

        explicit bstree_const_iterator(const std::stack<std::pair<bstree_node* const*, size_t>>& path = std::stack<std::pair<bstree_node* const*, size_t>>(), size_t index = 0);
    };

    class bstree_reverse_iterator final
    {
        std::stack<std::pair<bstree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_reverse_iterator;

        friend class BS_tree;
        friend class bstree_iterator;
        friend class bstree_const_iterator;
        friend class bstree_const_reverse_iterator;

        bstree_reverse_iterator(const bstree_iterator& it) noexcept;
        operator bstree_iterator() const noexcept;

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

        explicit bstree_reverse_iterator(const std::stack<std::pair<bstree_node**, size_t>>& path = std::stack<std::pair<bstree_node**, size_t>>(), size_t index = 0);
    };

    class bstree_const_reverse_iterator final
    {
        std::stack<std::pair<bstree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_const_reverse_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_const_iterator;
        friend class bstree_iterator;

        bstree_const_reverse_iterator(const bstree_reverse_iterator& it) noexcept;
        operator bstree_const_iterator() const noexcept;

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

        explicit bstree_const_reverse_iterator(const std::stack<std::pair<bstree_node* const*, size_t>>& path = std::stack<std::pair<bstree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class bstree_iterator;
    friend class bstree_const_iterator;
    friend class bstree_reverse_iterator;
    friend class bstree_const_reverse_iterator;

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

    bstree_iterator begin();
    bstree_iterator end();

    bstree_const_iterator begin() const;
    bstree_const_iterator end() const;

    bstree_const_iterator cbegin() const;
    bstree_const_iterator cend() const;

    bstree_reverse_iterator rbegin();
    bstree_reverse_iterator rend();

    bstree_const_reverse_iterator rbegin() const;
    bstree_const_reverse_iterator rend() const;

    bstree_const_reverse_iterator crbegin() const;
    bstree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bstree_iterator find(const tkey& key);
    bstree_const_iterator find(const tkey& key) const;

    bstree_iterator lower_bound(const tkey& key);
    bstree_const_iterator lower_bound(const tkey& key) const;

    bstree_iterator upper_bound(const tkey& key);
    bstree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bstree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bstree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bstree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bstree_iterator insert_or_assign(const tree_data_type& data);
    bstree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bstree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bstree_iterator erase(bstree_iterator pos);
    bstree_iterator erase(bstree_const_iterator pos);

    bstree_iterator erase(bstree_iterator beg, bstree_iterator en);
    bstree_iterator erase(bstree_const_iterator beg, bstree_const_iterator en);


    bstree_iterator erase(const tkey& key);

    // endregion modifiers declaration
public:

    class bstree_exception: public std::exception
    {
    protected:
        std::string _msg;
    public:
        explicit bstree_exception(std::string msg): _msg(std::move(msg)) { }
        const char* what() const noexcept override { return _msg.c_str(); }
    };

    class key_not_found_exception final : public bstree_exception
    {
    public:
        explicit key_not_found_exception(): bstree_exception("BS-Tree: key not found") { }
    };

    class memory_allocation_exception: public bstree_exception
    {
    public:
        explicit memory_allocation_exception(): bstree_exception("BS-Tree: failed to allocate memory") { } 
    };

    class duplicate_key_exception: public bstree_exception
    {
    public:
        explicit duplicate_key_exception(): bstree_exception("BS-Tree: key already exists") { }
    };

};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BS_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BS_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BS_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::compare_pairs(const BS_tree::tree_data_type &lhs,
                                                     const BS_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_node::bstree_node() noexcept: _keys(), _pointers() { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BS_tree<tkey, tvalue, compare, t>::value_type> BS_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::pointer BS_tree<tkey, tvalue, compare, t>::bstree_iterator
::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self & BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator++()
{
    if (_path.empty()) return *this;

    bstree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index + 1;
        bstree_node** child_ptr = &current->_pointers[_index + 1];
        
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

        bstree_node* parent = *_path.top().first;
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self & BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator--()
{
    if (_path.empty()) return *this;

    bstree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index;
        bstree_node** child_ptr = &current->_pointers[_index];

        while(!(*child_ptr)->_pointers.empty())
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator==(const self &other) const noexcept
{
    if (_path.empty() && other._path.empty()) return true;
    if (_path.empty() != other._path.empty()) return false;

    return _path.top().first == other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) return 0;
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) return false;
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::bstree_iterator(
    const std::stack<std::pair<bstree_node **, size_t>> &path, size_t index)
    : _path(path), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::bstree_const_iterator(const bstree_iterator &it) noexcept
    : _path(it._path), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator++()
{
    if (_path.empty()) return *this;

    bstree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index + 1;
        bstree_node* const* child_ptr = &current->_pointers[_index + 1];

        while(!(*child_ptr)->_pointers.empty())
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

    while(!_path.empty())
    {
        _path.pop();

        if (_path.empty())
        {
            _index = 0;
            return *this;
        }

        bstree_node* parent = *_path.top().first;
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator--()
{
    if (_path.empty()) return *this;

    bstree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index;
        bstree_node** child_ptr = &current->_pointers[_index];

        while(!(*child_ptr)->_pointers.empty())
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator==(const self &other) const noexcept
{
    if (_path.empty() && other._path.empty()) return true;
    if (_path.empty() != other._path.empty()) return false;

    return _path.top().first == other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) return 0;
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) return false;
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::bstree_const_iterator(
    const std::stack<std::pair<bstree_node * const*, size_t>> &path, size_t index)
    : _path(path), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::bstree_reverse_iterator(const bstree_iterator &it) noexcept
    : _path(it._path), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator BS_tree<tkey, tvalue, compare, t>::bstree_iterator() const noexcept
{
    return bstree_reverse_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator++()
{
    if (_path.empty()) return *this; 

    bstree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index; 
        bstree_node** child_ptr = &current->_pointers[_index];
        
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator--()
{
    if (_path.empty()) return *this; 

    bstree_node* current = *_path.top().first;

    if (!current->_pointers.empty()) 
    {
        _path.top().second = _index + 1; 
        bstree_node** child_ptr = &current->_pointers[_index + 1];
        
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

        bstree_node* parent = *_path.top().first;
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator==(const self &other) const noexcept
{
    if (_path.empty() && other._path.empty()) return true;
    if (_path.empty() != other._path.empty()) return false;

    return _path.top().first == other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) return 0;
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) return false;
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::bstree_reverse_iterator(
    const std::stack<std::pair<bstree_node **, size_t>> &path, size_t index)
    : _path(path), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::bstree_const_reverse_iterator(
    const bstree_reverse_iterator &it) noexcept
    : _path(it._path), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator() const noexcept
{
    return bstree_const_reverse_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator++()
{
    if (_path.empty()) return *this; 

    bstree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index; 
        bstree_node* const* child_ptr = &current->_pointers[_index];
        
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator--()
{
    if (_path.empty()) return *this; 

    bstree_node* current = *_path.top().first;

    if (!current->_pointers.empty())
    {
        _path.top().second = _index; 
        bstree_node* const* child_ptr = &current->_pointers[_index];
        
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator==(const self &other) const noexcept
{
    if (_path.empty() && other._path.empty()) return true;
    if (_path.empty() != other._path.empty()) return false;

    return _path.top().first == other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) return 0;
    
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) return false;
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::bstree_const_reverse_iterator(
    const std::stack<std::pair<bstree_node * const*, size_t>> &path, size_t index)
    : _path(path), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(const compare& cmp, pp_allocator<value_type> alloc)
    : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(pp_allocator<value_type> alloc, const compare& comp)
    : compare(comp), _allocator(alloc), _root(nullptr), _size(0) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BS_tree<tkey, tvalue, compare, t>::BS_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
    : BS_tree(cmp, alloc)
{
    for (auto it = begin; it != end; ++it) insert(*it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
    : BS_tree(cmp, alloc)
{
    for (const auto& item : data) insert(item);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(const BS_tree& other)
    : compare(other.compare), _allocator(other._allocator.select_on_container_copy_construction()), _root(nullptr), _size(0) 
{
    if (!other._root) return;

    std::vector<bstree_node*> allocated_nodes;
    allocated_nodes.reserve(other._size);

    auto copy_node = [&](auto& self, bstree_node* other_node) -> bstree_node*
    {
        if (!other_node) return nullptr;
        bstree_node* new_node;
        try { new_node = _allocator.template new_object<bstree_node>(); }
        catch (...) { throw memory_allocation_exception(); }

        allocated_nodes.push_back(new_node); 
        new_node->_keys = other_node->_keys;

        for (bstree_node* child : other_node->_pointers)
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
        for (auto it = allocated_nodes.rbegin(); it != allocated_nodes.rend(); ++it) {
            _allocator.template delete_object<bstree_node>(*it);
        }
        _root = nullptr;
        _size = 0;
        throw;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(BS_tree&& other) noexcept
    : compare(std::move(static_cast<compare&>(other))), _allocator(std::move(other._allocator)), _root(other._root), _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>& BS_tree<tkey, tvalue, compare, t>::operator=(const BS_tree& other)
{
    if (this == &other) return *this;
    std::vector<bstree_node*> allocated_nodes;
    allocated_nodes.reserve(other._size);

    auto copy_node = [&](auto& self, bstree_node* other_node) -> bstree_node*
    {
        if (!other_node) return nullptr;
        bstree_node* new_node;
        try { new_node = _allocator.template new_object<bstree_node>(); }
        catch (...) { throw memory_allocation_exception(); }

        allocated_nodes.push_back(new_node);
        new_node->_keys = other_node->_keys;

        for (bstree_node* child : other_node->_pointers)
        {
            new_node->_pointers.push_back(self(self, child));
        }

        return new_node;
    };

    bstree_node* new_root = nullptr;
    try
    {
        if (other._root) new_root = copy_node(copy_node, other._root);
    }
    catch (...)
    {
        for (auto it = allocated_nodes.rbegin(); it != allocated_nodes.rend(); ++it)
            _allocator.template delete_object<bstree_node>(*it);

        throw;
    }

    clear(); 
    static_cast<compare&>(*this) = static_cast<const compare&>(other);
    _root = new_root;
    _size = other._size;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>& BS_tree<tkey, tvalue, compare, t>::operator=(BS_tree&& other) noexcept
{
    if (this == &other) return *this;

    clear();

    static_cast<compare&>(*this) = std::move(static_cast<compare&>(other));
    if constexpr (pp_allocator<value_type>::propagate_on_container_move_assignment::value)
        _allocator = std::move(other._allocator);

    _root = other._root;
    _size = other._size;
    other._root = nullptr;
    other._size = 0;

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::~BS_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) throw key_not_found_exception();
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BS_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) throw key_not_found_exception();
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    auto it = find(key);
    if (it != end()) return it->second;
    return emplace(key, tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    auto it = find(key);
    if (it != end()) return it->second;
    return emplace(std::move(key), tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::begin()
{
    if (!_root) return end();

    std::stack<std::pair<bstree_node**, size_t>> path;

    bstree_node** current_ptr = &_root;
    while (!(*current_ptr)->_pointers.empty())
    {
        path.push({current_ptr, 0});
        current_ptr = &(*current_ptr)->_pointers[0];
    }
    path.push({current_ptr, 0});

    return bstree_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::end()
{
    return bstree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::begin() const
{
    if (!_root) return end();
    
    std::stack<std::pair<bstree_node* const*, size_t>> path;

    bstree_node* const* current_ptr = &_root;
    while(!(*current_ptr)->_pointers.empty())
    {
        path.push({current_ptr, 0});
        current_ptr = &(*current_ptr)->_pointers[0];
    }
    path.push({current_ptr, 0});

    return bstree_const_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::end() const
{
    return bstree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rbegin()
{
    if (!_root) return rend();

    std::stack<std::pair<bstree_node**, size_t>> path;
    bstree_node** current_ptr = &_root;

    while(!(*current_ptr)->_pointers.empty())
    {
        size_t last_child = (*current_ptr)->_pointers.size() - 1;
        path.push({current_ptr, last_child});
        current_ptr = &(*current_ptr)->_pointers[last_child];
    }
    size_t last_key = (*current_ptr)->_keys.size() - 1;
    path.push({current_ptr, last_key});

    return bstree_reverse_iterator(path, last_key);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rend()
{
    return bstree_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return crbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rend() const
{
    return crend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::crbegin() const
{
    return rbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::crend() const
{
    return rend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (!_root) return end();

    std::stack<std::pair<bstree_node**, size_t>> path;

    bstree_node** current_ptr = &_root;
    while(*current_ptr)
    {
        bstree_node* current = *current_ptr;
        auto it = std::lower_bound
        (
            current->_keys.begin(), current->_keys.end(), key, [this](const tree_data_type& data, const tkey& k)
            {
                return compare_keys(data.first, k);
            }
        );

        size_t idx = std::distance(current->_keys.begin(), it);
        
        if (idx < current->_keys.size() && !compare_keys(key, current->_keys[idx].first))
        {
            path.push({current_ptr, idx});
            return bstree_iterator(path, idx);
        }

        if (current->_pointers.empty()) break; 
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }

    return end(); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    if (!_root) return end();

    std::stack<std::pair<bstree_node* const*, size_t>> path;

    bstree_node* const* current_ptr = &_root;
    while(*current_ptr)
    {
        bstree_node* current = *current_ptr;
        auto it = std::lower_bound
        (
            current->_keys.begin(), current->_keys.end(), key, [this](const tree_data_type& data, const tkey& k)
            {
                return compare_keys(data.first, k);
            }
        );

        size_t idx = std::distance(current->_keys.begin(), it);
        
        if (idx < current->_keys.size() && !compare_keys(key, current->_keys[idx].first))
        {
            path.push({current_ptr, idx});
            return bstree_const_iterator(path, idx);
        }

        if (current->_pointers.empty()) break; 
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }

    return end(); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    std::stack<std::pair<bstree_node**, size_t>> path, best_path;
    size_t best_index = 0;

    bstree_node** current_ptr = &_root;
    while (*current_ptr)
    {
        bstree_node* current = *current_ptr;

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
            if (!compare_keys(key, current->_keys[idx].first)) return bstree_iterator(best_path, best_index);
        }

        if (current->_pointers.empty()) break;
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }

    if (best_path.empty()) return end();
    return bstree_iterator(best_path, best_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    std::stack<std::pair<bstree_node* const*, size_t>> path, best_path;
    size_t best_index = 0;

    bstree_node* const* current_ptr = &_root;
    while (*current_ptr)
    {
        bstree_node* current = *current_ptr;
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
            if (!compare_keys(key, current->_keys[idx].first)) return bstree_const_iterator(best_path, best_index);
        }

        if (current->_pointers.empty()) break;
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }

    if (best_path.empty()) return end();
    return bstree_const_iterator(best_path, best_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    std::stack<std::pair<bstree_node**, size_t>> path;
    std::stack<std::pair<bstree_node**, size_t>> best_path;
    size_t best_index = 0;
    
    bstree_node** current_ptr = &_root;
    
    while (*current_ptr)
    {
        bstree_node* current = *current_ptr;
        
        auto it = std::upper_bound
        (
            current->_keys.begin(), current->_keys.end(), key, 
            [this](const tkey& k, const tree_data_type& data)
            {
                return compare_keys(k, data.first);
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
                return bstree_iterator(best_path, best_index);
            }
        }
        
        if (current->_pointers.empty()) break;
        
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }
    
    if (best_path.empty()) return end();
    return bstree_iterator(best_path, best_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    std::stack<std::pair<bstree_node* const*, size_t>> path;
    std::stack<std::pair<bstree_node* const*, size_t>> best_path;
    size_t best_index = 0;
    
    bstree_node* const* current_ptr = &_root;
    
    while (*current_ptr)
    {
        bstree_node* const current = *current_ptr;
        
        auto it = std::upper_bound
        (
            current->_keys.begin(), current->_keys.end(), key, 
            [this](const tkey& k, const tree_data_type& data)
            {
                return compare_keys(k, data.first);
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
                return bstree_iterator(best_path, best_index);
            }
        }
        
        if (current->_pointers.empty()) break;
        
        path.push({current_ptr, idx});
        current_ptr = &current->_pointers[idx];
    }
    
    if (best_path.empty()) return end();
    return bstree_iterator(best_path, best_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (!_root) return;
    try
    {
        std::vector<bstree_node*> nodes_to_delete;
        nodes_to_delete.reserve(_size);
        nodes_to_delete.push_back(_root);
        while(!nodes_to_delete.empty())
        {
            bstree_node* current = nodes_to_delete.back();
            nodes_to_delete.pop_back();
            if (!current->_pointers.empty())
            {
                for (bstree_node* child : current->_pointers)
                    if (child) nodes_to_delete.push_back(child);
                    
            }
            _allocator.template delete_object<bstree_node>(current);
        }
    }
    catch (...) { }

    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> BS_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> BS_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> BS_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);

    if (!_root)
    {
        try { _root = _allocator.template new_object<bstree_node>(); }
        catch (...) { throw memory_allocation_exception(); }
        _root->_keys.push_back(std::move(new_data));
        _size++;

        return {find(new_data.first), true}; 
    }

    std::stack<std::pair<bstree_node**, size_t>> path;
    bstree_node** current_ptr = &_root;

    while (*current_ptr)
    {
        bstree_node* current = *current_ptr;
        auto it = std::lower_bound
        (
            current->_keys.begin(), current->_keys.end(), new_data.first, [this](const tree_data_type& data, const tkey& k) 
            { 
                return compare_keys(data.first, k);
            }
        );
            
        size_t idx = std::distance(current->_keys.begin(), it);
        
        if (idx < current->_keys.size() && !compare_keys(new_data.first, current->_keys[idx].first))
            return {find(new_data.first), false}; 
        
        path.push({current_ptr, idx});
        if (current->_pointers.empty()) break; 
        current_ptr = &current->_pointers[idx];
    }

    bstree_node* leaf = *path.top().first;
    size_t idx = path.top().second;
    leaf->_keys.insert(leaf->_keys.begin() + idx, std::move(new_data));
    _size++;

    std::stack<std::pair<bstree_node**, size_t>> split_path = path;

    while (!split_path.empty())
    {
        bstree_node** node_ptr_ptr = split_path.top().first;
        bstree_node* node = *node_ptr_ptr;

        if (node->_keys.size() <= maximum_keys_in_node) break; 

        split_path.pop();

        if (split_path.empty())
        {
            bstree_node* right;
            bstree_node* new_root;
            try
            { 
                right = _allocator.template new_object<bstree_node>(); 
                new_root = _allocator.template new_object<bstree_node>();
            }
            catch (...) { throw memory_allocation_exception(); }

            size_t median_idx = 3 * (t - 1);
            if (median_idx == 0 || median_idx >= node->_keys.size())
                median_idx = node->_keys.size() / 2;

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
                node->_pointers.erase(node->_pointers.begin() + median_idx + 1, node->_pointers.end());
            }

            node->_keys.erase(node->_keys.begin() + median_idx, node->_keys.end());

            new_root->_keys.push_back(std::move(median));
            new_root->_pointers.push_back(node);
            new_root->_pointers.push_back(right);
            _root = new_root;
            break;
        }

        bstree_node* parent = *split_path.top().first;
        size_t child_idx = split_path.top().second;

        bstree_node* left_sibling = (child_idx > 0) ? parent->_pointers[child_idx - 1] : nullptr;
        bstree_node* right_sibling = (child_idx < parent->_pointers.size() - 1) ? parent->_pointers[child_idx + 1] : nullptr;

        bstree_node* sibling_to_borrow = nullptr;
        bool is_right = false;

        if (right_sibling && right_sibling->_keys.size() < maximum_keys_in_node)
        {
            sibling_to_borrow = right_sibling;
            is_right = true;
        }
        else if (left_sibling && left_sibling->_keys.size() < maximum_keys_in_node)
        {
            sibling_to_borrow = left_sibling;
            is_right = false;
        }

        if (sibling_to_borrow)
        {
            bstree_node* left_node = is_right ? node : sibling_to_borrow;
            bstree_node* right_node = is_right ? sibling_to_borrow : node;
            size_t parent_key_idx = is_right ? child_idx : child_idx - 1;

            std::vector<tree_data_type> temp_keys;
            std::vector<bstree_node*> temp_ptrs;
            
            for (auto& k : left_node->_keys) temp_keys.push_back(std::move(k));
            temp_keys.push_back(std::move(parent->_keys[parent_key_idx]));
            for (auto& k : right_node->_keys) temp_keys.push_back(std::move(k));

            if (!left_node->_pointers.empty())
            {
                for (auto* p : left_node->_pointers) temp_ptrs.push_back(p);
                for (auto* p : right_node->_pointers) temp_ptrs.push_back(p);
            }

            left_node->_keys.clear();
            left_node->_pointers.clear();
            right_node->_keys.clear();
            right_node->_pointers.clear();

            size_t total = temp_keys.size();
            size_t left_size = total / 2;
            size_t right_size = total - 1 - left_size;

            size_t k_idx = 0;
            size_t p_idx = 0;

            for(size_t i = 0; i < left_size; ++i) left_node->_keys.push_back(std::move(temp_keys[k_idx++]));
            if (!temp_ptrs.empty()) 
                for(size_t i = 0; i <= left_size; ++i) left_node->_pointers.push_back(temp_ptrs[p_idx++]);

            parent->_keys[parent_key_idx] = std::move(temp_keys[k_idx++]); 

            for(size_t i = 0; i < right_size; ++i) right_node->_keys.push_back(std::move(temp_keys[k_idx++]));
            if (!temp_ptrs.empty())
                while(p_idx < temp_ptrs.size()) right_node->_pointers.push_back(temp_ptrs[p_idx++]);
            
            break; 
        }

        bstree_node* sibling = right_sibling ? right_sibling : left_sibling;
        bool is_right_sibling = (sibling == right_sibling);
        size_t parent_key_idx = is_right_sibling ? child_idx : child_idx - 1;

        bstree_node* left_node = is_right_sibling ? node : sibling;
        bstree_node* right_node = is_right_sibling ? sibling : node;

        std::vector<tree_data_type> temp_keys;
        std::vector<bstree_node*> temp_ptrs;
        
        for (auto& k : left_node->_keys) temp_keys.push_back(std::move(k));
        temp_keys.push_back(std::move(parent->_keys[parent_key_idx]));
        for (auto& k : right_node->_keys) temp_keys.push_back(std::move(k));

        if (!left_node->_pointers.empty())
        {
            for (auto* p : left_node->_pointers) temp_ptrs.push_back(p);
            for (auto* p : right_node->_pointers) temp_ptrs.push_back(p);
        }

        left_node->_keys.clear();
        left_node->_pointers.clear();
        right_node->_keys.clear();
        right_node->_pointers.clear();

        bstree_node* middle_node;
        try { middle_node = _allocator.template new_object<bstree_node>(); }
        catch (...) { throw memory_allocation_exception(); }

        size_t total_keys = temp_keys.size();
        size_t keys_per_node = (total_keys - 2) / 3;
        size_t left_size = keys_per_node;
        size_t middle_size = keys_per_node;
        size_t right_size = total_keys - 2 - left_size - middle_size; 

        size_t temp_k_idx = 0;
        size_t temp_p_idx = 0;

        for (size_t i = 0; i < left_size; ++i) left_node->_keys.push_back(std::move(temp_keys[temp_k_idx++]));
        if (!temp_ptrs.empty())
            for (size_t i = 0; i <= left_size; ++i) left_node->_pointers.push_back(temp_ptrs[temp_p_idx++]);

        tree_data_type first_median = std::move(temp_keys[temp_k_idx++]);

        for (size_t i = 0; i < middle_size; ++i) middle_node->_keys.push_back(std::move(temp_keys[temp_k_idx++]));
        if (!temp_ptrs.empty())
            for (size_t i = 0; i <= middle_size; ++i) middle_node->_pointers.push_back(temp_ptrs[temp_p_idx++]);

        tree_data_type second_median = std::move(temp_keys[temp_k_idx++]);

        for (size_t i = 0; i < right_size; ++i) right_node->_keys.push_back(std::move(temp_keys[temp_k_idx++]));
        if (!temp_ptrs.empty())
            while (temp_p_idx < temp_ptrs.size()) right_node->_pointers.push_back(temp_ptrs[temp_p_idx++]);

        parent->_keys[parent_key_idx] = std::move(first_median);
        parent->_keys.insert(parent->_keys.begin() + parent_key_idx + 1, std::move(second_median));
        parent->_pointers.insert(parent->_pointers.begin() + parent_key_idx + 1, middle_node);
    }

    return {find(new_data.first), true};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
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
template <typename ...Args>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);
    return insert_or_assign(std::move(new_data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_iterator pos)
{
    if (pos == end()) return end();

    bstree_iterator next_it = pos;
    ++next_it;
    std::optional<tkey> next_key = std::nullopt;
    if (next_it != end()) next_key = next_it->first;

    std::stack<std::pair<bstree_node**, size_t>> path = pos._path;
    bstree_node* current = *path.top().first;
    size_t idx = path.top().second;

    if (!current->_pointers.empty())
    {
        bstree_node** pred_ptr = &current->_pointers[idx];
        std::stack<std::pair<bstree_node**, size_t>> pred_path = path;
        pred_path.top().second = idx;

        while (!(*pred_ptr)->_pointers.empty())
        {
            size_t last_child = (*pred_ptr)->_pointers.size() - 1;
            pred_path.push({pred_ptr, last_child});
            pred_ptr = &(*pred_ptr)->_pointers[last_child];
        }

        bstree_node* leaf = *pred_ptr;
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
        bstree_node* node = *path.top().first;
        if (node->_keys.size() >= minimum_keys_in_node) break; 

        path.pop();
        bstree_node* parent = *path.top().first;
        size_t child_idx = path.top().second;

        size_t start_idx = 0;
        size_t end_idx = 0;

        if (parent->_pointers.size() >= 3)
        {
            if (child_idx == 0)
            {
                start_idx = 0;
                end_idx = 2;
            }
            else if (child_idx == parent->_pointers.size() - 1)
            {
                start_idx = child_idx - 2; end_idx = child_idx;
            }
            else
            {
                start_idx = child_idx - 1; end_idx = child_idx + 1;
            }
        }
        else
        {
            start_idx = 0; end_idx = 1;
        }

        size_t num_pooled_nodes = end_idx - start_idx + 1;

        std::vector<tree_data_type> temp_keys;
        std::vector<bstree_node*> temp_ptrs;

        for (size_t i = start_idx; i <= end_idx; ++i)
        {
            bstree_node* child = parent->_pointers[i];
            for (auto& k : child->_keys) temp_keys.push_back(std::move(k));
            
            if (!child->_pointers.empty())
                for (auto* p : child->_pointers) temp_ptrs.push_back(p);
            
            if (i < end_idx)
                temp_keys.push_back(std::move(parent->_keys[start_idx + (i - start_idx)]));
        }

        size_t target_node_count = num_pooled_nodes;
        if (num_pooled_nodes > 1)
        {
            size_t keys_for_nodes = temp_keys.size() - (num_pooled_nodes - 2);
            size_t max_capacity = (num_pooled_nodes - 1) * maximum_keys_in_node;
            
            if (parent == _root && num_pooled_nodes == 2) max_capacity += 1;
            
            if (keys_for_nodes <= max_capacity) target_node_count = num_pooled_nodes - 1; 
        } 

        std::vector<bstree_node*> new_nodes;
        try
        {
            for (size_t i = 0; i < target_node_count; ++i)
            {
                new_nodes.push_back(_allocator.template new_object<bstree_node>());
            }
        }
        catch (...)
        {
            for (auto* n : new_nodes) _allocator.template delete_object<bstree_node>(n);
            throw memory_allocation_exception();
        }

        size_t base_keys = (temp_keys.size() - (target_node_count - 1)) / target_node_count;
        size_t remainder = (temp_keys.size() - (target_node_count - 1)) % target_node_count;

        size_t key_idx = 0;
        size_t ptr_idx = 0;
        std::vector<tree_data_type> new_parent_keys;

        for (size_t i = 0; i < target_node_count; ++i) {
            size_t keys_to_put = base_keys + (i < remainder ? 1 : 0);
            
            for (size_t k = 0; k < keys_to_put; ++k) {
                new_nodes[i]->_keys.push_back(std::move(temp_keys[key_idx++]));
            }
            
            if (!temp_ptrs.empty()) {
                for (size_t k = 0; k <= keys_to_put; ++k) {
                    new_nodes[i]->_pointers.push_back(temp_ptrs[ptr_idx++]);
                }
            }
            
            if (i < target_node_count - 1) {
                new_parent_keys.push_back(std::move(temp_keys[key_idx++]));
            }
        }

        std::vector<bstree_node*> old_nodes;
        for (size_t i = start_idx; i <= end_idx; ++i)
            old_nodes.push_back(parent->_pointers[i]);

        parent->_keys.erase(parent->_keys.begin() + start_idx, parent->_keys.begin() + end_idx);
        parent->_pointers.erase(parent->_pointers.begin() + start_idx, parent->_pointers.begin() + end_idx + 1);

        parent->_keys.insert(parent->_keys.begin() + start_idx,
            std::make_move_iterator(new_parent_keys.begin()),
            std::make_move_iterator(new_parent_keys.end()));

        parent->_pointers.insert(parent->_pointers.begin() + start_idx, new_nodes.begin(), new_nodes.end());

        for (auto* old_n : old_nodes)
            _allocator.template delete_object<bstree_node>(old_n);
    }

    if (_root && _root->_keys.empty())
    {
        if (!_root->_pointers.empty())
        {
            bstree_node* old_root = _root;
            _root = _root->_pointers[0];
            _allocator.template delete_object<bstree_node>(old_root);
        }
        else
        {
            _allocator.template delete_object<bstree_node>(_root);
            _root = nullptr;
        }
    }

    if (next_key.has_value()) return find(next_key.value());
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_const_iterator pos)
{
    return erase(find(pos->first));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_iterator beg, bstree_iterator en)
{
    std::vector<tkey> keys_to_delete;
    for (auto it = beg; it != en; ++it) keys_to_delete.push_back(it->first);
    for (const auto& k : keys_to_delete) erase(k);
    if (keys_to_delete.empty()) return en;
    return lower_bound(keys_to_delete.back());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_const_iterator beg, bstree_const_iterator en)
{
    auto non_const_beg = (beg != cend()) ? find(beg->first) : end();
    auto non_const_en = (en != cend()) ? find(en->first) : end();
    return erase(non_const_beg, non_const_en);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto it = find(key);
    if (it != end()) return erase(it);
    return end();
}

#endif