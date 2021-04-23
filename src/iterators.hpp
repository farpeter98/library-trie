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
		 bool is_const,
         bool is_reverse>
class _Iterator_base {
private:

	template<bool is_const>
	struct constness_struct;

	template<>
	struct constness_struct<true> {
		using get = typename const T::value_type;
	};

	template<>
	struct constness_struct<false> {
		using get_type = typename T::value_type;
	};

	template<bool is_reverse>
	struct reverseness_struct;

	using node_type  = typename T::node_type;
	using mover_type = typename reverseness_struct<is_reverse>;
public:
	using difference_type   = typename std::ptrdiff_t;
	using value_type        = typename constness_struct<is_const>::get_type;
	using pointer           = typename value_type*;
	using reference         = typename value_type&;
	using iterator_category = typename std::bidirectional_iterator_tag;

	_Iterator_base() noexcept : node(nullptr) {}
	_Iterator_base(const _Iterator_base& other) = default;
	_Iterator_base(node_type* node) : node(node) {}
	_Iterator_base& operator=(const _Iterator_base & other) = default;

	reference operator*() {
		return *(node->value);
	}

	pointer operator->() {
		return node->value.operator->();
	}

	bool operator== (const _Iterator_base& rhs) {
		return this->node == rhs.node;
	}

	bool operator!= (const _Iterator_base& rhs) {
		return !(*this == rhs);
	}

	_Iterator_base& operator++() {
		node = mover_type::increment(node);
		return *this;
	}

	_Iterator_base operator++(int) {
		_Iterator_base old = *this;
		operator++();
		return old;
	}

	_Iterator_base& operator--() {
		node = mover_type::decrement(node);
		return *this;
	}

	_Iterator_base operator--(int) {
		_Iterator_base old = *this;
		operator--();
		return old;
	}

private:

	template<>
	struct reverseness_struct<false> {

		// preorder-like tree traversal for next element containing a value
		// from begin to end returns a lexicographically ordered sequence of keys
		// some invariants: -node always either has value, or is the root
		//                  -leaf nodes always have value, hence no check when descending to a leaf
		static node_type* increment(node_type* node) {
			// case 1: current node is not a leaf
			if (node->child) {
				node = node->child;
				// traverse along left children until found a node with a value
				// although not explicitly checked, if a node has no value
				// it can't be a leaf, thus it always has a child
				while (!(node->value.has_value()) && node->child)
					node = node->child;
				return node;
			}

			// case 2: current node has a right-side sibling
			if (node->next) {
				node = node->next;
				// traverse along left children until found a node with a value
				while (!(node->value.has_value()) && node->child)
					node = node->child;
				return node;
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
			return node;
		}

		static node_type* decrement(node_type* node) {
			// case 1: node has a left-side sibling - either leaf or intermediate node
			if (node->prev) {
				node = node->prev;
				// go to its rightmost child until on a leaf
				while (node->child) {
					node = node->child;
					while (node->next)
						node = node->next;
				}
				return node;
			}

			// case 2: no left-side sibling
			// ascend until found a node with a value, or a left-side sibling
			if (node->parent) {
				node = node->parent;
				while (!(node->value.has_value()) && node->prev == nullptr && node->parent)
					node = node->parent;

				// if found a value, return
				if (node->value.has_value())
					return node;

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
				return node;
			}

			// case 3: started at the root
			// descend to the rightmost child
			while (node->child) {
				node = node->child;
				while (node->next)
					node = node->next;
			}
			return node;
		}
	};
	
	template<>
	struct reverseness_struct<true> {
		static node_type* increment(node_type* node) {
			return reverseness_struct<false>::decrement(node);
		}

		static node_type* decrement(node_type* node) {
			return reverseness_struct<false>::increment(node);
		}
	};

	node_type* node;
}; // class _Iterator_base

} // namespace ltr

#endif // LTR_ITERATORS
