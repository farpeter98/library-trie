#pragma once

#ifndef LTR_TRIE
#define LTR_TRIE

#include <utility>
#include <string>
#include <memory>
#include <type_traits>
#include <initializer_list>
#include <stdexcept>
#include <iostream>

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

	using key_type               = typename Seq<K, Traits<K>, Alloc<K>>;
	using mapped_type            = typename V;
	using value_type             = typename std::pair<const key_type, V>;
	using key_concat             = typename Concat_expr_t;
	using size_type              = typename std::size_t;
	using difference_type        = typename std::ptrdiff_t;
	using key_compare            = typename Comp<K>;
	using allocator_type         = typename Alloc<value_type>;
	using reference              = typename value_type&;
	using const_reference        = typename const value_type&;
	using pointer                = typename std::allocator_traits<allocator_type>::pointer;
	using const__pointer         = typename std::allocator_traits<allocator_type>::const_pointer;
	using node_type              = typename _Node<K, value_type, Alloc>;
	using iterator               = typename _Iterator_base<node_type, false, false>;
	using const_iterator         = typename _Iterator_base<node_type, true, false>;
	using reverse_iterator       = typename _Iterator_base<node_type, false, true>;
	using const_reverse_iterator = typename _Iterator_base<node_type, true, true>;
	
	// ----------- ctors and assignment ------------

	constexpr trie() noexcept = delete;
	trie(const key_concat& concat, const key_compare& comp = key_compare()) : _concat(concat), _comp(comp), _root(new node_type()) {}

	template<typename InputIt>
	trie(const key_concat& concat,
		 InputIt first, InputIt last,
		 const key_compare& comp = key_compare()) : trie(concat, comp)
	{
		for (InputIt it = first; it != last; ++it) {
			emplace(*it);
		}
	}
	trie(const trie& other) : _root(new node_type(*(other._root))), _concat(other._concat), _comp(other._comp) {}
	trie(trie&& other) noexcept : _root(other._root), _concat(std::move(other._concat)), _comp(std::move(other._comp)) { other._root = nullptr; }
	trie(std::initializer_list<value_type> init,
		 const key_concat& concat,
		 const key_compare& comp = key_compare()) : trie(concat, comp)
	{
		for (const value_type& val : init) {
			insert(val);
		}
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
		for (const value_type& val : init) {
			emplace(init);
		}
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
			throw std::out_of_range();

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

	// ----------------- capacity ------------------

	bool empty() const noexcept {
		return begin() == end();
	}

	size_type size() const {
		return std::distance(begin(), end());
	}

	// max_size()

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
		bool result = target->value.has_value();
		if (!result)
			target->value.emplace(value);
		return std::make_pair(std::move(iterator(target)), result);
	}

	template<typename P,
	         std::enable_if_t<std::is_constructible<value_type, P&&>::value, bool> = true>
	std::pair<iterator, bool> insert(P&& value) {
		return emplace(std::forward<P>(value));
	}

	std::pair<iterator, bool> insert(value_type&& value) {
		node_type* target = try_insert(value.first);
		bool result = target->value.has_value();
		if (!result)
			target->value.emplace(std::move(value));
		return std::make_pair(std::move(iterator(target)), result);
	}

	template<typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		value_type value(std::forward<Args>(args)...);
		node_type* target = try_insert(value.first);
		bool result = target->value.has_value();
		if (!result)
			target->value.emplace(std::move(value));
		return std::make_pair(std::move(iterator(target)), result);
	}

	template<typename... Args>
	std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {

	}
	
	// ------------------ lookup -------------------

	size_type count(const key_type& key) {
		return try_find(key).second ? 1 : 0;
	}

	// ----------------- iterators -----------------

	iterator begin() {
		iterator it(_root);
		++it;
		return it;
	}
	
	const_iterator begin() const {
		const_iterator it(_root);
		++it;
		return it;
	}

	const_iterator cbegin() const {
		const_iterator it(_root);
		++it;
		return it;
	}

	reverse_iterator rbegin() {
		reverse_iterator it(_root);
		--it;
		return it;
	}

	const_reverse_iterator rbegin() const {
		const_reverse_iterator it(_root);
		--it;
		return it;
	}

	const_reverse_iterator crbegin() const {
		const_reverse_iterator it(_root);
		--it;
		return it;
	}

	iterator end() {
		return iterator(_root);
	}

	const_iterator end() const {
		return const_iterator(_root);
	}

	const_iterator cend() const {
		return const_iterator(_root);
	}

	reverse_iterator rend() {
		return reverse_iterator(_root);
	}

	const_reverse_iterator rend() const {
		return const_reverse_iterator(_root);
	}

	const_reverse_iterator crend() const {
		return const_reverse_iterator(_root);
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

			if (_comp(fragment, current->key))
				return std::make_pair(current, false);
		}
		return std::make_pair(current, current->value.has_value());
	}

	key_concat _concat;
	node_type* _root;
	const key_compare _comp;

}; // class trie

} // namespace ltr

#endif // LTR_TRIE
