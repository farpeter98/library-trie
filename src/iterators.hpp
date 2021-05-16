#pragma once

#ifndef LTR_ITERATORS
#define LTR_ITERATORS

#include <utility>
#include <iterator>
#include <memory>

#include "node.hpp"

namespace ltr {

template<typename N, bool const_v>
struct consted_type;

// Bidirectional iterator class
template<typename N,	// associated node type
		 bool is_const,
         bool is_reverse>
class _Iterator_base {
private:
	using node_type  = N;
public:
	using difference_type   = std::ptrdiff_t;
	using value_type        = typename consted_type<N, is_const>::val;
	using pointer           = typename consted_type<N, is_const>::ptr;
	using reference         = typename consted_type<N, is_const>::ref;
	using iterator_category = std::bidirectional_iterator_tag;

	constexpr _Iterator_base() noexcept : node(nullptr) {}
	constexpr _Iterator_base(const _Iterator_base& other) noexcept = default;
	constexpr _Iterator_base(node_type* node) noexcept : node(node) {}
	constexpr _Iterator_base& operator=(const _Iterator_base& other) noexcept = default;

	reference operator*() {
		return *(node->value);
	}

	pointer operator->() {
		return node->value.operator->();
	}

	friend bool operator==(const _Iterator_base& lhs, const _Iterator_base& rhs) {
		return lhs.node == rhs.node;
	}

	friend bool operator!=(const _Iterator_base& lhs, const _Iterator_base& rhs) {
		return !(lhs == rhs);
	}

	_Iterator_base& operator++() noexcept {
		if constexpr (is_reverse)
			decrement();
		else
			increment();
		return *this;
	}

	_Iterator_base operator++(int) noexcept {
		_Iterator_base old = *this;
		operator++();
		return old;
	}

	_Iterator_base& operator--() noexcept {
		if constexpr (is_reverse)
			increment();
		else
			decrement();
		return *this;
	}

	_Iterator_base operator--(int) noexcept {
		_Iterator_base old = *this;
		operator--();
		return old;
	}

	constexpr friend node_type* get_node(const _Iterator_base& it) noexcept {
		return it.node;
	}

private:
	// preorder-like tree traversal for next element containing a value
	// from begin to end returns a lexicographically ordered sequence of keys
	// some invariants: -node always either has value, or is the root
	//                  -leaf nodes always have value, hence no check when descending to a leaf
	void increment() noexcept {
		// case 1: current node is not a leaf
		if (node->child) {
			node = node->child;
			// traverse along left children until found a node with a value
			// although not explicitly checked, if a node has no value
			// it can't be a leaf, thus it always has a child
			while (!(node->value.has_value()))
				node = node->child;
			return;
		}

		// case 2: current node has a right-side sibling
		if (node->next) {
			node = node->next;
			// traverse along left children until found a node with a value
			while (!(node->value.has_value()))
				node = node->child;
			return;
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
	}

	void decrement() noexcept {
		// case 1: node has a left-side sibling - either leaf or intermediate node
		if (node->prev) {
			node = node->prev;
			// go to its rightmost child until on a leaf
			while (node->child) {
				node = node->child;
				while (node->next)
					node = node->next;
			}
			return;
		}

		// case 2: no left-side sibling
		// ascend until found a node with a value, or a left-side sibling
		if (node->parent) {
			node = node->parent;
			while (!(node->value.has_value()) && node->prev == nullptr && node->parent)
				node = node->parent;

			// if found a value, return
			if (node->value.has_value())
				return;

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
			return;
		}

		// case 3: started at the root
		// descend to the rightmost child
		while (node->child) {
			node = node->child;
			while (node->next)
				node = node->next;
		}
	}

	node_type* node;
}; // class _Iterator_base

template<typename N>
struct consted_type<N, true> {
	using val = const typename N::value_type;
	using ptr = const val*;
	using ref = const val&;
};

template<typename N>
struct consted_type<N, false> {
	using val = typename N::value_type;
	using ptr = val*;
	using ref = val&;
};

} // namespace ltr

#endif // LTR_ITERATORS
