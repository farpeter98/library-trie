#pragma once

#ifndef LTR_TRIE
#define LTR_TRIE

#include <utility>
#include <string>
#include <memory>
#include <type_traits>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>

#include "node.hpp"
#include "iterators.hpp"

namespace ltr {

template<typename K,
		 typename V,
		 typename Concat_expr_t,
		 template<typename T>    typename Comp   = std::less,
		 template<typename SeqT, typename SeqTraits, typename SeqAlloc> typename Seq = std::basic_string,
		 template<typename T>    typename Traits = std::char_traits,
		 template<typename T>    typename Alloc  = std::allocator>
class trie {
public:

	// ---------------- member types ---------------

	using key_type               = Seq<K, Traits<K>, Alloc<K>>;
	using mapped_type            = V;
	using value_type             = std::pair<const key_type, V>;
	using key_concat             = Concat_expr_t;
	using size_type              = std::size_t;
	using difference_type        = std::ptrdiff_t;
	using key_compare            = Comp<K>;
	using allocator_type         = Alloc<value_type>;
	using reference              = value_type&;
	using const_reference        = const value_type&;
	using pointer                = typename std::allocator_traits<allocator_type>::pointer;
	using const__pointer         = typename std::allocator_traits<allocator_type>::const_pointer;
	using node_type              = _Node<K, value_type, Alloc>;
	using iterator               = _Iterator_base<node_type, false, false>;
	using const_iterator         = _Iterator_base<node_type, true, false>;
	using reverse_iterator       = _Iterator_base<node_type, false, true>;
	using const_reverse_iterator = _Iterator_base<node_type, true, true>;

	// ----------- ctors and assignment ------------

	constexpr trie() noexcept = delete;
	trie(const key_concat& concat, const key_compare& comp = key_compare()) : _concat(concat), _comp(comp), _root(new node_type()) {}

	template<typename InputIt>
	trie(const key_concat& concat,
		InputIt first, InputIt last,
		const key_compare& comp = key_compare()) : trie(concat, comp)
	{
		insert(first, last);
	}
	trie(const trie& other) : _root(new node_type(*(other._root))), _concat(other._concat), _comp(other._comp) {}
	trie(trie&& other) noexcept : _root(other._root), _concat(std::move(other._concat)), _comp(std::move(other._comp)) { other._root = nullptr; }
	trie(std::initializer_list<value_type> init,
		const key_concat& concat,
		const key_compare& comp = key_compare()) : trie(concat, comp)
	{
		insert(init);
	}

	~trie() {
		// need nullptr check in case _root was taken by move
		if (_root)
			delete _root;
	}

	trie& operator=(const trie& other) {
		if (this != &other) {
			delete _root;

			// no need to copy _comp and _concat as the matching type ensures they are the same
			_root = new node_type(*(other._root));
		}
		return *this;
	}

	trie& operator=(trie&& other) noexcept {
		if (this != &other) {
			delete _root;

			// no need to take _comp and _concat as the matching type ensures they are the same
			_root = other._root;
			other._root = nullptr;
		}
		return *this;
	}

	trie& operator=(std::initializer_list<value_type> init) {
		delete _root;
		_root = new node_type();
		insert(init);
		return *this;
	}

	// -------------- element access ---------------

	mapped_type& at(const key_type& key) {
		const std::pair<node_type*, bool>& result = std::move(try_find(key));
		if (!result.second)
			throw std::out_of_range("invalid trie key");

		return result.first->value->second;
	}

	const mapped_type& at(const key_type& key) const {
		const std::pair<node_type*, bool>& result = std::move(try_find(key));
		if (!result.second)
			throw std::out_of_range("invalid trie key");

		return result.first->value->second;
	}

	mapped_type& operator[](const key_type& key) {
		node_type* target = try_insert(key);
		if (!(target->value.has_value()))
			target->value.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>());
		return target->value->second;
	}

