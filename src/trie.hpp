#pragma once

#ifndef LTR_TRIE
#define LTR_TRIE

#include <utility>
#include <string>
#include <type_traits>
#include "node.hpp"

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
	using key_type = typename Seq<K, Traits<K>, Alloc<K>>;
	using value_type = typename std::pair<const key_type, V>;
	using key_compare = typename Comp<K>;
	using allocator_type = typename Alloc<value_type>;
	using key_concat = typename Concat_expr_t;
	using node_type = typename _Node<K, V>;

	trie() = delete;

	constexpr trie(const trie& other) = default;

	constexpr trie(trie&& other) = default;

	trie(const key_concat& concat_expr) : _concat_expr(concat_expr), _root(new node_type()) {}

	~trie() {
		delete _root;
	}

	trie& operator=(const trie& other) = default;

	trie& operator=(trie&& other) = default;

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

	key_concat _concat_expr;
	node_type* _root;
};

}	// namespace ltr

#endif // LTR_TRIE