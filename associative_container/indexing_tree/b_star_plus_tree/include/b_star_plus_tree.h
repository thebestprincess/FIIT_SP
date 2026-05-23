#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <initializer_list>
#include <not_implemented.h>

#ifndef SYS_PROG_BS_PLUS_TREE_H
#define SYS_PROG_BS_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BSP_tree final : private compare
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

    struct bsptree_node_base
    {
        bool _is_terminated;

        bsptree_node_base() noexcept;
        virtual ~bsptree_node_base() =default;
    };

    struct bsptree_node_term : public bsptree_node_base
    {
        bsptree_node_term* _next;
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;
        bsptree_node_term() noexcept;
    };

    struct bsptree_node_middle : public bsptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bsptree_node_base*, maximum_keys_in_node + 2> _pointers;
        bsptree_node_middle() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bsptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BSP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BSP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BSP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BSP_tree(const BSP_tree& other);

    BSP_tree(BSP_tree&& other) noexcept;

    BSP_tree& operator=(const BSP_tree& other);

    BSP_tree& operator=(BSP_tree&& other) noexcept;

    ~BSP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bsptree_iterator;
    class bsptree_const_iterator;

    class bsptree_iterator final
    {
        bsptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_iterator;

        friend class BSP_tree;
        friend class bsptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bsptree_iterator(bsptree_node_term* node = nullptr, size_t index = 0);

    };

    class bsptree_const_iterator final
    {
        const bsptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_const_iterator;

        friend class BSP_tree;
        friend class bsptree_iterator;

        bsptree_const_iterator(const bsptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bsptree_const_iterator(const bsptree_node_term* node = nullptr, size_t index = 0);
    };

    friend class bsptree_iterator;
    friend class bsptree_const_iterator;

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

    bsptree_iterator begin();
    bsptree_iterator end();

    bsptree_const_iterator begin() const;
    bsptree_const_iterator end() const;

    bsptree_const_iterator cbegin() const;
    bsptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bsptree_iterator find(const tkey& key);
    bsptree_const_iterator find(const tkey& key) const;

    bsptree_iterator lower_bound(const tkey& key);
    bsptree_const_iterator lower_bound(const tkey& key) const;

    bsptree_iterator upper_bound(const tkey& key);
    bsptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bsptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bsptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bsptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bsptree_iterator insert_or_assign(const tree_data_type& data);
    bsptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bsptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bsptree_iterator erase(bsptree_iterator pos);
    bsptree_iterator erase(bsptree_const_iterator pos);

    bsptree_iterator erase(bsptree_iterator beg, bsptree_iterator en);
    bsptree_iterator erase(bsptree_const_iterator beg, bsptree_const_iterator en);


    bsptree_iterator erase(const tkey& key);

    // endregion modifiers declaration

public:

    class bsptree_exception : public std::exception
    {
    private:
        std::string _message;
    public:
        explicit bsptree_exception(std::string message): _message(std::move(message)) { }
        const char* what() const noexcept override { return _message.c_str(); }
    };

    class key_not_found_exception final : public bsptree_exception
    {
    public:
        key_not_found_exception(): bsptree_exception("BSP-Tree: key not found") { }
    };

    class memory_allocation_exception final : public bsptree_exception
    {
    public:
        memory_allocation_exception(): bsptree_exception("BSP-Tree: failed to allocate memory") { }
    };

    class duplicate_key_exception final : public bsptree_exception
    {
    public:
        duplicate_key_exception(): bsptree_exception("BSP-Tree: key already exists") { }
    };
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BSP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BSP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BSP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_pairs(const BSP_tree::tree_data_type &lhs,
                                                      const BSP_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

// region bsptree_node_base implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_base::bsptree_node_base() noexcept
    : _is_terminated(false) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term::bsptree_node_term() noexcept
    : bsptree_node_base(), _next(nullptr), _data() { this->_is_terminated = true; }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle::bsptree_node_middle() noexcept
    : bsptree_node_base(), _keys(), _pointers() { }

// region BSP_tree constructor implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BSP_tree<tkey, tvalue, compare, t>::value_type> BSP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_node_term *node,
    size_t index)
    : _node(node), _index(index) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const compare& cmp, pp_allocator<value_type> alloc)
    : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(pp_allocator<value_type> alloc, const compare& cmp)
    : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
    : BSP_tree(cmp, alloc) 
{
    for (auto it = begin(); it != end(); ++it) insert(*it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
    : BSP_tree(cmp, alloc)
{
    for (const auto& item : data) insert(item);
}

// endregion BSP_tree constructor implementations

// region BSP_tree copy and move constructors

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const BSP_tree& other)
    : compare(other), _allocator(other._allocator.select_on_container_copy_construction()), _root(nullptr), _size(0)
{
    if (!other._root) return;

    std::vector<bsptree_node_base*> allocated_nodes;
    bsptree_node_term* prev_leaf = nullptr;

    auto copy_node = [&](auto& self, bsptree_node_base* other_node) -> bsptree_node_base*
    {
        if (!other_node) return nullptr;

        if (other_node->_is_terminated)
        {
            bsptree_node_term* old_term = static_cast<bsptree_node_term*>(other_node);

            bsptree_node_term* new_term;
            try { new_term = _allocator.template new_object<bsptree_node_term>(); }
            catch (...) { throw memory_allocation_exception(); }
            allocated_nodes.push_back(new_term);

            new_term->_data = old_term->_data;

            if (prev_leaf) prev_leaf->_next = new_term;
            prev_leaf = new_term;

            return new_term;
        }
        else
        {
            bsptree_node_middle* old_middle = static_cast<bsptree_node_middle*>(other_node);

            bsptree_node_middle* new_middle;
            try { new_middle = _allocator.template new_object<bsptree_node_middle>(); }
            catch (...) { throw memory_allocation_exception(); }
            allocated_nodes.push_back(new_middle);

            new_middle->_keys = old_middle->_keys;
            for (auto* child : old_middle->_pointers) new_middle->_pointers.push_back(self(self, child));
            return new_middle;
        }

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
            if ((*it)->_is_terminated) _allocator.template delete_object<bsptree_node_term>(static_cast<bsptree_node_term*>(*it));
            else _allocator.template delete_object<bsptree_node_middle>(static_cast<bsptree_node_middle*>(*it));
        }

        _root = nullptr;
        _size = 0;

        throw;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(BSP_tree&& other) noexcept
    : compare(std::move(static_cast<compare&>(other))), _allocator(std::move(other._allocator)), _root(other._root), _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

// endregion BSP_tree copy and move constructors

// region BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(const BSP_tree& other)
{
    if (this == &other) return *this;
    BSP_tree temp_tree(other);
    *this = std::move(temp_tree);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(BSP_tree&& other) noexcept
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

// endregion BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::~BSP_tree() noexcept { clear(); }

// region BSP_tree iterators implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::bsptree_iterator(bsptree_node_term* node, size_t index)
: _node(node), _index(index) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++()
{
    if (!_node) return *this;

    _index++;
    if (_index >= _node->_data.size())
    {
        _node = _node->_next;
        _index = 0;
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator==(const self& other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->_data.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_iterator& it) noexcept
: _node(it._node), _index(it._index) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++()
{
    if (!_node) return *this;

    ++_index;
    if (_index >= _node->_data.size())
    {
        _node = _node->_next;
        _index = 0;
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator==(const self& other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->_data.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::index() const noexcept
{
    return _index;
}

// endregion BSP_tree iterators implementations

// region BSP_tree element access implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) throw key_not_found_exception();
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) throw key_not_found_exception();
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    auto it = find(key);
    if (it != end()) return it->second;
    return emplace(key, tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    auto it = find(key);
    if (it != end()) return it->second;
    return emplace(std::move(key), tvalue()).first->second;
}

// endregion BSP_tree element access implementations

// region BSP_tree iterator begins implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::begin()
{
    if (!_root) return end();

    bsptree_node_base* current = _root;
    while (!current->_is_terminated) current = static_cast<bsptree_node_middle*>(current)->_pointers[0];

    return bsptree_iterator(static_cast<bsptree_node_term*>(current), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::end()
{
    return bsptree_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::begin() const
{
    if (!_root) return end();

    bsptree_node_base* current = _root;
    while (!current->_is_terminated) current = static_cast<bsptree_node_middle*>(current)->_pointers[0];

    return bsptree_const_iterator(static_cast<bsptree_node_term*>(current), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::end() const
{
    return bsptree_const_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

// endregion BSP_tree iterator begins implementations

// region BSP_tree lookup implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (!_root) return end();

    bsptree_node_base* current = _root;
    while (!current->_is_terminated)
    {
        auto middle = static_cast<bsptree_node_middle*>(current);
        auto it = std::upper_bound
        (
            middle->_keys.begin(),
            middle->_keys.end(),
            key, 
            [this](const tkey& k, const tkey& data_k) { return compare_keys(k, data_k); }
        );

        size_t idx = std::distance(middle->_keys.begin(), it);
        current = middle->_pointers[idx];
    }

    auto term = static_cast<bsptree_node_term*>(current);
    auto it = std::lower_bound
    (
        term->_data.begin(),
        term->_data.end(),
        key, 
        [this](const tree_data_type& data, const tkey& k) { return compare_keys(data.first, k); }
    );
    
    size_t idx = std::distance(term->_data.begin(), it);
    if (idx < term->_data.size() && !compare_keys(key, term->_data[idx].first)) return bsptree_iterator(term, idx);
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    auto it = const_cast<BSP_tree*>(this)->find(key);
    if (it == const_cast<BSP_tree*>(this)->end()) return end();
    return bsptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (!_root) return end();

    bsptree_node_base* current = _root;
    while (!current->_is_terminated)
    {
        auto middle = static_cast<bsptree_node_middle*>(current);
        auto it = std::upper_bound
        (
            middle->_keys.begin(),
            middle->_keys.end(),
            key, 
            [this](const tkey& k, const tkey& data_k) { return compare_keys(k, data_k); }
        );

        size_t idx = std::distance(middle->_keys.begin(), it);
        current = middle->_pointers[idx];
    }

    auto term = static_cast<bsptree_node_term*>(current);
    auto it = std::lower_bound
    (
        term->_data.begin(),
        term->_data.end(),
        key, 
        [this](const tree_data_type& data, const tkey& k) { return compare_keys(data.first, k); }
    );

    size_t idx = std::distance(term->_data.begin(), it);
    if (idx == term->_data.size())
    {
        if (term->_next) return bsptree_iterator(term->_next, 0);
        return end();
    }

    return bsptree_iterator(term, idx);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    auto it = const_cast<BSP_tree*>(this)->lower_bound(key);
    if (it == const_cast<BSP_tree*>(this)->end()) return end();
    return bsptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (!_root) return end();
    bsptree_node_base* current = _root;
    
    while (!current->_is_terminated)
    {
        auto middle = static_cast<bsptree_node_middle*>(current);
        auto it = std::upper_bound
        (
            middle->_keys.begin(),
            middle->_keys.end(),
            key, 
            [this](const tkey& k, const tkey& data_k) { return compare_keys(k, data_k); }
        );

        size_t idx = std::distance(middle->_keys.begin(), it);
        current = middle->_pointers[idx];
    }
    
    auto term = static_cast<bsptree_node_term*>(current);
    auto it = std::upper_bound
    (
        term->_data.begin(),
        term->_data.end(),
        key, 
        [this](const tkey& k, const tree_data_type& data) { return compare_keys(k, data.first); }
    );
        
    size_t idx = std::distance(term->_data.begin(), it);
    if (idx == term->_data.size()) 
    {
        if (term->_next) return bsptree_iterator(term->_next, 0);
        return end();
    }

    return bsptree_iterator(term, idx);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    auto it = const_cast<BSP_tree*>(this)->upper_bound(key);
    if (it == const_cast<BSP_tree*>(this)->end()) return end();
    return bsptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion BSP_tree lookup implementations

// region BSP_tree modifiers implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (!_root) return;

    try
    {
        std::vector<bsptree_node_base*> nodes_to_delete;
        nodes_to_delete.reserve(_size);
        nodes_to_delete.push_back(_root);

        while (!nodes_to_delete.empty())
        {
            bsptree_node_base* current = nodes_to_delete.back();
            nodes_to_delete.pop_back();

            if (current->_is_terminated)
            {
                bsptree_node_term* term = static_cast<bsptree_node_term*>(current);
                _allocator.template delete_object<bsptree_node_term>(term);
            }
            else
            {
                bsptree_node_middle* middle = static_cast<bsptree_node_middle*>(current);
                for (auto* child : middle->_pointers)
                    if (child) nodes_to_delete.push_back(child);

                _allocator.template delete_object<bsptree_node_middle>(middle);
            }
        }
    }
    catch (...) { }

    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename ...Args>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);

    if (!_root)
    {
        bsptree_node_term* new_root;
        try { new_root = _allocator.template new_object<bsptree_node_term>(); }
        catch (...) { throw memory_allocation_exception(); }
        
        new_root->_data.push_back(std::move(new_data));
        _root = new_root;
        _size++;

        return {bsptree_iterator(new_root, 0), true};
    }

    std::stack<std::pair<bsptree_node_middle*, size_t>> path;
    bsptree_node_base* current = _root;

    while (!current->_is_terminated)
    {
        auto middle = static_cast<bsptree_node_middle*>(current);
        auto it = std::upper_bound
        (
            middle->_keys.begin(),
            middle->_keys.end(),
            new_data.first,
            [this](const tkey& k, const tkey& data_k) { return compare_keys(k, data_k); }
        );
            
        size_t idx = std::distance(middle->_keys.begin(), it);
        path.push({middle, idx});
        current = middle->_pointers[idx];
    }

    bsptree_node_term* leaf = static_cast<bsptree_node_term*>(current);

    auto it = std::lower_bound
    (
        leaf->_data.begin(),
        leaf->_data.end(),
        new_data.first,
        [this](const tree_data_type& data, const tkey& k) { return compare_keys(data.first, k); }
    );
        
    size_t leaf_idx = std::distance(leaf->_data.begin(), it);
    if (leaf_idx < leaf->_data.size() && !compare_keys(new_data.first, leaf->_data[leaf_idx].first))
    {
        return {bsptree_iterator(leaf, leaf_idx), false};
    }

    leaf->_data.insert(leaf->_data.begin() + leaf_idx, std::move(new_data));
    _size++;

    bsptree_node_base* current_node = leaf;
    
    std::vector<tkey> keys_to_push_up;
    std::vector<bsptree_node_base*> nodes_to_push_up;
    size_t insert_idx = 0; 

    while (true)
    {
        bool is_term = current_node->_is_terminated;
        size_t current_size = is_term ? static_cast<bsptree_node_term*>(current_node)->_data.size() 
                                      : static_cast<bsptree_node_middle*>(current_node)->_keys.size();

        if (current_size <= maximum_keys_in_node) break; 

        if (path.empty())
        {
            bsptree_node_middle* new_root;
            try { new_root = _allocator.template new_object<bsptree_node_middle>(); }
            catch (...) { throw memory_allocation_exception(); }

            if (is_term)
            {
                auto old_term = static_cast<bsptree_node_term*>(current_node);
                bsptree_node_term* right_term;
                try { right_term = _allocator.template new_object<bsptree_node_term>(); }
                catch (...)
                { 
                    _allocator.template delete_object<bsptree_node_middle>(new_root);
                    throw memory_allocation_exception();
                }

                size_t mid = old_term->_data.size() / 2;
                
                if (leaf_idx > mid) mid++;

                right_term->_data.insert
                (
                    right_term->_data.begin(),
                    std::make_move_iterator(old_term->_data.begin() + mid),
                    std::make_move_iterator(old_term->_data.end())
                );
                old_term->_data.erase(old_term->_data.begin() + mid, old_term->_data.end());

                right_term->_next = old_term->_next;
                old_term->_next = right_term;

                new_root->_keys.push_back(right_term->_data[0].first); 
                new_root->_pointers.push_back(old_term);
                new_root->_pointers.push_back(right_term);
            }
            else
            {
                auto old_mid = static_cast<bsptree_node_middle*>(current_node);
                bsptree_node_middle* right_mid;
                try { right_mid = _allocator.template new_object<bsptree_node_middle>(); }
                catch (...) 
                { 
                    _allocator.template delete_object<bsptree_node_middle>(new_root);
                    throw memory_allocation_exception();
                }

                size_t mid = old_mid->_keys.size() / 2;
                
                if (insert_idx > mid) mid++;

                tkey median_key = std::move(old_mid->_keys[mid]); 

                right_mid->_keys.insert
                (
                    right_mid->_keys.begin(),
                    std::make_move_iterator(old_mid->_keys.begin() + mid + 1),
                    std::make_move_iterator(old_mid->_keys.end())
                );
                right_mid->_pointers.insert
                (
                    right_mid->_pointers.begin(),
                    old_mid->_pointers.begin() + mid + 1,
                    old_mid->_pointers.end()
                );

                old_mid->_keys.erase(old_mid->_keys.begin() + mid, old_mid->_keys.end());
                old_mid->_pointers.erase(old_mid->_pointers.begin() + mid + 1, old_mid->_pointers.end());

                new_root->_keys.push_back(std::move(median_key));
                new_root->_pointers.push_back(old_mid);
                new_root->_pointers.push_back(right_mid);
            }

            _root = new_root;
            break;
        }

        bsptree_node_middle* parent = path.top().first;
        size_t child_idx = path.top().second;
        path.pop();

        if (is_term) 
        {
            auto term = static_cast<bsptree_node_term*>(current_node);
            auto left_sibling = (child_idx > 0) ? static_cast<bsptree_node_term*>(parent->_pointers[child_idx - 1]) : nullptr;
            auto right_sibling = (child_idx < parent->_pointers.size() - 1) ? static_cast<bsptree_node_term*>(parent->_pointers[child_idx + 1]) : nullptr;

            if (right_sibling && right_sibling->_data.size() < maximum_keys_in_node)
            {
                right_sibling->_data.insert(right_sibling->_data.begin(), std::move(term->_data.back()));
                term->_data.pop_back();
                parent->_keys[child_idx] = right_sibling->_data.front().first; 
                break;
            }
            else if (left_sibling && left_sibling->_data.size() < maximum_keys_in_node) 
            {
                left_sibling->_data.push_back(std::move(term->_data.front()));
                term->_data.erase(term->_data.begin());
                parent->_keys[child_idx - 1] = term->_data.front().first; 
                break;
            }
            else
            {
                auto sibling = right_sibling ? right_sibling : left_sibling;
                bool is_right = (sibling == right_sibling);
                insert_idx = is_right ? child_idx : child_idx - 1; 
                
                auto left_node = is_right ? term : sibling;
                auto right_node = is_right ? sibling : term;

                std::vector<tree_data_type> temp_data;
                temp_data.reserve(left_node->_data.size() + right_node->_data.size());
                for (auto& d : left_node->_data) temp_data.push_back(std::move(d));
                for (auto& d : right_node->_data) temp_data.push_back(std::move(d));

                left_node->_data.clear();
                right_node->_data.clear();

                bsptree_node_term* middle_node;
                try { middle_node = _allocator.template new_object<bsptree_node_term>(); }
                catch (...) { throw memory_allocation_exception(); }

                size_t total = temp_data.size();
                size_t part1 = total / 3;
                size_t part2 = total / 3;
                size_t part3 = total - part1 - part2;

                size_t idx = 0;
                for (size_t i = 0; i < part1; ++i) left_node->_data.push_back(std::move(temp_data[idx++]));
                for (size_t i = 0; i < part2; ++i) middle_node->_data.push_back(std::move(temp_data[idx++]));
                for (size_t i = 0; i < part3; ++i) right_node->_data.push_back(std::move(temp_data[idx++]));

                middle_node->_next = right_node;
                left_node->_next = middle_node;

                keys_to_push_up = { middle_node->_data.front().first, right_node->_data.front().first };
                nodes_to_push_up = { middle_node, right_node };

                parent->_keys.erase(parent->_keys.begin() + insert_idx);
                parent->_pointers.erase(parent->_pointers.begin() + insert_idx + 1);
            }
        } 
        else
        {
            auto mid = static_cast<bsptree_node_middle*>(current_node);
            auto left_sibling = (child_idx > 0) ? static_cast<bsptree_node_middle*>(parent->_pointers[child_idx - 1]) : nullptr;
            auto right_sibling = (child_idx < parent->_pointers.size() - 1) ? static_cast<bsptree_node_middle*>(parent->_pointers[child_idx + 1]) : nullptr;

            if (right_sibling && right_sibling->_keys.size() < maximum_keys_in_node)
            {
                right_sibling->_keys.insert(right_sibling->_keys.begin(), std::move(parent->_keys[child_idx]));
                parent->_keys[child_idx] = std::move(mid->_keys.back());
                mid->_keys.pop_back();

                right_sibling->_pointers.insert(right_sibling->_pointers.begin(), mid->_pointers.back());
                mid->_pointers.pop_back();
                break;
            }
            else if (left_sibling && left_sibling->_keys.size() < maximum_keys_in_node)
            {
                left_sibling->_keys.push_back(std::move(parent->_keys[child_idx - 1]));
                parent->_keys[child_idx - 1] = std::move(mid->_keys.front());
                mid->_keys.erase(mid->_keys.begin());

                left_sibling->_pointers.push_back(mid->_pointers.front());
                mid->_pointers.erase(mid->_pointers.begin());
                break;
            }
            else
            {
                auto sibling = right_sibling ? right_sibling : left_sibling;
                bool is_right = (sibling == right_sibling);
                insert_idx = is_right ? child_idx : child_idx - 1; 
                
                auto left_node = is_right ? mid : sibling;
                auto right_node = is_right ? sibling : mid;

                std::vector<tkey> temp_keys;
                std::vector<bsptree_node_base*> temp_ptrs;
                
                for (auto& k : left_node->_keys) temp_keys.push_back(std::move(k));
                temp_keys.push_back(std::move(parent->_keys[insert_idx])); 
                for (auto& k : right_node->_keys) temp_keys.push_back(std::move(k));

                for (auto* p : left_node->_pointers) temp_ptrs.push_back(p);
                for (auto* p : right_node->_pointers) temp_ptrs.push_back(p);

                left_node->_keys.clear(); left_node->_pointers.clear();
                right_node->_keys.clear(); right_node->_pointers.clear();

                bsptree_node_middle* middle_node;
                try { middle_node = _allocator.template new_object<bsptree_node_middle>(); }
                catch (...) { throw memory_allocation_exception(); }

                size_t total_k = temp_keys.size();
                size_t keys_per_node = (total_k - 2) / 3;
                size_t left_k = keys_per_node;
                size_t middle_k = keys_per_node;
                size_t right_k = total_k - 2 - left_k - middle_k;

                size_t k_idx = 0; size_t p_idx = 0;

                for (size_t i = 0; i < left_k; ++i) left_node->_keys.push_back(std::move(temp_keys[k_idx++]));
                for (size_t i = 0; i <= left_k; ++i) left_node->_pointers.push_back(temp_ptrs[p_idx++]);

                tkey median1 = std::move(temp_keys[k_idx++]); 

                for (size_t i = 0; i < middle_k; ++i) middle_node->_keys.push_back(std::move(temp_keys[k_idx++]));
                for (size_t i = 0; i <= middle_k; ++i) middle_node->_pointers.push_back(temp_ptrs[p_idx++]);

                tkey median2 = std::move(temp_keys[k_idx++]); 

                for (size_t i = 0; i < right_k; ++i) right_node->_keys.push_back(std::move(temp_keys[k_idx++]));
                while (p_idx < temp_ptrs.size()) right_node->_pointers.push_back(temp_ptrs[p_idx++]);

                keys_to_push_up = { std::move(median1), std::move(median2) };
                nodes_to_push_up = { middle_node, right_node };

                parent->_keys.erase(parent->_keys.begin() + insert_idx);
                parent->_pointers.erase(parent->_pointers.begin() + insert_idx + 1);
            }
        }

        if (!keys_to_push_up.empty())
        {
            parent->_keys.insert
            (
                parent->_keys.begin() + insert_idx, 
                std::make_move_iterator(keys_to_push_up.begin()), 
                std::make_move_iterator(keys_to_push_up.end())
            );
            
            parent->_pointers.insert
            (
                parent->_pointers.begin() + insert_idx + 1, 
                nodes_to_push_up.begin(),
                nodes_to_push_up.end()
            );
            
            keys_to_push_up.clear();
            nodes_to_push_up.clear();
        }

        current_node = parent;
    }

    auto result_it = find(new_data.first);
    if (result_it == end())
    {
        auto it = begin();
        while (it != end()) ++it;
    }
    return {result_it, true};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    auto it = find(data.first);
    if (it != end()) { it->second = data.second; return it; }
    return emplace(data).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    auto it = find(data.first);
    if (it != end()) { it->second = std::move(data.second); return it; }
    return emplace(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename ...Args>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);
    return insert_or_assign(std::move(new_data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator pos)
{
    if (pos == end()) return end();

    bsptree_iterator next_it = pos;
    ++next_it;
    std::optional<tkey> next_key = std::nullopt;
    if (next_it != end()) next_key = next_it->first;

    tkey key_to_remove = pos->first;

    std::stack<std::pair<bsptree_node_middle*, size_t>> path;
    bsptree_node_base* current = _root;

    while (!current->_is_terminated)
    {
        auto middle = static_cast<bsptree_node_middle*>(current);
        auto it = std::upper_bound
        (
            middle->_keys.begin(),
            middle->_keys.end(),
            key_to_remove,
            [this](const tkey& k, const tkey& data_k) { return compare_keys(k, data_k); }
        );
            
        size_t idx = std::distance(middle->_keys.begin(), it);
        path.push({middle, idx});
        current = middle->_pointers[idx];
    }

    bsptree_node_term* leaf = static_cast<bsptree_node_term*>(current);

    auto it = std::lower_bound
    (
        leaf->_data.begin(),
        leaf->_data.end(),
        key_to_remove,
        [this](const tree_data_type& data, const tkey& k) { return compare_keys(data.first, k); }
    );
    
    size_t leaf_idx = std::distance(leaf->_data.begin(), it);
    if (leaf_idx >= leaf->_data.size() || compare_keys(key_to_remove, leaf->_data[leaf_idx].first))
    {
        return end(); 
    }

    leaf->_data.erase(leaf->_data.begin() + leaf_idx);
    _size--;

    if (leaf == _root)
    {
        if (leaf->_data.empty())
        {
            _allocator.template delete_object<bsptree_node_term>(leaf);
            _root = nullptr;
        }
        if (next_key.has_value()) return find(next_key.value());

        return end();
    }

    bsptree_node_base* current_node = leaf;

    while (!path.empty())
    {
        bool is_term = current_node->_is_terminated;
        size_t current_size = is_term ? static_cast<bsptree_node_term*>(current_node)->_data.size() 
                                      : static_cast<bsptree_node_middle*>(current_node)->_keys.size();

        if (current_size >= minimum_keys_in_node) break; 

        bsptree_node_middle* parent = path.top().first;
        size_t child_idx = path.top().second;
        path.pop();

        size_t start_idx = 0;
        size_t end_idx = 0;

        if (parent->_pointers.size() >= 3)
        {
            if (child_idx == 0) { start_idx = 0; end_idx = 2; } 
            else if (child_idx == parent->_pointers.size() - 1) { start_idx = child_idx - 2; end_idx = child_idx; } 
            else { start_idx = child_idx - 1; end_idx = child_idx + 1; }
        }
        else
        {
            start_idx = 0; end_idx = 1;
        }

        size_t num_nodes = end_idx - start_idx + 1;

        if (is_term)
        {
            std::vector<tree_data_type> temp_data;
            
            for (size_t i = start_idx; i <= end_idx; ++i)
            {
                auto n = static_cast<bsptree_node_term*>(parent->_pointers[i]);
                for (auto& d : n->_data) temp_data.push_back(std::move(d));
                n->_data.clear();
            }

            size_t target_nodes = num_nodes;
            if (temp_data.size() <= (num_nodes - 1) * maximum_keys_in_node)
            {
                target_nodes = num_nodes - 1; 
            }

            size_t base = temp_data.size() / target_nodes;
            size_t rem = temp_data.size() % target_nodes;
            size_t d_idx = 0;

            for (size_t i = 0; i < target_nodes; ++i)
            {
                auto n = static_cast<bsptree_node_term*>(parent->_pointers[start_idx + i]);
                size_t count = base + (i < rem ? 1 : 0);
                for (size_t j = 0; j < count; ++j) n->_data.push_back(std::move(temp_data[d_idx++]));
            }

            if (target_nodes < num_nodes)
            {
                auto last_kept = static_cast<bsptree_node_term*>(parent->_pointers[start_idx + target_nodes - 1]);
                auto node_to_delete = static_cast<bsptree_node_term*>(parent->_pointers[start_idx + target_nodes]);
                
                last_kept->_next = node_to_delete->_next;
                _allocator.template delete_object<bsptree_node_term>(node_to_delete);

                parent->_pointers.erase(parent->_pointers.begin() + start_idx + target_nodes);
                parent->_keys.erase(parent->_keys.begin() + start_idx + target_nodes - 1);
            }

            for (size_t i = 0; i < target_nodes - 1; ++i)
            {
                auto next_node = static_cast<bsptree_node_term*>(parent->_pointers[start_idx + i + 1]);
                parent->_keys[start_idx + i] = next_node->_data[0].first; 
            }
            
        }
        else
        {
            std::vector<tkey> temp_keys;
            std::vector<bsptree_node_base*> temp_ptrs;

            for (size_t i = start_idx; i <= end_idx; ++i)
            {
                auto n = static_cast<bsptree_node_middle*>(parent->_pointers[i]);
                for (auto& k : n->_keys) temp_keys.push_back(std::move(k));
                for (auto* p : n->_pointers) temp_ptrs.push_back(p);
                
                if (i < end_idx) temp_keys.push_back(std::move(parent->_keys[start_idx + i - start_idx]));
                
                n->_keys.clear(); n->_pointers.clear();
            }

            size_t target_nodes = num_nodes;
            size_t keys_for_nodes_if_merged = temp_keys.size() - ((num_nodes - 1) - 1); 
            if (keys_for_nodes_if_merged <= (num_nodes - 1) * maximum_keys_in_node) 
            {
                target_nodes = num_nodes - 1; 
            }

            size_t keys_for_nodes = temp_keys.size() - (target_nodes - 1);
            size_t base = keys_for_nodes / target_nodes;
            size_t rem = keys_for_nodes % target_nodes;
            size_t k_idx = 0, p_idx = 0;

            for (size_t i = 0; i < target_nodes; ++i)
            {
                auto n = static_cast<bsptree_node_middle*>(parent->_pointers[start_idx + i]);
                size_t count = base + (i < rem ? 1 : 0);
                
                for (size_t j = 0; j < count; ++j) n->_keys.push_back(std::move(temp_keys[k_idx++]));
                for (size_t j = 0; j <= count; ++j) n->_pointers.push_back(temp_ptrs[p_idx++]);
                
                if (i < target_nodes - 1) parent->_keys[start_idx + i] = std::move(temp_keys[k_idx++]);
            }

            if (target_nodes < num_nodes)
            {
                auto node_to_delete = static_cast<bsptree_node_middle*>(parent->_pointers[start_idx + target_nodes]);
                _allocator.template delete_object<bsptree_node_middle>(node_to_delete);
                parent->_pointers.erase(parent->_pointers.begin() + start_idx + target_nodes);
                parent->_keys.erase(parent->_keys.begin() + start_idx + target_nodes - 1);
            }
        }
        
        current_node = parent;
    }

    if (_root && !_root->_is_terminated)
    {
        auto root_mid = static_cast<bsptree_node_middle*>(_root);
        if (root_mid->_keys.empty())
        {
            _root = root_mid->_pointers[0];
            _allocator.template delete_object<bsptree_node_middle>(root_mid);
        }
    }

    if (next_key.has_value()) return find(next_key.value());
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator pos)
{
    return erase(find(pos->first));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator beg, bsptree_iterator en)
{
    std::vector<tkey> keys_to_delete;
    for (auto it = beg; it != en; ++it) keys_to_delete.push_back(it->first);
    for (const auto& k : keys_to_delete) erase(k);
    if (keys_to_delete.empty()) return en;
    return lower_bound(keys_to_delete.back());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator beg, bsptree_const_iterator en)
{
        auto non_const_beg = (beg != cend()) ? find(beg->first) : end();
    auto non_const_en = (en != cend()) ? find(en->first) : end();
    return erase(non_const_beg, non_const_en);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto it = find(key);
    if (it != end()) return erase(it);
    return end();
}

// endregion BSP_tree modifiers implementations


#endif