	mapped_type& operator[](key_type&& key) {
		node_type* target = try_insert(key);
		if (!(target->value.has_value()))
			target->value.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::tuple<>());
		return target->value->second;
	}

	// ----------------- iterators -----------------

	iterator begin() noexcept {
		iterator it(_root);
		++it;
		return it;
	}

	const_iterator begin() const noexcept {
		const_iterator it(_root);
		++it;
		return it;
	}

	const_iterator cbegin() const noexcept {
		const_iterator it(_root);
		++it;
		return it;
	}

	reverse_iterator rbegin() noexcept {
		reverse_iterator it(_root);
		++it;
		return it;
	}

	const_reverse_iterator rbegin() const noexcept {
		const_reverse_iterator it(_root);
		++it;
		return it;
	}

	const_reverse_iterator crbegin() const noexcept {
		const_reverse_iterator it(_root);
		++it;
		return it;
	}

	constexpr iterator end() noexcept {
		return iterator(_root);
	}

	constexpr const_iterator end() const noexcept {
		return const_iterator(_root);
	}

	constexpr const_iterator cend() const noexcept {
		return const_iterator(_root);
	}

	constexpr reverse_iterator rend() noexcept {
		return reverse_iterator(_root);
	}

	constexpr const_reverse_iterator rend() const noexcept {
		return const_reverse_iterator(_root);
	}

	constexpr const_reverse_iterator crend() const noexcept {
		return const_reverse_iterator(_root);
	}

	// ----------------- capacity ------------------

	bool empty() const noexcept {
		return _root->child == nullptr;
	}

	size_type size() const {
		return std::distance(begin(), end());
	}

	// ----------------- modifiers -----------------

	void clear() {
		node_type copy;
		// dtor called at end of scope will destroy the tree below the root
		// this over deleting and allocating root again, to not invalidate iterators poiting to end
		copy.child = _root->child;
		_root->child = nullptr;
	}

	std::pair<iterator, bool> insert(const value_type& value) {
		node_type* target = try_insert(value.first);
		bool has_value = target->value.has_value();
		if (!has_value)
			target->value.emplace(value);
		return std::make_pair(std::move(iterator(target)), !has_value);
	}

	template<typename P,
		std::enable_if_t<std::is_constructible<value_type, P&&>::value, bool> = true>
	std::pair<iterator, bool> insert(P&& value) {
		return emplace(std::forward<P>(value));
	}

	std::pair<iterator, bool> insert(value_type&& value) {
		node_type* target = try_insert(value.first);
		bool has_value = target->value.has_value();
		if (!has_value)
			target->value.emplace(std::move(value));
		return std::make_pair(std::move(iterator(target)), !has_value);
	}

	template<typename InputIt>
	void insert(InputIt first, InputIt last) {
		for (InputIt it = first; it != last; ++it) {
			emplace(*it);
		}
	}

	void insert(std::initializer_list<value_type> init) {
		for (const value_type& val : init) {
			emplace(val);
		}
	}

	template<typename M,
		     std::enable_if_t<std::is_assignable<mapped_type&, M&&>::value, bool> = true>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& obj) {
		node_type* target = try_insert(key);
		bool has_value = target->value.has_value();
		target->value.emplace(value_type(key, std::forward<M>(obj)));
		return std::make_pair(std::move(iterator(target)), !has_value);
	}

	template<typename M,
	         std::enable_if_t<std::is_assignable<mapped_type&, M&&>::value, bool> = true>
	std::pair<iterator, bool> insert_or_assign(key_type&& key, M&& obj) {
		node_type* target = try_insert(key);
		bool has_value = target->value.has_value();
		target->value.emplace(value_type(std::move(key), std::forward<M>(obj)));
		return std::make_pair(std::move(iterator(target)), !has_value);
	}

	template<typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		value_type value(std::forward<Args>(args)...);
		node_type* target = try_insert(value.first);
		bool has_value = target->value.has_value();
		if (!has_value)
			target->value.emplace(std::move(value));
		return std::make_pair(std::move(iterator(target)), !has_value);
	}

	template<typename... Args>
	std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
		node_type* target = try_insert(key);
		bool has_value = target->value.has_value();
		if (!has_value) {
			value_type value(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<Args>(args)...));
			target->value.emplace(std::move(value));
		}
		return std::make_pair(std::move(iterator(target)), !has_value);
	}

	template<typename... Args>
	std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
		node_type* target = try_insert(key);
		bool has_value = target->value.has_value();
		if (!has_value) {
			value_type value(std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward_as_tuple(std::forward<Args>(args)...));
			target->value.emplace(std::move(value));
		}
		return std::make_pair(std::move(iterator(target)), !has_value);
	}

	iterator erase(iterator pos) {
		node_type* node = get_node(pos);
		++pos;
		// if node has a subtree, only remove the value
		if (node->child)
			node->value.reset();
		// else remove the node and all now obsolete nodes
		else
			node->remove_branch();
		return pos;
	}

	iterator erase(const_iterator first, const_iterator last) {
		while (first != last) {
			node_type* node = get_node(first);
			++first;
			if (node->child)
				node->value.reset();
			else
				node->remove_branch();
		}
		return iterator(get_node(first));
	}

	size_type erase(const key_type& key) {
		const std::pair<node_type*, bool>& result = std::move(try_find(key));
		if (result.second && result.first->value.has_value()) {
			if (result.first->child)
				result.first->value.reset();
			else
				result.first->remove_branch();
			return 1;
		}
		return 0;
	}

	constexpr void swap(trie& other) noexcept {
		node_type* tmp = _root;
		this->_root = other._root;
		other._root = tmp;
	}

	// ------------------ lookup -------------------

	size_type count(const key_type& key) const {
		return try_find(key).second ? 1 : 0;
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<
		                      std::is_default_constructible<comp>,
			                  std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	size_type count(const key_t& key) const {
		return try_find<key_t, comp>(key).second ? 1 : 0;
	}

	iterator find(const key_type& key) {
		const std::pair<node_type*, bool>& result = std::move(try_find(key));
		if (result.second)
			return iterator(result.first);
		return end();
	}

	const_iterator find(const key_type& key) const {
		const std::pair<node_type*, bool>& result = std::move(try_find(key));
		if (result.second)
			return const_iterator(result.first);
		return cend();
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	iterator find(const key_t& key) {
		const std::pair<node_type*, bool>& result = std::move(try_find<key_t, comp>(key));
		if (result.second)
			return iterator(result.first);
		return end();
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	const_iterator find(const key_t& key) const {
		const std::pair<node_type*, bool>& result = std::move(try_find<key_t, comp>(key));
		if (result.second)
			return const_iterator(result.first);
		return cend();
	}

	bool contains(const key_type& key) const {
		return try_find(key).second;
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	bool contains(const key_t& key) const {
		return try_find<key_t, comp>(key).second;
	}

	std::pair<iterator, iterator> equal_range(const key_type& key) {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	std::pair<iterator, iterator> equal_range(const key_t& key) {
		return std::make_pair(lower_bound<key_t, comp>(key), upper_bound<key_t, comp>(key));
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	std::pair<const_iterator, const_iterator> equal_range(const key_t& key) const {
		return std::make_pair(lower_bound<key_t, comp>(key), upper_bound<key_t, comp>(key));
	}

	iterator lower_bound(const key_type& key) {
		const std::pair<node_type*, bool>& result = try_find(key);
		if (result.second)
			return iterator(result.first);
		node_type* node = result.first;
		return iterator(find_next(key, node));
	}

	const_iterator lower_bound(const key_type& key) const {
		const std::pair<node_type*, bool>& result = try_find(key);
		if (result.second)
			return const_iterator(result.first);
		node_type* node = result.first;
		return const_iterator(find_next(key, node));
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	iterator lower_bound(const key_t& key) {
		iterator it = begin();
		comp instance{};
		while (get_node(it) != _root && instance(it->first, key))
			++it;
		return it;
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	const_iterator lower_bound(const key_t& key) const {
		const_iterator it = begin();
		comp instance{};
		while (get_node(it) != _root && instance(it->first, key))
			++it;
		return it;
	}

	iterator upper_bound(const key_type& key) {
		const std::pair<node_type*, bool>& result = try_find(key);
		if (result.second) {
			return ++iterator(result.first);
		}
		node_type* node = result.first;
		return iterator(find_next(key, node));
	}

	const_iterator upper_bound(const key_type& key) const {
		const std::pair<node_type*, bool>& result = try_find(key);
		if (result.second) {
			return ++const_iterator(result.first);
		}
		node_type* node = result.first;
		return const_iterator(find_next(key, node));
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	iterator upper_bound(const key_t& key) {
		iterator it = begin();
		comp instance{};
		while (get_node(it) != _root && !instance(key, it->first))
			++it;
		return it;
	}

	template<typename key_t, typename comp = key_compare,
		     typename = typename comp::is_transparent,
		     std::enable_if_t<std::conjunction_v<std::is_default_constructible<comp>,
		                                         std::negation<std::is_convertible<key_t, key_type>>>,
		     bool> = true>
	const_iterator upper_bound(const key_t& key) const {
		const_iterator it = begin();
		comp instance{};
		while (get_node(it) != _root && !instance(key, it->first))
			++it;
		return it;
	}

	// ----------------- observers -----------------

	key_compare key_comp() const {
		return key_compare();
	}

	// ----------------- nonmember -----------------

	friend bool operator==(const trie& lhs, const trie& rhs) {
		return !(lhs < rhs) && !(rhs < lhs);
	}

	friend bool operator!=(const trie& lhs, const trie& rhs) {
		return !(lhs == rhs);
	}

	friend bool operator<(const trie& lhs, const trie& rhs) {
		return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	friend bool operator<=(const trie& lhs, const trie& rhs) {
		return !(rhs < lhs);
	}

	friend bool operator>(const trie& lhs, const trie& rhs) {
		return rhs < lhs;
	}

	friend bool operator>=(const trie& lhs, const trie& rhs) {
		return !(lhs < rhs);
	}

	friend constexpr void swap(trie& lhs, trie& rhs) noexcept {
		lhs.swap(rhs);
	}

private:

	// function to find the node of the given key,
	// creating the intermediate nodes in the process, if neccessary
	node_type* try_insert(const key_type& key) {
		if (key.size() == 0)
			throw std::invalid_argument("key must be of positive size");
		node_type* current = _root;
		// note that shortly after every new there's a continue for early "return"
		for (const K& fragment : key) {
			// action 1: insert fragment as new child
			// occurs when current has no child
			if (current->child == nullptr) {
				current->set_child(new node_type(fragment));
				current = current->child;
				continue;
			}

			current = current->child;

			// first handle cases around first child
			if (!_comp(current->key, fragment)) {
				// first child has same key as fragment
				if (!_comp(fragment, current->key))
					continue;
				// first child has bigger key than fragment
				current->set_prev(new node_type(fragment));
				current = current->prev;
				continue;
			}

			// traverse nodes and stop on the last with a smaller key than fragment
			while (current->next && _comp(current->next->key, fragment))
				current = current->next;

			// case 2: we're at last node with key smaller than fragment
			if (current->next == nullptr || _comp(fragment, current->next->key)) {
				current->set_next(new node_type(fragment));
				current = current->next;
				continue;
			}

			// case 3: next node has same key as fragment
			current = current->next;
		}
		return current;
	}

	// for general keys lookup compares to the entire key, not per-fragment
	// which requires the comparator to also provide comparison for key_type, not just K
	template<typename key_t, typename comp,
		     std::enable_if_t<std::is_default_constructible_v<comp>, bool> = true>
	const std::pair<node_type*, bool> try_find(const key_t& key) const {
		comp instance{};
		for (const_iterator it = cbegin(); it != cend(); ++it) {
			if (!instance(key, it->first) && !instance(it->first, key))
				return std::make_pair(get_node(it), true);
		}
		return std::make_pair(_root, false);
	}

	// similar to try_insert_key, but returns when creating a new node would be required
	const std::pair<node_type*, bool> try_find(const key_type& key) const {
		if (key.size() == 0)
			throw std::invalid_argument("key must be of positive size");
		node_type* current = _root;
		for (const K& fragment : key) {
			if (current->child == nullptr)
				return std::make_pair(current, false);

			current = current->child;
			while (current->next && _comp(current->key, fragment))
				current = current->next;

			// still need to check both _comp-s next might be nullptr
			if (_comp(fragment, current->key) || _comp(current->key, fragment))
				return std::make_pair(current, false);
		}
		return std::make_pair(current, current->value.has_value());
	}

	// helper function used for bounds functions
	// the param node is presumably supplied by try_find
	node_type* find_next(const key_type& key, node_type* node) const {
		// early check for root
		if (node == _root)
			return node;

		node_type* current = node;
		// first exit point of try_find -> here key definitely is smaller
		if (current->child == nullptr) {
			while (current->parent != nullptr && current->next == nullptr)
				current = current->parent;
			if (current->next) {
				current = current->next;
				// no need to check if child is nullptr, because leaves always contain a value
				while (!current->value.has_value())
					current = current->child;
			}
			// this might return _root
			return current;
		}
		// second exit point, current can be either greater or less
		// if next is not nullptr, current node is always greater
		if (current->next != nullptr) {
			while (!current->value.has_value())
				current = current->child;
			return current;
		}
		
		// find out if the key was smaller or greater
		// first get to the mismatched char
		auto it = key.begin();
		node_type* temp = current;
		// checking for temp == _root would cause one more increment than neccessary
		// rather have it this way than requiring the iterator to be bidirectional
		while (temp->parent != _root) {
			++it;
			temp = temp->parent;
		}
		// node's key was smaller, same as at first exit point
		if (_comp(current->key, *it)) {
			while (current->parent != nullptr && current->next == nullptr)
				current = current->parent;
			if (current->next) {
				current = current->next;
				while (!current->value.has_value())
					current = current->child;
			}
			// this might return _root
			return current;
		}
		// node's key was greater, find first value in subtree
		// since key was not found equality can never occur
		while (!current->value.has_value())
			current = current->child;
		return current;
	}

	key_concat _concat;
	node_type* _root;
	const key_compare _comp;

}; // class trie

} // namespace ltr

#endif // LTR_TRIE
