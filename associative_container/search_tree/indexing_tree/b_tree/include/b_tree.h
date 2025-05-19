// #include <iterator>
// #include <utility>
// #include <vector>
// #include <boost/container/static_vector.hpp>
// #include <concepts>
// #include <stack>
// #include <pp_allocator.h>
// #include <search_tree.h>
// #include <initializer_list>
// #include <logger_guardant.h>

// #ifndef MP_OS_B_TREE_H
// #define MP_OS_B_TREE_H

// template <typename tkey, typename tvalue, compator<tkey> compare = std::less<tkey>, std::size_t t = 5>
// class B_tree final : private logger_guardant, private compare
// {
// public:

//     using tree_data_type = std::pair<tkey, tvalue>;
//     using tree_data_type_const = std::pair<const tkey, tvalue>;
//     using value_type = tree_data_type_const;

// private:

//     static constexpr const size_t minimum_keys_in_node = t - 1;
//     static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

//     // region comparators declaration

//     inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
//     inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

//     // endregion comparators declaration


//     struct btree_node
//     {
//         boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
//         boost::container::static_vector<btree_node*, maximum_keys_in_node + 2> _pointers;
//         btree_node() noexcept;
// //        std::vector<tree_data_type, pp_allocator<tree_data_type>> _keys;
// //        std::vector<btree_node*, pp_allocator<btree_node*>> _pointers;

// //        btree_node(pp_allocator<value_type> al);
//     };

//     pp_allocator<value_type> _allocator;
//     logger* _logger;
//     btree_node* _root;
//     size_t _size;

//     logger* get_logger() const noexcept override;
//     pp_allocator<value_type> get_allocator() const noexcept;

// public:

// friend void swap(B_tree<tkey, tvalue, compare, t>& lhs, B_tree<tkey, tvalue, compare, t>& rhs) noexcept
// {
//     using std::swap;
//     swap(static_cast<compare&>(lhs), static_cast<compare&>(rhs));
//     swap(lhs._allocator, rhs._allocator);
//     swap(lhs._logger, rhs._logger);
//     swap(lhs._root, rhs._root);
//     swap(lhs._size, rhs._size);
// }

//     // region constructors declaration

//     explicit B_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>(), logger* logger = nullptr);

//     explicit B_tree(pp_allocator<value_type> alloc, const compare& comp = compare(), logger *logger = nullptr);

//     template<input_iterator_for_pair<tkey, tvalue> iterator>
//     explicit B_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>(), logger* logger = nullptr);

//     B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>(), logger* logger = nullptr);

//     // endregion constructors declaration

//     // region five declaration

//     B_tree(const B_tree& other);

//     B_tree(B_tree&& other) noexcept;

//     B_tree& operator=(const B_tree& other);

//     B_tree& operator=(B_tree&& other) noexcept;

//     ~B_tree() noexcept override;

//     // endregion five declaration

//     // region iterators declaration

//     class btree_iterator;
//     class btree_reverse_iterator;
//     class btree_const_iterator;
//     class btree_const_reverse_iterator;

//     class btree_iterator final
//     {
//         std::stack<std::pair<btree_node**, size_t>> _path;
//         size_t _index;

//     public:
//         using value_type = tree_data_type_const;
//         using reference = value_type&;
//         using pointer = value_type*;
//         using iterator_category = std::bidirectional_iterator_tag;
//         using difference_type = ptrdiff_t;
//         using self = btree_iterator;

//         friend class B_tree;
//         friend class btree_reverse_iterator;
//         friend class btree_const_iterator;
//         friend class btree_const_reverse_iterator;

//         reference operator*() const noexcept;
//         pointer operator->() const noexcept;

//         self& operator++();
//         self operator++(int);

//         self& operator--();
//         self operator--(int);

//         bool operator==(const self& other) const noexcept;
//         bool operator!=(const self& other) const noexcept;

//         size_t depth() const noexcept;
//         size_t current_node_keys_count() const noexcept;
//         bool is_terminate_node() const noexcept;
//         size_t index() const noexcept;

//         explicit btree_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);

//     };

//     class btree_const_iterator final
//     {
//         std::stack<std::pair<btree_node**, size_t>> _path;
//         size_t _index;

//     public:

//         using value_type = tree_data_type_const;
//         using reference = const value_type&;
//         using pointer = const value_type*;
//         using iterator_category = std::bidirectional_iterator_tag;
//         using difference_type = ptrdiff_t;
//         using self = btree_const_iterator;

//         friend class B_tree;
//         friend class btree_reverse_iterator;
//         friend class btree_iterator;
//         friend class btree_const_reverse_iterator;

//         btree_const_iterator(const btree_iterator& it) noexcept;

//         reference operator*() const noexcept;
//         pointer operator->() const noexcept;

//         self& operator++();
//         self operator++(int);

//         self& operator--();
//         self operator--(int);

//         bool operator==(const self& other) const noexcept;
//         bool operator!=(const self& other) const noexcept;

//         size_t depth() const noexcept;
//         size_t current_node_keys_count() const noexcept;
//         bool is_terminate_node() const noexcept;
//         size_t index() const noexcept;

