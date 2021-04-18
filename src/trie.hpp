#pragma once

#ifndef LIB_TRIE_TRIE
#define LIB_TRIE_TRIE

#include <utility>
#include <string>
#include <type_traits>
#include "node.hpp"

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
	using node_type = typename _Node<K, V, typename key_type::allocator_type>;

	trie() = delete;

	constexpr trie(const trie& other) = default;

	constexpr trie(trie&& other) = default;

	trie(const key_concat& concat_expr) : _concat_expr(concat_expr) {}

	trie& operator=(const trie& other) = default;

	trie& operator=(trie&& other) = default;


private:
	key_concat _concat_expr;
	node_type _root;
};

#endif // LIB_TRIE_TRIE