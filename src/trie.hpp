#pragma once

#ifndef LTR_TRIE
#define LTR_TRIE

#include <utility>
#include <string>
#include <memory>
#include <type_traits>
#include <functional>

#include "node.hpp"
#include "iterators.hpp"

namespace ltr {

template<typename K,
		 typename V,
		 typename Concat_expr_t,
		 template<typename T> typename Comp = std::less,
		 template<typename SeqT, typename SeqTraits, typename SeqAlloc> typename Seq = std::basic_string,
		 template<typename T> typename Traits = std::char_traits,
		 template<typename T> typename Alloc = std::allocator>
class trie {
public:
	using key_type       = typename Seq<K, Traits<K>, Alloc<K>>;
	using mapped_type    = typename V;
	using value_type     = typename std::pair<const key_type, mapped_type>;
	using key_compare    = typename Comp<K>;
	using key_concat     = typename Concat_expr_t;
	using concat_type    = typename std::function<key_type& (key_type&, K)>;
	using node_type      = typename _Node<K, V, Alloc>;
	using iterator       = typename _Tree_bidirectional_iterator<trie>;

	constexpr trie() noexcept = delete;
	trie(const trie& other) : _concat(other._concat), _root(new node_type(*(other._root))) {}
	trie(trie&& other) = default;
	trie& operator=(trie && other) = default;

	trie& operator=(const trie& other) {
		return trie(other);
	}

	trie(const key_concat& concat) : _concat(concat), _root(new node_type()) {
		_root->value.emplace(42);
	}

	~trie() {
		delete _root;
	}

	iterator begin() {
		iterator it = end();
		++it;
		return it;
	}

	iterator end() {
		return iterator(_root, _concat);
	}


private:

	concat_type _concat;
	node_type* _root;
};

} // namespace ltr

#endif // LTR_TRIE