//         explicit btree_const_iterator(const std::stack<std::pair<const btree_node**, size_t>>& path = std::stack<std::pair<const btree_node**, size_t>>(), size_t index = 0);
//     };

//     class btree_reverse_iterator final
//     {
//         std::stack<std::pair<btree_node**, size_t>> _path;
//         size_t _index;

//     public:

//         using value_type = tree_data_type_const;
//         using reference = value_type&;
//         using pointer = value_type*;
//         using iterator_category = std::bidirectional_iterator_tag;
//         using difference_type = ptrdiff_t;
//         using self = btree_reverse_iterator;

//         friend class B_tree;
//         friend class btree_iterator;
//         friend class btree_const_iterator;
//         friend class btree_const_reverse_iterator;

//         btree_reverse_iterator(const btree_iterator& it) noexcept;
//         operator btree_iterator() const noexcept;

//         reference operator*() const noexcept;
//         pointer operator->() const noexcept;

//         self& operator++();
//         self operator++(int);

//         self& operator--();
//         self operator--(int);

//         bool operator==(const self& other) const noexcept;
//         bool operator!=(const self& other) const noexcept;

//         size_t depth() const noexcept;
//         size_t current_node_keys_count() const noexcept;
//         bool is_terminate_node() const noexcept;
//         size_t index() const noexcept;

//         explicit btree_reverse_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);
//     };

//     class btree_const_reverse_iterator final
//     {
//         std::stack<std::pair<btree_node**, size_t>> _path;
//         size_t _index;

//     public:

//         using value_type = tree_data_type_const;
//         using reference = const value_type&;
//         using pointer = const value_type*;
//         using iterator_category = std::bidirectional_iterator_tag;
//         using difference_type = ptrdiff_t;
//         using self = btree_const_reverse_iterator;

//         friend class B_tree;
//         friend class btree_reverse_iterator;
//         friend class btree_const_iterator;
//         friend class btree_iterator;

//         btree_const_reverse_iterator(const btree_reverse_iterator& it) noexcept;
//         operator btree_const_iterator() const noexcept;

//         reference operator*() const noexcept;
//         pointer operator->() const noexcept;

//         self& operator++();
//         self operator++(int);

//         self& operator--();
//         self operator--(int);

//         bool operator==(const self& other) const noexcept;
//         bool operator!=(const self& other) const noexcept;

//         size_t depth() const noexcept;
//         size_t current_node_keys_count() const noexcept;
//         bool is_terminate_node() const noexcept;
//         size_t index() const noexcept;

//         explicit btree_const_reverse_iterator(const std::stack<std::pair<const btree_node**, size_t>>& path = std::stack<std::pair<const btree_node**, size_t>>(), size_t index = 0);
//     };

//     friend class btree_iterator;
//     friend class btree_const_iterator;
//     friend class btree_reverse_iterator;
//     friend class btree_const_reverse_iterator;

//     // endregion iterators declaration

//     // region element access declaration

//     /*
//      * Returns a reference to the mapped value of the element with specified key. If no such element exists, an exception of type std::out_of_range is thrown.
//      */
//     tvalue& at(const tkey&);
//     const tvalue& at(const tkey&) const;

//     /*
//      * If key not exists, makes default initialization of value
//      */
//     tvalue& operator[](const tkey& key);
//     tvalue& operator[](tkey&& key);

//     // endregion element access declaration
//     // region iterator begins declaration

//     btree_iterator begin();
//     btree_iterator end();

//     btree_const_iterator begin() const;
//     btree_const_iterator end() const;

//     btree_const_iterator cbegin() const;
//     btree_const_iterator cend() const;

//     btree_reverse_iterator rbegin();
//     btree_reverse_iterator rend();

//     btree_const_reverse_iterator rbegin() const;
//     btree_const_reverse_iterator rend() const;

//     btree_const_reverse_iterator crbegin() const;
//     btree_const_reverse_iterator crend() const;

//     // endregion iterator begins declaration

//     // region lookup declaration

//     size_t size() const noexcept;
//     bool empty() const noexcept;

//     /*
//      * Returns end() if not exist
//      */

//     btree_iterator find(const tkey& key);
//     btree_const_iterator find(const tkey& key) const;

//     btree_iterator lower_bound(const tkey& key);
//     btree_const_iterator lower_bound(const tkey& key) const;

//     btree_iterator upper_bound(const tkey& key);
//     btree_const_iterator upper_bound(const tkey& key) const;

//     bool contains(const tkey& key) const;

//     // endregion lookup declaration

//     // region modifiers declaration

//     void clear() noexcept;

//     /*
//      * Does nothing if key exists, delegates to emplace.
//      * Second return value is true, when inserted
//      */
//     std::pair<btree_iterator, bool> insert(const tree_data_type& data);
//     std::pair<btree_iterator, bool> insert(tree_data_type&& data);

//     template <typename ...Args>
//     std::pair<btree_iterator, bool> emplace(Args&&... args);

//     /*
//      * Updates value if key exists, delegates to emplace.
//      */
//     btree_iterator insert_or_assign(const tree_data_type& data);
//     btree_iterator insert_or_assign(tree_data_type&& data);

//     template <typename ...Args>
//     btree_iterator emplace_or_assign(Args&&... args);

