#pragma once

#ifndef LTR_ITERATORS
#define LTR_ITERATORS

#include <utility>
#include <iterator>
#include <memory>

#include "node.hpp"

namespace ltr {

// Bidirectional iterator class
template<typename T,	// associated trie type
		 bool is_const>
class _Iterator_base {
private:
	template<bool is_const>
	struct value_type_struct;

	template<>
	struct value_type_struct<true> {
		using get = typename const T::value_type;
	};

	template<>
	struct value_type_struct<false> {
		using get = typename T::value_type;
	};

	using trie_type   = typename T;
	using node_type   = typename trie_type::node_type;
	using key_type    = typename trie_type::key_type;
	using concat_type = typename trie_type::concat_type;
	using mapped_type = typename trie_type::mapped_type;

public:
	using difference_type   = typename std::ptrdiff_t;
	using value_type        = typename value_type_struct<is_const>::get;
	using pointer           = typename value_type*;
	using reference         = typename value_type&;
	using iterator_category = typename std::bidirectional_iterator_tag;

	_Iterator_base() noexcept : node(nullptr) {}
	_Iterator_base(const _Iterator_base& other) = default;
	_Iterator_base(node_type * node, concat_type concat) : node(node), concat(concat) {}
	_Iterator_base& operator=(const _Iterator_base & other) = default;

	reference operator*() {
		set_value();
		return *(this->value_ptr);
	}

	pointer operator->() {
		set_value();
		return (this->value_ptr).get();
	}

	bool operator== (const _Iterator_base& rhs) {
		return this->node == rhs.node;
	}

	bool operator!= (const _Iterator_base& rhs) {
		return !(*this == rhs);
	}

	// preorder-like tree traversal for next element containing a value
	// from begin to end returns a lexicographically ordered sequence of keys
	// some invariants: -node always either has value, or is the root
	//                  -leaf nodes always have value, hence no check when descending to a leaf
	_Iterator_base& operator++() {
		// case 1: current node is not a leaf
		if (node->child) {
			node = node->child;
			// traverse along left children until found a node with a value
			// although not explicitly checked, if a node has no value
			// it can't be a leaf, thus it always has a child
			while (!(node->value.has_value()) && node->child)
				node = node->child;
			return *this;
		}

		// case 2: current node has a right-side sibling
		if (node->next) {
			node = node->next;
			// traverse along left children until found a node with a value
			while (!(node->value.has_value()) && node->child)
				node = node->child;
			return *this;
		}

		// case 3: current node has no children or right-side sibling
		// need to ascend until there's a right-side sibling, or we're at the root
		while (node->next == nullptr && node->parent)
			node = node->parent;
		// same as case 2
		if (node->next) {
			node = node->next;
			while (!(node->value.has_value()))
				node = node->child;
		}
		// node is either root or a node in a rightside subtree
		return *this;
	}

	_Iterator_base operator++(int) {
		_Iterator_base old = *this;
		operator++();
		return old;
	}

	_Iterator_base& operator--() {
		// case 1: node has a left-side sibling - either leaf or intermediate node
		if (node->prev) {
			node = node->prev;
			// go to its rightmost child until on a leaf
			while (node->child) {
				node = node->child;
				while (node->next)
					node = node->next;
			}
			return *this;
		}

		// case 2: no left-side sibling
		// ascend until found a node with a value, or a left-side sibling
		if (node->parent) {
			node = node->parent;
			while (!(node->value.has_value()) && node->prev == nullptr && node->parent)
				node = node->parent;

			// if found a value, return
			if (node->value.has_value())
				return *this;

			// if there's a left-side sibling, repeat case 1
			if (node->prev) {
				node = node->prev;
				while (node->child) {
					node = node->child;
					while (node->next)
						node = node->next;
				}
			}

			// reached left-subtree target node or the root
			return *this;
		}
		
		// case 3: started at the root
		// descend to the rightmost child
		while (node->child) {
			node = node->child;
			while (node->next)
				node = node->next;
		}
		return *this;
	}

	_Iterator_base operator--(int) {
		_Iterator_base old = *this;
		operator--();
		return old;
	}

private:

	void set_value() {
		assert(node->value.has_value());
		mapped_type& v = *(node->value);
		value_type val = value_type(concat(node), v);
		value_ptr = std::make_shared<value_type>(val);
	}

	node_type* node;
	std::shared_ptr<value_type> value_ptr;
	concat_type concat;
};

} // namespace ltr

#endif // LTR_ITERATORS
