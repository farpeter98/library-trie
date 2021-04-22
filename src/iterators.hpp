#pragma once

#ifndef LTR_ITERATORS
#define LTR_ITERATORS

#include <utility>
#include <iterator>
#include <memory>

#include "node.hpp"

namespace ltr {

// base class for iterators mainly responsible for creating the pair values
template<typename T> // associated trie type
class _Iterator_base {
protected:
	using trie_type   = typename T;
	using node_type   = typename trie_type::node_type;
	using key_type    = typename trie_type::key_type;
	using value_type  = typename trie_type::value_type;
	using concat_type = typename trie_type::concat_type;

public:

	_Iterator_base() noexcept : node(nullptr), prev_visited(nullptr) {}

	value_type& operator*() {
		set_value();
		return *(this->value_ptr);
	}

	value_type* operator->() {
		set_value();
		return (this->value_ptr).get();
	}

	bool operator== (const _Iterator_base& rhs) {
		return this->node == rhs.node;
	}

	bool operator!= (const _Iterator_base& rhs) {
		return !(*this == rhs);
	}

	// depth-first tree traversal for first element with value
	// some invariants: - iterator is always on either on a node with value, or the root node, in which case parent is nullptr
	//                  - all leaf nodes (meaning child is nullptr) have a value
	// also need to keep track of the previously visited node, to make it possible to go towards the root
	_Iterator_base& operator++() {
		// if not on a leaf node, and wasn't coming from towards the child, go to the leftmost leaf
		if (node->child != nullptr && node->child != prev_visited) {
			while (node->child != nullptr)
				node = node->child;
			prev_visited = node->parent;
			return *this;
		}

		// if on a leaf node, and there's a right-side sibling go to the sibling's leftmost leaf
		if (node->next != nullptr) {
			prev_visited = node;
			node = node->next;
			while (node->child != nullptr)
				node = node->child;
			// check if there was any depth traversal
			if (prev_visited != node->prev)
				prev_visited = node->parent;
			return *this;
		}

		// else go one level towards the root - need to check if parent is not nullptr, due to trees consisting only of the root
		while (node->parent != nullptr) {
			node = node->parent;
			// if the node has a value, it's a valid stop
			if (node->value.has_value()) {
				prev_visited = node->child;
				return *this;
			}

			// if the node has a right-side sibling, go to sibling's leftmost leaf
			if (node->next != nullptr) {
				prev_visited = node;
				node = node->child;
				while (node->child != nullptr)
					node = node->child;
				if (prev_visited != node->prev)
					prev_visited = node->parent;
				return *this;
			}

		}
		// at this point we're always at the root
		prev_visited = node;
		return *this;
	}

	_Iterator_base operator++(int) {
		_Iterator_base old = *this;
		operator++();
		return old;
	}

protected:

	_Iterator_base(const _Iterator_base& other) = default;
	_Iterator_base(node_type* node, concat_type concat) : node(node), concat(concat), prev_visited(nullptr) {}
	_Iterator_base& operator=(const _Iterator_base& other) = default;

	void set_value() {
		assert(node->value.has_value());
		value_type val = value_type(concat.get_key(node), node->value.value());
		value_ptr = std::make_shared<value_type>(std::move(val));
	}

	node_type* node;
	std::shared_ptr<value_type> value_ptr;

private:
	node_type* prev_visited;
	concat_type concat;
};

template<typename T>
class _Tree_bidirectional_iterator : public _Iterator_base<T> {
private:
	using base_type   = typename _Iterator_base<T>;
	using node_type   = typename base_type::node_type;
	using key_type    = typename base_type::key_type;
	using concat_type = typename base_type::concat_type;
public:
	// iterator traits definitions
	using difference_type   = typename std::ptrdiff_t;
	using value_type        = typename base_type::value_type;
	using pointer           = typename value_type*;
	using reference         = typename value_type&;
	using iterator_category = typename std::bidirectional_iterator_tag;

	_Tree_bidirectional_iterator() noexcept : base_type() {}
	_Tree_bidirectional_iterator(const _Tree_bidirectional_iterator& other) : base_type(static_cast<const base_type&>(other)) {}
	_Tree_bidirectional_iterator& operator=(const _Tree_bidirectional_iterator& other) {
		return _Tree_bidirectional_iterator(other);
	}

	_Tree_bidirectional_iterator(node_type* node, concat_type concat) : base_type(node, concat) {}

};

} // namespace ltr

#endif // LTR_ITERATORS