//     /*
//      * Return iterator to node next ro removed or end() if key not exists
//      */
//     btree_iterator erase(btree_iterator pos);
//     btree_iterator erase(btree_const_iterator pos);

//     btree_iterator erase(btree_iterator beg, btree_iterator en);
//     btree_iterator erase(btree_const_iterator beg, btree_const_iterator en);


//     btree_iterator erase(const tkey& key);

//     // endregion modifiers declaration

//     private:
//     void balance_after_erase(
//         btree_node* node, 
//         std::stack<std::pair<btree_node**, size_t>>& path
//     );
    
//     void borrow_from_left(
//         btree_node* parent, 
//         size_t index
//     );

//     void borrow_from_right(
//         btree_node* parent, 
//         size_t index
//     );
    
//     void merge_nodes(
//         btree_node* parent, 
//         size_t index
//     );

//     void split_child(btree_node* parent, size_t index);

// };

// template<std::input_iterator iterator, compator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
//         std::size_t t = 5, typename U>
// B_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>(),
//        logger *logger = nullptr) -> B_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

// template<typename tkey, typename tvalue, compator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
// B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>(),
//        logger *logger = nullptr) -> B_tree<tkey, tvalue, compare, t>;

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::compare_pairs(const B_tree::tree_data_type &lhs,
//                                                      const B_tree::tree_data_type &rhs) const
// {
//     return compare_keys(lhs.first, rhs.first);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
// {
//     return compare::operator()(lhs, rhs);
// }


// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_node::btree_node() noexcept = default;

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// logger* B_tree<tkey, tvalue, compare, t>::get_logger() const noexcept
// {
//     return _logger;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// pp_allocator<typename B_tree<tkey, tvalue, compare, t>::value_type> B_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept
// {
//     return _allocator;
// }

// // region constructors implementation

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::B_tree(
//         const compare& cmp,
//         pp_allocator<value_type> alloc,
//         logger* logger)
//         : compare(cmp),
//       _allocator(alloc),
//       _logger(logger),
//       _root(nullptr),
//       _size(0)
// {}

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::B_tree(
//         pp_allocator<value_type> alloc,
//         const compare& comp,
//         logger* logger)
//         : compare(comp),
//       _allocator(alloc),
//       _logger(logger),
//       _root(nullptr),
//       _size(0)
// {}

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// template<input_iterator_for_pair<tkey, tvalue> iterator>
// B_tree<tkey, tvalue, compare, t>::B_tree(
//         iterator begin,
//         iterator end,
//         const compare& cmp,
//         pp_allocator<value_type> alloc,
//         logger* logger)
//         : B_tree(cmp, alloc, logger)
// {
//     for (auto it = begin; it != end; ++it)
//     {
//         insert(*it);
//     }
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::B_tree(
//         std::initializer_list<std::pair<tkey, tvalue>> data,
//         const compare& cmp,
//         pp_allocator<value_type> alloc,
//         logger* logger)
//         : B_tree(data.begin(), data.end(), cmp, alloc, logger)
// {}

// // endregion constructors implementation

// // region five implementation

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::~B_tree() noexcept
// {
//     auto delete_subtree = [this](auto& self, btree_node* node) -> void {
//         if (!node) return;
//         for (auto* child : node->_pointers) {
//             self(self, child);
//         }
//         _allocator.destroy(node);
//         _allocator.deallocate(node, 1);
//     };
    
//     delete_subtree(delete_subtree, _root);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::B_tree(const B_tree& other)
// : compare(static_cast<const compare&>(other)),
//       _allocator(other._allocator),
//       _logger(other._logger),
//       _size(other._size)
// {
//     auto copy_subtree = [this](auto& self, const btree_node* src) -> btree_node* {
//         if (!src) return nullptr;
        
//         btree_node* dst = _allocator.allocate(1);
//         _allocator.construct(dst, btree_node());
//         dst->_keys = src->_keys;
        
//         for (const auto* child : src->_pointers) {
//             dst->_pointers.push_back(self(self, child));
//         }
//         return dst;
//     };
    
//     _root = copy_subtree(copy_subtree, other._root);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(const B_tree& other)
// {
//     if (this != &other) {
//         B_tree temp(other);
//         swap(*this, temp);
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::B_tree(B_tree&& other) noexcept
// : compare(std::move(other)),
//       _allocator(std::move(other._allocator)),
//       _logger(std::exchange(other._logger, nullptr)),
//       _root(std::exchange(other._root, nullptr)),
//       _size(std::exchange(other._size, 0))
// {}

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(B_tree&& other) noexcept
// {
//     if (this != &other) {
//         this->~B_tree();
//         _allocator = std::move(other._allocator);
//         _logger = std::exchange(other._logger, nullptr);
//         _root = std::exchange(other._root, nullptr);
//         _size = std::exchange(other._size, 0);
//     }
//     return *this;
// }

// // endregion five implementation

// // region iterators implementation

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_iterator::btree_iterator(
//         const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)
//         : _path(path), _index(index) {}

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator::reference
// B_tree<tkey, tvalue, compare, t>::btree_iterator::operator*() const noexcept
// {
//     return (*_path.top().first)->_keys[_index];
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator::pointer
// B_tree<tkey, tvalue, compare, t>::btree_iterator::operator->() const noexcept
// {
//     return &(**this);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
// B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++()
// {
//     auto* node = *_path.top().first;
//     if (!node->_pointers.empty()) {
//         _path.emplace(&node->_pointers[_index + 1], 0);
//         node = node->_pointers[_index + 1];
//         while (!node->_pointers.empty()) {
//             _path.emplace(&node->_pointers[0], 0);
//             node = node->_pointers[0];
//         }
//         _index = 0;
//     } else if (++_index >= node->_keys.size()) {
//         while (!_path.empty()) {
//             auto idx = _path.top().second;
//             _path.pop();
//             if (!_path.empty() && ++_path.top().second <= (*_path.top().first)->_pointers.size() - 1) {
//                 _index = _path.top().second - 1;
//                 break;
//             }
//         }
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++(int)
// {
//     auto temp = *this;
//     ++*this;
//     return temp;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
// B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--()
// {
//     if (_path.empty()) return *this;
    
//     auto* node = *_path.top().first;
//     if (!node->_pointers.empty()) {
//         _path.emplace(&node->_pointers[_index], node->_pointers[_index]->_keys.size() - 1);
//         _index = node->_pointers[_index]->_keys.size() - 1;
//     } else if (_index-- == 0) {
//         while (!_path.empty()) {
//             auto idx = _path.top().second;
//             _path.pop();
//             if (!_path.empty() && _path.top().second-- > 0) {
//                 _index = (*_path.top().first)->_keys.size() - 1;
//                 break;
//             }
//         }
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--(int)
// {
//     auto temp = *this;
//     --*this;
//     return temp;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator==(const self& other) const noexcept
// {
//     return _path == other._path && _index == other._index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator!=(const self& other) const noexcept
// {
//     return !(*this == other);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::depth() const noexcept
// {
//     return _path.size();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::current_node_keys_count() const noexcept
// {
//     return (*_path.top().first)->_keys.size();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_iterator::is_terminate_node() const noexcept
// {
//     return (*_path.top().first)->_pointers.empty();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::index() const noexcept
// {
//     return _index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
//         const std::stack<std::pair<const btree_node**, size_t>>& path, size_t index)
// {
//     while (!path.empty()) {
//         _path.emplace(const_cast<btree_node**>(path.top().first), path.top().second);
//         path.pop();
//     }
//     _index = index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
//         const btree_iterator& it) noexcept
//         : _path(it._path), _index(it._index)
// {}

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::reference
// B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator*() const noexcept
// {
//     return const_cast<reference>((*_path.top().first)->_keys[_index]);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::pointer
// B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator->() const noexcept
// {
//     return &(**this);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
// B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++()
// {
//     auto* node = *_path.top().first;
//     if (!node->_pointers.empty()) {
//         _path.emplace(&node->_pointers[_index + 1], 0);
//         node = node->_pointers[_index + 1];
//         while (!node->_pointers.empty()) {
//             _path.emplace(&node->_pointers[0], 0);
//             node = node->_pointers[0];
//         }
//         _index = 0;
//     } else if (++_index >= node->_keys.size()) {
//         while (!_path.empty()) {
//             auto idx = _path.top().second;
//             _path.pop();
//             if (!_path.empty() && ++_path.top().second <= (*_path.top().first)->_pointers.size() - 1) {
//                 _index = _path.top().second - 1;
//                 break;
//             }
//         }
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
// B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++(int)
// {
//     auto temp = *this;
//     ++*this;
//     return temp;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
// B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--()
// {
//     if (_path.empty()) return *this;
    
//     auto* node = *_path.top().first;
//     if (!node->_pointers.empty()) {
//         _path.emplace(&node->_pointers[_index], node->_pointers[_index]->_keys.size() - 1);
//         _index = node->_pointers[_index]->_keys.size() - 1;
//     } else if (_index-- == 0) {
//         while (!_path.empty()) {
//             auto idx = _path.top().second;
//             _path.pop();
//             if (!_path.empty() && _path.top().second-- > 0) {
//                 _index = (*_path.top().first)->_keys.size() - 1;
//                 break;
//             }
//         }
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
// B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int)
// {
//     auto temp = *this;
//     --*this;
//     return temp;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator==(const self& other) const noexcept
// {
//     return _path == other._path && _index == other._index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator!=(const self& other) const noexcept
// {
//     return !(*this == other);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::depth() const noexcept
// {
//     return _path.size();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::current_node_keys_count() const noexcept
// {
//     return (*_path.top().first)->_keys.size();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::is_terminate_node() const noexcept
// {
//     return (*_path.top().first)->_pointers.empty();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::index() const noexcept
// {
//     return _index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
//         const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)
//         : _path(path), _index(index) {}

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
//         const btree_iterator& it) noexcept
//         : _path(it._path), _index(it._index) {}

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_iterator() const noexcept
// {
//     return btree_iterator(_path, _index);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::reference
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator*() const noexcept
// {
//     return (*_path.top().first)->_keys[_index];
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::pointer
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->() const noexcept
// {
//     return &(**this);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++()
// {
//     if (_path.empty()) return *this;
    
//     auto* node = *_path.top().first;
//     if (!node->_pointers.empty()) {
//         _path.emplace(&node->_pointers[_index], node->_pointers[_index]->_keys.size() - 1);
//         _index = node->_pointers[_index]->_keys.size() - 1;
//     } else if (_index-- == 0) {
//         while (!_path.empty()) {
//             auto idx = _path.top().second;
//             _path.pop();
//             if (!_path.empty() && _path.top().second-- > 0) {
//                 _index = (*_path.top().first)->_keys.size() - 1;
//                 break;
//             }
//         }
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++(int)
// {
//     auto temp = *this;
//     ++*this;
//     return temp;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--()
// {
//     auto* node = *_path.top().first;
//     if (!node->_pointers.empty()) {
//         _path.emplace(&node->_pointers[_index + 1], 0);
//         node = node->_pointers[_index + 1];
//         while (!node->_pointers.empty()) {
//             _path.emplace(&node->_pointers[0], 0);
//             node = node->_pointers[0];
//         }
//         _index = 0;
//     } else if (++_index >= node->_keys.size()) {
//         while (!_path.empty()) {
//             auto idx = _path.top().second;
//             _path.pop();
//             if (!_path.empty() && ++_path.top().second <= (*_path.top().first)->_pointers.size() - 1) {
//                 _index = _path.top().second - 1;
//                 break;
//             }
//         }
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
// B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--(int)
// {
//     auto temp = *this;
//     --*this;
//     return temp;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator==(const self& other) const noexcept
// {
//     return _path == other._path && _index == other._index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator!=(const self& other) const noexcept
// {
//     return !(*this == other);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::depth() const noexcept
// {
//     return _path.size();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::current_node_keys_count() const noexcept
// {
//     return (*_path.top().first)->_keys.size();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::is_terminate_node() const noexcept
// {
//     return (*_path.top().first)->_pointers.empty();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::index() const noexcept
// {
//     return _index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
//         const std::stack<std::pair<const btree_node**, size_t>>& path, size_t index)
// {
//     std::stack<std::pair<btree_node**, size_t>> temp_path;
//     auto s = path;
//     while (!s.empty()) {
//         auto [ptr, idx] = s.top();
//         temp_path.emplace(const_cast<btree_node**>(ptr), idx);
//         s.pop();
//     }
//     _path = temp_path;
//     _index = index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
//         const btree_reverse_iterator& it) noexcept
//         : _path(it._path), _index(it._index)
// {}

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_const_iterator() const noexcept
// {
//     return btree_const_iterator(_path, _index);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::reference
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator*() const noexcept
// {
//     return (*_path.top().first)->_keys[_index];
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::pointer
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator->() const noexcept
// {
//     return &(**this);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++()
// {
//     if (_path.empty()) return *this;
    
//     auto* node = *_path.top().first;
//     if (!node->_pointers.empty()) {
//         _path.emplace(&node->_pointers[_index], node->_pointers[_index]->_keys.size() - 1);
//         _index = node->_pointers[_index]->_keys.size() - 1;
//     } else if (_index-- == 0) {
//         while (!_path.empty()) {
//             auto idx = _path.top().second;
//             _path.pop();
//             if (!_path.empty() && _path.top().second-- > 0) {
//                 _index = (*_path.top().first)->_keys.size() - 1;
//                 break;
//             }
//         }
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++(int)
// {
//     auto temp = *this;
//     ++*this;
//     return temp;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--()
// {
//     auto* node = *_path.top().first;
//     if (!node->_pointers.empty()) {
//         _path.emplace(&node->_pointers[_index + 1], 0);
//         node = node->_pointers[_index + 1];
//         while (!node->_pointers.empty()) {
//             _path.emplace(&node->_pointers[0], 0);
//             node = node->_pointers[0];
//         }
//         _index = 0;
//     } else if (++_index >= node->_keys.size()) {
//         while (!_path.empty()) {
//             auto idx = _path.top().second;
//             _path.pop();
//             if (!_path.empty() && ++_path.top().second <= (*_path.top().first)->_pointers.size() - 1) {
//                 _index = _path.top().second - 1;
//                 break;
//             }
//         }
//     }
//     return *this;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
// B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--(int)
// {
//     auto temp = *this;
//     --*this;
//     return temp;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator==(const self& other) const noexcept
// {
//     return _path == other._path && _index == other._index;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator!=(const self& other) const noexcept
// {
//     return !(*this == other);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::depth() const noexcept
// {
//     return _path.size();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::current_node_keys_count() const noexcept
// {
//     return (*_path.top().first)->_keys.size();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::is_terminate_node() const noexcept
// {
//     return (*_path.top().first)->_pointers.empty();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::index() const noexcept
// {
//     return _index;
// }

// // endregion iterators implementation

// // region element access implementation

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key)
// {
//     auto it = find(key);
//     if (it == end()) {
//         throw std::out_of_range("Key not found in B_tree::at");
//     }
//     return it->second;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// const tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
// {
//     auto it = find(key);
//     if (it == end()) {
//         throw std::out_of_range("Key not found in B_tree::at");
//     }
//     return it->second;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// tvalue& B_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
// {
//     auto [iterator, inserted] = insert(std::make_pair(key, tvalue()));
//     return iterator->second;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// tvalue& B_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
// {
//     auto [iterator, inserted] = insert(std::make_pair(std::move(key), tvalue()));
//     return iterator->second;
// }

// // endregion element access implementation

// // region iterator begins implementation

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::begin()
// {
//     if (!_root) return end();

//     std::stack<std::pair<btree_node**, size_t>> path;
//     btree_node* current = _root;
    
//     while (!current->_pointers.empty()) {
//         path.emplace(&current->_pointers.front(), 0);
//         current = current->_pointers.front();
//     }
    
//     return btree_iterator(path, 0);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::end()
// {
//     return btree_iterator(std::stack<std::pair<btree_node**, size_t>>(), 0);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::begin() const
// {
//     return cbegin();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::end() const
// {
//     return cend();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cbegin() const
// {
//     if (!_root) return cend();

//     std::stack<std::pair<btree_node**, size_t>> path;
//     const btree_node* current = _root;
    
//     while (!current->_pointers.empty()) {
//         path.emplace(const_cast<btree_node**>(&current->_pointers.front()), 0);
//         current = current->_pointers.front();
//     }
    
//     return btree_const_iterator(path, 0);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cend() const
// {
//     return btree_const_iterator(std::stack<std::pair<btree_node**, size_t>>(), 0);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin()
// {
//     return btree_reverse_iterator(end());
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend()
// {
//     return btree_reverse_iterator(begin());
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin() const
// {
//     return btree_const_reverse_iterator(cend());
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend() const
// {
//     return btree_const_reverse_iterator(cbegin());
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crbegin() const
// {
//     return rbegin();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crend() const
// {
//     return rend();
// }

// // endregion iterator begins implementation

// // region lookup implementation

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// size_t B_tree<tkey, tvalue, compare, t>::size() const noexcept
// {
//     return _size;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::empty() const noexcept
// {
//     return _size == 0;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key)
// {
//     std::stack<std::pair<btree_node**, size_t>> path;
//     btree_node* current = _root;
    
//     while (current) {
//         size_t i = 0;
//         while (i < current->_keys.size() && compare::operator()(current->_keys[i].first, key)) {
//             ++i;
//         }
        
//         if (i < current->_keys.size() && !compare::operator()(key, current->_keys[i].first)) {
//             path.emplace(&current->_pointers[i], i);
//             return btree_iterator(path, i);
//         }
        
//         if (current->_pointers.empty()) break;
//         path.emplace(&current->_pointers[i], i);
//         current = current->_pointers[i];
//     }
    
//     return end();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
// {
//     std::stack<std::pair<const btree_node**, size_t>> path;
//     const btree_node* current = _root;
    
//     while (current) {
//         size_t i = 0;
//         while (i < current->_keys.size() && compare::operator()(current->_keys[i].first, key)) {
//             ++i;
//         }
        
//         if (i < current->_keys.size() && !compare::operator()(key, current->_keys[i].first)) {
//             return btree_const_iterator(path, i);
//         }
        
//         if (current->_pointers.empty()) break;
//         path.emplace(&current->_pointers[i], i);
//         current = current->_pointers[i];
//     }
    
//     return cend();
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
// {
//     std::stack<std::pair<btree_node**, size_t>> path;
//     btree_node* current = _root;
//     btree_iterator result = end();
    
//     while (current) {
//         size_t i = 0;
//         while (i < current->_keys.size() && compare::operator()(current->_keys[i].first, key)) {
//             ++i;
//         }
        
//         if (i < current->_keys.size() && !compare::operator()(key, current->_keys[i].first)) {
//             path.emplace(&current->_pointers[i], i);
//             return btree_iterator(path, i);
//         }
        
//         if (i > 0) result = btree_iterator(path, i-1);
//         if (current->_pointers.empty()) {
//             if (i < current->_keys.size()) result = btree_iterator(path, i);
//             break;
//         }
        
//         path.emplace(&current->_pointers[i], i);
//         current = current->_pointers[i];
//     }
    
//     return result;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
// {
//     const btree_node* current = _root;
//     std::stack<std::pair<const btree_node**, size_t>> path;
//     btree_const_iterator result = cend();
    
//     while (current) {
//         size_t i = 0;
//         while (i < current->_keys.size() && compare::operator()(current->_keys[i].first, key)) {
//             ++i;
//         }
        
//         if (i < current->_keys.size() && !compare::operator()(key, current->_keys[i].first)) {
//             return btree_const_iterator(path, i);
//         }
        
//         if (i > 0) result = btree_const_iterator(path, i-1);
//         if (current->_pointers.empty()) {
//             if (i < current->_keys.size()) result = btree_const_iterator(path, i);
//             break;
//         }
        
//         path.emplace(&current->_pointers[i], i);
//         current = current->_pointers[i];
//     }
//     return result;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
// {
//     auto it = lower_bound(key);
//     if (it != end() && !compare::operator()(key, it->first)) {
//         ++it;
//     }
//     return it;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
// {
//     auto it = lower_bound(key);
//     if (it != cend() && !compare::operator()(key, it->first)) {
//         ++it;
//     }
//     return it;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// bool B_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
// {
//     return find(key) != cend();
// }

// // endregion lookup implementation

// // region modifiers implementation

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// void B_tree<tkey, tvalue, compare, t>::clear() noexcept
// {
//     auto delete_subtree = [this](auto& self, btree_node* node) -> void {
//         if (!node) return;
//         for (auto* child : node->_pointers) {
//             self(self, child);
//         }
//         _allocator.destroy(node);
//         _allocator.deallocate(node, 1);
//     };
    
//     delete_subtree(delete_subtree, _root);
//     _root = nullptr;
//     _size = 0;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
// B_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
// {
//     auto it = find(data.first);
//     if (it != end()) {
//         return {it, false};
//     }

//     if (!_root) {
//         _root = _allocator.allocate(1);
//         _allocator.construct(_root, btree_node());
//         _root->_keys.push_back(data);
//         _size = 1;
//         return {btree_iterator({}, 0), true};
//     }

//     if (_root->_keys.size() == maximum_keys_in_node) {
//         btree_node* new_root = _allocator.allocate(1);
//         _allocator.construct(new_root, btree_node());
//         new_root->_pointers.push_back(_root);
//         split_child(new_root, 0);
//         _root = new_root;
//     }
//     btree_node* current = _root;
//     std::stack<std::pair<btree_node**, size_t>> path;

//     while (true) {
//         size_t i = 0;
//         while (i < current->_keys.size() && compare::operator()(current->_keys[i].first, data.first)) {
//             ++i;
//         }

//         if (current->_pointers.empty()) {
//             current->_keys.insert(current->_keys.begin() + i, data);
//             _size++;
//             return {btree_iterator(path, i), true};
//         }

//         if (current->_pointers[i]->_keys.size() == maximum_keys_in_node) {
//             split_child(current, i);
//             if (compare::operator()(data.first, current->_keys[i].first)) {
//                 i = 0;
//             } else {
//                 i = 1;
//             }
//         }

//         path.emplace(&current->_pointers[i], i);
//         current = current->_pointers[i];
//     }
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
// B_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
// {
//     auto it = find(data.first);
//     if (it != end()) {
//         return {it, false};
//     }

//     if (!_root) {
//         _root = _allocator.allocate(1);
//         _allocator.construct(_root, btree_node());
//         _root->_keys.emplace_back(std::move(data));
//         _size = 1;
//         return {btree_iterator({}, 0), true};
//     }

//     if (_root->_keys.size() == maximum_keys_in_node) {
//         btree_node* new_root = _allocator.allocate(1);
//         _allocator.construct(new_root, btree_node());
//         new_root->_pointers.push_back(_root);
//         split_child(new_root, 0);
//         _root = new_root;
//     }

//     btree_node* current = _root;
//     std::stack<std::pair<btree_node**, size_t>> path;

//     while (true) {
//         size_t i = 0;
//         while (i < current->_keys.size() && compare::operator()(current->_keys[i].first, data.first)) {
//             ++i;
//         }

//         if (current->_pointers.empty()) {
//             current->_keys.emplace(current->_keys.begin() + i, std::move(data));
//             _size++;
//             return {btree_iterator(path, i), true};
//         }

//         if (current->_pointers[i]->_keys.size() == maximum_keys_in_node) {
//             split_child(current, i);
//             if (compare::operator()(data.first, current->_keys[i].first)) {
//                 i = 0;
//             } else {
//                 i = 1;
//             }
//         }

//         path.emplace(&current->_pointers[i], i);
//         current = current->_pointers[i];
//     }
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// void B_tree<tkey, tvalue, compare, t>::split_child(btree_node* parent, size_t index)
// {
//     btree_node* child = parent->_pointers[index];
//     btree_node* new_child = _allocator.allocate(1);
//     _allocator.construct(new_child, btree_node());

//     const size_t median = minimum_keys_in_node;
    
//     parent->_keys.emplace(parent->_keys.begin() + index, std::move(child->_keys[median]));
    
//     new_child->_keys.reserve(minimum_keys_in_node);
//     auto keys_start = std::make_move_iterator(child->_keys.begin() + median + 1);
//     auto keys_end = std::make_move_iterator(child->_keys.end());
//     new_child->_keys.insert(new_child->_keys.end(), keys_start, keys_end);
//     child->_keys.erase(child->_keys.begin() + median, child->_keys.end());
//     if (!child->_pointers.empty()) {
//         new_child->_pointers.reserve(t + 1);
//         auto ptrs_start = std::make_move_iterator(child->_pointers.begin() + median + 1);
//         auto ptrs_end = std::make_move_iterator(child->_pointers.end());
//         new_child->_pointers.insert(new_child->_pointers.end(), ptrs_start, ptrs_end);
//         child->_pointers.erase(child->_pointers.begin() + median + 1, child->_pointers.end());
//     }

//     parent->_pointers.emplace(parent->_pointers.begin() + index + 1, new_child);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// template<typename... Args>
// std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
// B_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
// {
//     tree_data_type data(std::forward<Args>(args)...);
    
//     return insert(std::move(data));
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
// {
//     auto [it, inserted] = insert(data);
//     if (!inserted) {
//         const_cast<tvalue&>(it->second) = data.second;
//     }
//     return it;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
// {
//     auto [it, inserted] = insert(std::move(data));
//     if (!inserted) {
//         const_cast<tvalue&>(it->second) = std::move(data.second);
//     }
//     return it;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// template<typename... Args>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
// {
//     tree_data_type data(std::forward<Args>(args)...);
    
//     auto [it, inserted] = insert(std::move(data));
    
//     if (!inserted) {
//         const_cast<tvalue&>(it->second) = std::move(data.second);
//     }
//     return it;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos)
// {
//     if (pos == end()) return end();
    
//     auto next_pos = pos;
//     ++next_pos;
    
//     auto path = pos._path;
//     size_t index = pos._index;
//     btree_node* node = *(path.top().first);
    
//     if (node->_pointers.empty()) {
//         node->_keys.erase(node->_keys.begin() + index);
//         _size--;
        
//         balance_after_erase(node, path);
//         return next_pos;
//     }
    
//     auto predecessor = find_predecessor(pos);
//     std::swap(*pos, *predecessor);
//     return erase(predecessor);
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos)
// {
//     auto path = pos._path;
//     size_t index = pos._index;
//     return erase(btree_iterator(path, index));
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, btree_iterator en)
// {
//     while (beg != en) {
//         beg = erase(beg);
//     }
//     return en;
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg, btree_const_iterator en)
// {
//     return erase(btree_iterator(beg), btree_iterator(en));
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// typename B_tree<tkey, tvalue, compare, t>::btree_iterator
// B_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
// {
//     auto it = find(key);
//     if (it == end()) return end();
//     return erase(it);
// }


// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// void B_tree<tkey, tvalue, compare, t>::balance_after_erase(btree_node* node, 
//     std::stack<std::pair<btree_node**, size_t>>& path)
// {
//     while (node->_keys.size() < minimum_keys_in_node) {
//         if (node == _root) {
//             if (_root->_keys.empty() && !_root->_pointers.empty()) {
//                 _root = _root->_pointers[0];
//             }
//             return;
//         }
        
//         auto parent_info = path.top();
//         path.pop();
//         btree_node* parent = *parent_info.first;
//         size_t index = parent_info.second;
        
//         if (index > 0 && parent->_pointers[index-1]->_keys.size() > minimum_keys_in_node) {
//             borrow_from_left(parent, index);
//             return;
//         }

//         if (index < parent->_pointers.size()-1 && 
//             parent->_pointers[index+1]->_keys.size() > minimum_keys_in_node) {
//             borrow_from_right(parent, index);
//             return;
//         }
        
//         if (index > 0) {
//             merge_nodes(parent, index-1);
//         } else {
//             merge_nodes(parent, index);
//         }
        
//         node = parent;
//     }
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// void B_tree<tkey, tvalue, compare, t>::borrow_from_left(btree_node* parent, size_t index)
// {
//     btree_node* left = parent->_pointers[index-1];
//     btree_node* curr = parent->_pointers[index];
    
//     curr->_keys.insert(curr->_keys.begin(), parent->_keys[index-1]);
//     parent->_keys[index-1] = left->_keys.back();
//     left->_keys.pop_back();
    
//     if (!left->_pointers.empty()) {
//         curr->_pointers.insert(curr->_pointers.begin(), left->_pointers.back());
//         left->_pointers.pop_back();
//     }
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// void B_tree<tkey, tvalue, compare, t>::borrow_from_right(btree_node* parent, size_t index)
// {
//     btree_node* curr = parent->_pointers[index];
//     btree_node* right = parent->_pointers[index + 1];
    
//     curr->_keys.push_back(parent->_keys[index]);
    
//     parent->_keys[index] = right->_keys.front();
//     right->_keys.erase(right->_keys.begin());
    
//     if (!right->_pointers.empty()) {
//         curr->_pointers.push_back(right->_pointers.front());
//         right->_pointers.erase(right->_pointers.begin());
//     }
// }

// template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// void B_tree<tkey, tvalue, compare, t>::merge_nodes(btree_node* parent, size_t index)
// {
//     btree_node* left = parent->_pointers[index];
//     btree_node* right = parent->_pointers[index+1];
    
//     left->_keys.push_back(parent->_keys[index]);
    
//     left->_keys.insert(left->_keys.end(), right->_keys.begin(), right->_keys.end());
//     left->_pointers.insert(left->_pointers.end(), right->_pointers.begin(), right->_pointers.end());
    
//     parent->_keys.erase(parent->_keys.begin() + index);
//     parent->_pointers.erase(parent->_pointers.begin() + index + 1);
    
//     _allocator.destroy(right);
//     _allocator.deallocate(right, 1);
// }
// // endregion modifiers implementation

// // template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// // bool compare_pairs(const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &lhs,
// //                    const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &rhs)
// // {
// //     return this->compare_keys(lhs.first, rhs.first);
// // }

// // template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
// // bool compare_keys(const tkey &lhs, const tkey &rhs)
// // {
// //     return static_cast<const compare&>(*this)(lhs, rhs);
// // }


// #endif