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

#ifndef SYS_PROG_B_PLUS_TREE_H
#define SYS_PROG_B_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BP_tree final : private compare //EBCO
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

    struct bptree_node_base
    {
        bool _is_terminate;

        bptree_node_base() noexcept;
        virtual ~bptree_node_base() =default;
    };

    struct bptree_node_term : public bptree_node_base
    {
        bptree_node_term* _next;

        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;
        bptree_node_term() noexcept;
    };

    struct bptree_node_middle : public bptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bptree_node_base*, maximum_keys_in_node + 2> _pointers;
        bptree_node_middle() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BP_tree(const BP_tree& other);

    BP_tree(BP_tree&& other) noexcept;

    BP_tree& operator=(const BP_tree& other);

    BP_tree& operator=(BP_tree&& other) noexcept;

    ~BP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bptree_iterator;
    class bptree_const_iterator;

    class bptree_iterator final
    {
        bptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_iterator;

        friend class BP_tree;
        friend class bptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_iterator(bptree_node_term* node = nullptr, size_t index = 0);

    };

    class bptree_const_iterator final
    {
        const bptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_const_iterator;

        friend class BP_tree;
        friend class bptree_iterator;

        bptree_const_iterator(const bptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_const_iterator(const bptree_node_term* node = nullptr, size_t index = 0);
    };

    friend class bptree_iterator;
    friend class bptree_const_iterator;

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

    bptree_iterator begin();
    bptree_iterator end();

    bptree_const_iterator begin() const;
    bptree_const_iterator end() const;

    bptree_const_iterator cbegin() const;
    bptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bptree_iterator find(const tkey& key);
    bptree_const_iterator find(const tkey& key) const;

    bptree_iterator lower_bound(const tkey& key);
    bptree_const_iterator lower_bound(const tkey& key) const;

    bptree_iterator upper_bound(const tkey& key);
    bptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bptree_iterator insert_or_assign(const tree_data_type& data);
    bptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bptree_iterator erase(bptree_iterator pos);
    bptree_iterator erase(bptree_const_iterator pos);

    bptree_iterator erase(bptree_iterator beg, bptree_iterator en);
    bptree_iterator erase(bptree_const_iterator beg, bptree_const_iterator en);


    bptree_iterator erase(const tkey& key);

    // endregion modifiers declaration

public:

    class bptree_exception : public std::exception
    {
    private:
        std::string _message;
    public:
        bptree_exception(std::string message): _message(std::move(message)) { }
        const char* what() const noexcept override { return _message.c_str(); } 
    };

    class key_not_found_exception final : public bptree_exception
    {
    public:
        explicit key_not_found_exception() : bptree_exception("BP-Tree: key not found") {}
    };

    class memory_allocation_exception final : public bptree_exception
    {
    public:
        explicit memory_allocation_exception() : bptree_exception("BP-Tree: failed to allocate memory") {}
    };

    class duplicate_key_exception final : public bptree_exception
    {
    public:
        explicit duplicate_key_exception() : bptree_exception("BP-Tree: key already exists") {}
    };
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_pairs(const BP_tree::tree_data_type &lhs,
                                                     const BP_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_base::bptree_node_base() noexcept
    : _is_terminate(false) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_term::bptree_node_term() noexcept
    : bptree_node_base(), _next(nullptr), _data() { this->_is_terminate = true; }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_middle::bptree_node_middle() noexcept
    : bptree_node_base(), _keys(), _pointers() { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BP_tree<tkey, tvalue, compare, t>::value_type> BP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>(_node->_data[_index]); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::pointer BP_tree<tkey, tvalue, compare, t>::bptree_iterator
::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self & BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++()
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
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++(int)
{
    self temp = *this;
    ++(*this);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator==(const self &other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->_data.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_iterator::bptree_iterator(bptree_node_term *node, size_t index)
    : _node(node), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_iterator &it) noexcept
    : _node(it._node), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>(_node->_data[_index]); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::pointer BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self & BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++()
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
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator==(const self &other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->_data.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_node_term *node, size_t index)
    : _node(node), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey & key)
{
    auto it = find(key);
    if (it == end()) throw key_not_found_exception();
    
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) throw key_not_found_exception();

    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](const tkey &key)
{
    auto it = find(key);
    if (it != end()) return it->second;

    return emplace(key, tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](tkey &&key)
{
    auto it = find(key);
    if (it != end()) return it->second;

    return emplace(std::move(key), tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(
    const tree_data_type &data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const compare& cmp, pp_allocator<value_type> alloc)
    : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) { }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(pp_allocator<value_type> alloc, const compare& cmp)
    : compare(cmp), _allocator(alloc), _root(nullptr), _size(0){ }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BP_tree<tkey, tvalue, compare, t>::BP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
    : BP_tree(cmp, alloc)
{
    for (auto it = begin; it != end; ++it) insert(*it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
    : BP_tree(cmp, alloc)
{
    for (const auto& item : data) insert(item);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const BP_tree& other)
    : compare(other), _allocator(other._allocator.select_on_container_copy_construction()), _root(nullptr), _size(0)
{
    if (!other._root) return;

    std::vector<bptree_node_base*> allocated_nodes;
    bptree_node_term* prev_leaf = nullptr;

    auto copy_node = [&](auto& self, bptree_node_base* other_node) -> bptree_node_base*
    {
        if (!other_node) return nullptr;

        if (other_node->_is_terminate)
        {
            bptree_node_term* old_term = static_cast<bptree_node_term*>(other_node);

            bptree_node_term* new_term;
            try { new_term = _allocator.template new_object<bptree_node_term>();}
            catch (...) { throw memory_allocation_exception(); }
            allocated_nodes.push_back(new_term);

            new_term->_data = old_term->_data;
            if (prev_leaf) prev_leaf->_next = new_term;
            prev_leaf = new_term;

            return new_term;
        }
        else
        {
            bptree_node_middle* old_middle = static_cast<bptree_node_middle*>(other_node);

            bptree_node_middle* new_middle;
            try { new_middle = _allocator.template new_object<bptree_node_middle>(); }
            catch (...) { throw memory_allocation_exception(); }
            allocated_nodes.push_back(new_middle);

            new_middle->_keys = old_middle->_keys;
            for (auto* child : old_middle->_pointers)
                new_middle->_pointers.push_back(self(self, child));
            
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
            if ((*it)->_is_terminate) _allocator.template delete_object<bptree_node_term>(static_cast<bptree_node_term*>(*it));
            else _allocator.template delete_object<bptree_node_middle>(static_cast<bptree_node_middle*>(*it));
        }

        _root = nullptr;
        _size = 0;
        throw;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(BP_tree&& other) noexcept
    : compare(std::move(static_cast<compare&>(other))), _allocator(std::move(other._allocator)), _root(other._root), _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(const BP_tree& other)
{
    if (this == &other) return *this;

    BP_tree temp(other);
    *this = std::move(temp);

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(BP_tree&& other) noexcept
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
BP_tree<tkey, tvalue, compare, t>::~BP_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::begin()
{
    if (!_root) return end();

    bptree_node_base* current = _root;
    while (!current->_is_terminate)
        current = static_cast<bptree_node_middle*>(current)->_pointers[0];

    return bptree_iterator(static_cast<bptree_node_term*>(current), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::end()
{
    return bptree_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::begin() const
{
    if (!_root) return end();

    bptree_node_base* current = _root;
    while (!current->_is_terminate) 
        current = static_cast<bptree_node_middle*>(current)->_pointers[0];

    return bptree_const_iterator(static_cast<bptree_node_term*>(current), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::end() const
{
    return bptree_const_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (!_root) return end();

    bptree_node_base* current = _root;
    while (!current->_is_terminate)
    {
        bptree_node_middle* middle = static_cast<bptree_node_middle*>(current);

        auto it = std::upper_bound
        (
            middle->_keys.begin(),
            middle->_keys.end(),
            key,
            [&](const tkey& k, const tkey& data_k) { return compare_keys(k, data_k); }
        );

        size_t idx = std::distance(middle->_keys.begin(), it);
        current = middle->_pointers[idx];
    }

    bptree_node_term* term = static_cast<bptree_node_term*>(current);
    auto it = std::lower_bound
    (
        term->_data.begin(),
        term->_data.end(),
        key,
        [&](const tree_data_type& data, const tkey& k) { return compare_keys(data.first, k); }
    );
    
    size_t idx = std::distance(term->_data.begin(), it);
    if (idx < term->_data.size() && !compare_keys(key, term->_data[idx].first))
        return bptree_iterator(term, idx);

    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    auto it = const_cast<BP_tree*>(this)->find(key);
    if (it == const_cast<BP_tree*>(this)->end()) return end();

    return bptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (!_root) return end();
    
    bptree_node_base* current = _root;
    
    while (!current->_is_terminate)
    {
        auto middle = static_cast<bptree_node_middle*>(current);
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
    
    auto term = static_cast<bptree_node_term*>(current);
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
        if (term->_next) return bptree_iterator(term->_next, 0);
        return end();
    }
    
    return bptree_iterator(term, idx);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    auto it = const_cast<BP_tree*>(this)->lower_bound(key);
    if (it == const_cast<BP_tree*>(this)->end()) return end();

    return bptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (!_root) return end();
    
    bptree_node_base* current = _root;
    
    while (!current->_is_terminate)
    {
        auto middle = static_cast<bptree_node_middle*>(current);
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
    
    auto term = static_cast<bptree_node_term*>(current);
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
        if (term->_next) return bptree_iterator(term->_next, 0);
        return end();
    }
    
    return bptree_iterator(term, idx);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    auto it = const_cast<BP_tree*>(this)->upper_bound(key);
    if (it == const_cast<BP_tree*>(this)->end()) return end();

    return bptree_const_iterator(it);   
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (!_root) return;

    try
    {
        std::vector<bptree_node_base*> nodes_to_delete;
        nodes_to_delete.reserve(_size);
        nodes_to_delete.push_back(_root);
        
        while(!nodes_to_delete.empty())
        {
            bptree_node_base* current = nodes_to_delete.back();
            nodes_to_delete.pop_back();

            if (!current->_is_terminate)
            {
                bptree_node_middle* middle_node = static_cast<bptree_node_middle*>(current);
                for (auto* child : middle_node->_pointers)
                    if (child) nodes_to_delete.push_back(child);

                _allocator.template delete_object<bptree_node_middle>(middle_node);
            }
            else
            {
                bptree_node_term* term_node = static_cast<bptree_node_term*>(current);
                _allocator.template delete_object<bptree_node_term>(term_node);
            }
        }
    }
    catch (...) { }

    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);

    if (!_root)
    {
        bptree_node_term* new_root;
        try { new_root = _allocator.template new_object<bptree_node_term>(); }
        catch (...) { throw memory_allocation_exception(); }

        new_root->_data.push_back(std::move(new_data));
        _root = new_root;
        ++_size;

        return {bptree_iterator(new_root, 0), true};
    }

    std::stack<std::pair<bptree_node_middle*, size_t>> path;
    bptree_node_base* current = _root;
    while (!current->_is_terminate)
    {
        bptree_node_middle* middle = static_cast<bptree_node_middle*>(current);
        auto it = std::upper_bound
        (   
            middle->_keys.begin(),
            middle->_keys.end(),
            new_data.first,
            [&](const tkey& k, const tkey& data_k) { return compare_keys(k, data_k); }
        );

        size_t idx = std::distance(middle->_keys.begin(), it);
        path.push({middle, idx});
        current = middle->_pointers[idx];
    }

    bptree_node_term* leaf = static_cast<bptree_node_term*>(current);
    auto it = std::lower_bound
    (
        leaf->_data.begin(),
        leaf->_data.end(),
        new_data.first,
        [&](const tree_data_type& data, const tkey& k) { return compare_keys(data.first, k); }
    );

    size_t leaf_idx = std::distance(leaf->_data.begin(), it);
    if (leaf_idx < leaf->_data.size() && !compare_keys(new_data.first, leaf->_data[leaf_idx].first))
        return {bptree_iterator(leaf, leaf_idx), false};

    leaf->_data.insert(leaf->_data.begin() + leaf_idx, std::move(new_data));
    _size++;

    bptree_node_base* current_split_node = leaf;
    tkey key_to_push_up;
    bptree_node_base* new_right_node = nullptr;

    if (leaf->_data.size() > maximum_keys_in_node)
    {
        bptree_node_term* right_leaf;
        try { right_leaf = _allocator.template new_object<bptree_node_term>(); }
        catch (...) { throw memory_allocation_exception(); }

        size_t median = t;

        right_leaf->_data.insert
        (
            right_leaf->_data.begin(),
            std::make_move_iterator(leaf->_data.begin() + median),
            std::make_move_iterator(leaf->_data.end())
        );
            
        leaf->_data.erase(leaf->_data.begin() + median, leaf->_data.end());

        right_leaf->_next = leaf->_next;
        leaf->_next = right_leaf;

        key_to_push_up = right_leaf->_data[0].first; 
        new_right_node = right_leaf;

    }

    while (new_right_node != nullptr)
    {
        if (path.empty())
        {
            bptree_node_middle* new_root;
            try { new_root = _allocator.template new_object<bptree_node_middle>(); }
            catch (...) { throw memory_allocation_exception(); }

            new_root->_keys.push_back(std::move(key_to_push_up));
            new_root->_pointers.push_back(_root);
            new_root->_pointers.push_back(new_right_node);
            _root = new_root;
            break;
        }
    
        bptree_node_middle* parent = path.top().first;
        size_t child_idx = path.top().second;
        path.pop();

        parent->_keys.insert(parent->_keys.begin() + child_idx, std::move(key_to_push_up));
        parent->_pointers.insert(parent->_pointers.begin() + child_idx + 1, new_right_node);

        if (parent->_keys.size() > maximum_keys_in_node)
        {
            bptree_node_middle* right_middle;
            try { right_middle = _allocator.template new_object<bptree_node_middle>(); }
            catch (...) { throw memory_allocation_exception(); }

            size_t median_idx = t; 

            key_to_push_up = std::move(parent->_keys[median_idx]);

            right_middle->_keys.insert
            (
                right_middle->_keys.begin(),
                std::make_move_iterator(parent->_keys.begin() + median_idx + 1),
                std::make_move_iterator(parent->_keys.end())
            );

            right_middle->_pointers.insert
            (
                right_middle->_pointers.begin(),
                parent->_pointers.begin() + median_idx + 1,
                parent->_pointers.end()
            );

            parent->_keys.erase(parent->_keys.begin() + median_idx, parent->_keys.end());
            parent->_pointers.erase(parent->_pointers.begin() + median_idx + 1, parent->_pointers.end());

            new_right_node = right_middle;
        }
        else 
        {
            new_right_node = nullptr;
        }
    }

    bptree_node_term* target_term = leaf;
    size_t target_idx = leaf_idx;
    if (leaf_idx >= t)
    {
        target_term = static_cast<bptree_node_term*>(new_right_node ? new_right_node : leaf->_next);
        target_idx = leaf_idx - t;
    }

    return {bptree_iterator(target_term, target_idx), true};    
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
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
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
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
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);
    return insert_or_assign(std::move(new_data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator pos)
{
    if (pos == end()) return end();

    bptree_iterator next_it = pos;
    ++next_it;
    std::optional<tkey> next_key = std::nullopt;
    if (next_it != end()) next_key = next_it->first;

    tkey key_to_remove = pos->first;

    std::stack<std::pair<bptree_node_middle*, size_t>> path;
    bptree_node_base* current = _root;

    while (!current->_is_terminate)
    {
        bptree_node_middle* middle = static_cast<bptree_node_middle*>(current);
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

    bptree_node_term* leaf = static_cast<bptree_node_term*>(current);

    auto it = std::lower_bound
    (
        leaf->_data.begin(),
        leaf->_data.end(),
        key_to_remove,
        [this](const tree_data_type& data, const tkey& k) { return compare_keys(data.first, k); }
    );
    
    size_t leaf_idx = std::distance(leaf->_data.begin(), it);
    if (leaf_idx >= leaf->_data.size() || compare_keys(key_to_remove, leaf->_data[leaf_idx].first)) 
        return end(); 
    
    leaf->_data.erase(leaf->_data.begin() + leaf_idx);
    _size--;

    if (leaf == _root)
    {
        if (leaf->_data.empty())
        {
            _allocator.template delete_object<bptree_node_term>(leaf);
            _root = nullptr;
        }
        if (next_key.has_value()) return find(next_key.value());

        return end();
    }

    current = leaf;
    while (!path.empty())
    {
        bptree_node_middle* parent = path.top().first;
        size_t child_idx = path.top().second;
        path.pop(); 

        if (current->_is_terminate)
        {
            bptree_node_term* term = static_cast<bptree_node_term*>(current);
            if (term->_data.size() >= minimum_keys_in_node) break; 

            bptree_node_term* left_sibling = (child_idx > 0) ? static_cast<bptree_node_term*>(parent->_pointers[child_idx - 1]) : nullptr;
            bptree_node_term* right_sibling = (child_idx < parent->_pointers.size() - 1) ? static_cast<bptree_node_term*>(parent->_pointers[child_idx + 1]) : nullptr;

            if (right_sibling && right_sibling->_data.size() > minimum_keys_in_node)
            {
                term->_data.push_back(std::move(right_sibling->_data.front()));
                right_sibling->_data.erase(right_sibling->_data.begin());
                parent->_keys[child_idx] = right_sibling->_data.front().first; 
                break;
            }
            else if (left_sibling && left_sibling->_data.size() > minimum_keys_in_node)
            {
                term->_data.insert(term->_data.begin(), std::move(left_sibling->_data.back()));
                left_sibling->_data.pop_back();
                parent->_keys[child_idx - 1] = term->_data.front().first; 
                break;
            }
            else
            { 
                if (right_sibling) 
                {
                    term->_data.insert
                    (
                        term->_data.end(),
                        std::make_move_iterator(right_sibling->_data.begin()),
                        std::make_move_iterator(right_sibling->_data.end())
                    );
                    
                    term->_next = right_sibling->_next;
                    
                    parent->_keys.erase(parent->_keys.begin() + child_idx);
                    parent->_pointers.erase(parent->_pointers.begin() + child_idx + 1);
                    _allocator.template delete_object<bptree_node_term>(right_sibling);
                    current = parent;
                }
                else if (left_sibling)
                {
                    left_sibling->_data.insert
                    (
                        left_sibling->_data.end(),
                        std::make_move_iterator(term->_data.begin()),
                        std::make_move_iterator(term->_data.end())
                    );
                    
                    left_sibling->_next = term->_next; 
                    
                    parent->_keys.erase(parent->_keys.begin() + child_idx - 1);
                    parent->_pointers.erase(parent->_pointers.begin() + child_idx);
                    _allocator.template delete_object<bptree_node_term>(term);
                    current = parent;
                }
            }   
        }
        else
        {
            bptree_node_middle* middle = static_cast<bptree_node_middle*>(current);
            if (middle->_keys.size() >= minimum_keys_in_node) break; 

            bptree_node_middle* left_sibling = (child_idx > 0) ? static_cast<bptree_node_middle*>(parent->_pointers[child_idx - 1]) : nullptr;
            bptree_node_middle* right_sibling = (child_idx < parent->_pointers.size() - 1) ? static_cast<bptree_node_middle*>(parent->_pointers[child_idx + 1]) : nullptr;

            if (right_sibling && right_sibling->_keys.size() > minimum_keys_in_node)
            {
                middle->_keys.push_back(std::move(parent->_keys[child_idx]));
                parent->_keys[child_idx] = std::move(right_sibling->_keys.front());
                right_sibling->_keys.erase(right_sibling->_keys.begin());

                middle->_pointers.push_back(right_sibling->_pointers.front());
                right_sibling->_pointers.erase(right_sibling->_pointers.begin());
                break;
            }
            else if (left_sibling && left_sibling->_keys.size() > minimum_keys_in_node)
            {
                middle->_keys.insert(middle->_keys.begin(), std::move(parent->_keys[child_idx - 1]));
                parent->_keys[child_idx - 1] = std::move(left_sibling->_keys.back());
                left_sibling->_keys.pop_back();

                middle->_pointers.insert(middle->_pointers.begin(), left_sibling->_pointers.back());
                left_sibling->_pointers.pop_back();
                break;
            }
            else
            {
                if (right_sibling)
                {
                    middle->_keys.push_back(std::move(parent->_keys[child_idx]));
                    middle->_keys.insert
                    (
                        middle->_keys.end(),
                        std::make_move_iterator(right_sibling->_keys.begin()),
                        std::make_move_iterator(right_sibling->_keys.end())
                    );
                    middle->_pointers.insert
                    (
                        middle->_pointers.end(),
                        right_sibling->_pointers.begin(),
                        right_sibling->_pointers.end()
                    );

                    parent->_keys.erase(parent->_keys.begin() + child_idx);
                    parent->_pointers.erase(parent->_pointers.begin() + child_idx + 1);
                    _allocator.template delete_object<bptree_node_middle>(right_sibling);
                    current = parent;
                }
                else if (left_sibling)
                {
                    left_sibling->_keys.push_back(std::move(parent->_keys[child_idx - 1]));
                    left_sibling->_keys.insert
                    (
                        left_sibling->_keys.end(),
                        std::make_move_iterator(middle->_keys.begin()),
                        std::make_move_iterator(middle->_keys.end())
                    );
                    left_sibling->_pointers.insert
                    (
                        left_sibling->_pointers.end(),
                        middle->_pointers.begin(),
                        middle->_pointers.end()
                    );

                    parent->_keys.erase(parent->_keys.begin() + child_idx - 1);
                    parent->_pointers.erase(parent->_pointers.begin() + child_idx);
                    _allocator.template delete_object<bptree_node_middle>(middle);
                    current = parent;
                }
            }
        }
    }

    if (_root && !_root->_is_terminate)
    {
        bptree_node_middle* root_middle = static_cast<bptree_node_middle*>(_root);
        if (root_middle->_keys.empty())
        {
            _root = root_middle->_pointers[0];
            _allocator.template delete_object<bptree_node_middle>(root_middle);
        }
    }

    if (next_key.has_value()) return find(next_key.value());
    return end();

}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator pos)
{
    return erase(find(pos->first));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator beg, bptree_iterator en)
{
    std::vector<tkey> keys_to_delete;
    for (auto it = beg; it != en; ++it) keys_to_delete.push_back(it->first);
    for (const auto& k : keys_to_delete) erase(k);

    if (keys_to_delete.empty()) return en;
    return lower_bound(keys_to_delete.back());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator beg, bptree_const_iterator en)
{
    auto non_const_beg = (beg != cend()) ? find(beg->first) : end();
    auto non_const_en = (en != cend()) ? find(en->first) : end();

    return erase(non_const_beg, non_const_en);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto it = find(key);
    if (it != end()) return erase(it);

    return end();
}

#endif