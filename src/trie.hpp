#pragma once

#ifndef LTR_TRIE
#define LTR_TRIE

#include <utility>
#include <string>
#include <memory>
#include <type_traits>
#include "node.hpp"
#include "iterators.hpp"

namespace ltr {

template<typename K,
		 typename V,
		 typename Concat_expr_t,
		 template<typename T> typename Comp = std::less,
		 template<typename T, typename Traits_t, typename Alloc_t> typename Seq = std::basic_string,
		 template<typename T> typename Traits = std::char_traits,
		 template<typename T> typename Alloc = std::allocator>
class trie {
public:
	using key_type    = typename Seq<K, Traits<K>, Alloc<K>>;
	using mapped_type = typename V;
	using value_type  = typename std::pair<const key_type, mapped_type>;
	using key_compare = typename Comp<K>;
	using key_concat  = typename Concat_expr_t;
	using node_type   = typename _Node<K, V, Alloc>;
	using iterator    = typename _Tree_bidirectional_iterator<trie>;

	trie() = delete;
	constexpr trie(const trie& other) : _concat_expr(other._concat_expr), _root(new node_type(*(other._root))) {}
	constexpr trie(trie&& other) = default;
	constexpr trie& operator=(trie && other) = default;

	trie& operator=(const trie& other) {
		return trie(other);
	}

	trie(const key_concat& concat_expr) : _concat_expr(concat_expr), _root(new node_type()) {}

	~trie() {
		delete _root;
	}

private:
	const key_type& _Get_key(node_type* node) const {
		node_type* current = node;
		key_type key;
		while (current != _root) {
			key = _concat_expr(key, current->key);
			current = current->parent;
		}
		return key;
	}

	const node_type& _Find_node(const key_type& key) const {

	}


	key_concat _concat_expr;
	node_type* _root;
};

} // namespace ltr

#endif // LTR_TRIE
