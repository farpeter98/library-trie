#pragma once

#ifndef LTR_ITERATORS
#define LTR_ITERATORS

#include <utility>
#include <iterator>
#include <memory>
#include <stack>
#include <type_traits>

#include "node.hpp"

namespace ltr {

template<typename K, typename M, bool const_v>
struct consted_type;

// Bidirectional iterator class
// Not an outputiterator in this version
template<typename N,	// associated node type
         typename Conc,
         typename K,
         typename V,
         bool is_const,
         bool is_reverse>
class _Iterator_base {
private:
	using node_type   = N;
	using key_concat  = Conc;
	using key_type    = K;
	using mapped_type = V;
public:
	// no reference_type, so no iterator_tag either
	using difference_type   = std::ptrdiff_t;
	using value_type        = typename consted_type<key_type, mapped_type, is_const>::val;
	using pointer           = typename consted_type<key_type, mapped_type, is_const>::ptr;
	using reference         = value_type;
	using iterator_category = std::bidirectional_iterator_tag;

	constexpr _Iterator_base() noexcept : node(nullptr) {}
	constexpr _Iterator_base(const _Iterator_base& other) noexcept = default;
	constexpr _Iterator_base(node_type* node) noexcept : node(node) {}
	constexpr _Iterator_base& operator=(const _Iterator_base& other) noexcept = default;

	// this returns a copy, not a reference
	value_type operator*() {
		const key_type& key = get_key(node);
		value_type val = std::make_pair(std::move(key), std::move(node->value.value()));
		return val;
	}

	pointer operator->() {
		return std::make_unique<value_type>(get_key(node), std::move(node->value.value()));
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

	static const key_type get_key(node_type* node) {
		static key_concat instance{};
		std::stack<node_type*> parents{};
		key_type key{};
		while (node->parent) {
			parents.push(node);
			node = node->parent;
		}
		while (!parents.empty()) {
			node = parents.top();
			key = instance(key, node->key);
			parents.pop();
		}
		return key;
	}

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

template<typename K, typename M>
struct consted_type<K, M, true> {
	using val = const std::pair<const K, M>;
	using ptr = std::unique_ptr<val>;
};

template<typename K, typename M>
struct consted_type<K, M, false> {
	using val = std::pair<const K, M>;
	using ptr = std::unique_ptr<val>;
};

} // namespace ltr

#endif // LTR_ITERATORS
