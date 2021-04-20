#pragma once

#ifndef LTR_ITERATORS
#define LTR_ITERATORS

#include "node.hpp"
#include <utility>
#include <iterator>

namespace ltr {

// type of related trie
template<typename T>
class _Tree_bidirectional_iterator {
private:
	using trie_type   = typename T;
	using node_type   = typename trie_type::node_type;
	using key_type    = typename trie_type::key_type;
	using mapped_type = typename trie_type::mapped_type;
	using key_concat  = typename trie_type::key_concat;
public:
	// iterator traits definitions
	using difference_type   = typename std::ptrdiff_t;
	using value_type        = typename trie_type::value_type;
	using pointer           = typename value_type*;
	using reference         = typename value_type&;
	using iterator_category = typename std::bidirectional_iterator_tag;

	_Tree_bidirectional_iterator() = default;
	_Tree_bidirectional_iterator(const _Tree_bidirectional_iterator& other) = default;
	_Tree_bidirectional_iterator(_Tree_bidirectional_iterator&& other) = default;
	_Tree_bidirectional_iterator& operator=(const _Tree_bidirectional_iterator& other) = default;
	_Tree_bidirectional_iterator& operator=(_Tree_bidirectional_iterator&& other) = default;

	_Tree_bidirectional_iterator(node_type* node) : _node(node) {}

	/*reference operator*() const {
		return *_val;
	}*/

	/*pointer operator->() {
		return &_val;
	}*/

	// depth-first tree traversal for first element with value
	_Tree_bidirectional_iterator& operator++() {
		while (_node->child)
			_node = _node->child;
	}

private:
	void create_value() {
		// TODO create pair based on the current node
		//_val = value_type();
	}

	void find_next_node() {

	}

	node_type* _node;
	//value_type _val;
	//key_concat _concat;
};

} // namespace ltr

#endif // LTR_ITERATORS
