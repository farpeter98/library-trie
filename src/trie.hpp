#pragma once

#ifndef LTR_TRIE
#define LTR_TRIE

#include <utility>
#include <string>
#include <memory>
#include <type_traits>
#include <iostream>
#include <initializer_list>

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
	trie(const key_concat& concat, const key_compare& comp = key_compare()) : _concat(concat), _comp(comp), _root(new node_type()) {
		_root->value.emplace(std::make_pair("sajt", 42));
	}

	template<typename InputIt>
	trie(const key_concat& concat,
		 InputIt first, InputIt last,
		 const key_compare& comp = key_compare()) : trie(concat, comp)
	{
		for (InputIt it = first; it != last; ++it) {
			// TODO
		}
	}
	trie(const trie& other) : _concat(other._concat), _comp(other._comp), _root(new node_type(*(other._root))) {}
	trie(trie&& other) = default;
	trie(std::initializer_list<value_type> init,
		 const key_concat& concat,
		 const key_compare& comp = key_compare()) : trie(concat, comp)
	{
		for (const value_type& val : init) {
			// TODO
		}
	}

	~trie() {
		delete _root;
	}

	trie& operator=(const trie& other) {
		return trie(other);
	}

	trie& operator=(trie&& other) {
		return trie(std::move(other));
	}

	trie& operator=(std::initializer_list<value_type> init) {
		delete _root;
		_root = new node_type();
		for (const value_type& val : init) {
			// TODO
		}
	}

	// -------------- element access ---------------

	mapped_type& at(const key_type& key) {

	}

	const mapped_type& at(const key_type& key) const {

	}

	mapped_type& operator[](const key_type& key) {

	}

	mapped_type& operator[](key_type&& key) {

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

	// ------------------ lookup -------------------

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

	key_concat _concat;
	node_type* _root;
	const key_compare _comp;

}; // class trie

} // namespace ltr

#endif // LTR_TRIE
