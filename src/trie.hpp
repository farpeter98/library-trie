#pragma once

#ifndef LTR_TRIE
#define LTR_TRIE

// from c++20 captureless lambdas are default constructible
// but std::is_default_constructible still evaluated to false
// hence the macro workaround
// also visual studio 
#if __cplusplus > 201703L 
#define ISCPP20 true
#else
#define ISCPP20 false
#include <functional>
#endif

#include <utility>
#include <string>
#include <memory>
#include <type_traits>
#include <iostream>

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
private:
	// forward declare concater struct
	template<bool>
	struct key_concater;
public:
	using key_type       = typename Seq<K, Traits<K>, Alloc<K>>;
	using mapped_type    = typename V;
	using value_type     = typename std::pair<const key_type, mapped_type>;
	using key_compare    = typename Comp<K>;
	using key_concat     = typename Concat_expr_t;
	using concat_type    = typename key_concater<ISCPP20>;
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
		iterator it(_root, _concat);
		++it;
		return it;
	}

	iterator end() {
		return iterator(_root, _concat);
	}

	iterator rbegin() {
		iterator it(_root, _concat);
		--it;
		return it;
	}

	iterator rend() {
		return iterator(_root, _concat);
	}

private:
	// template struct with a condition based on whether the lambda is default constructible
	template<>
	struct key_concater<true> {
		// if default constructible, no need to store as member
		constexpr key_concater() noexcept = default;
		constexpr key_concater(const key_concat&) noexcept {}

		key_type operator()(node_type* node) const {
			node_type* current = node;
			key_type key;
			while ((current->parent) != nullptr) {
				key = key_concat{}(key, current->key);
				current = current->parent;
			}
			return key;
		}
	};
	template<>
	struct key_concater<false> {
		using func_type = typename std::function<key_type& (key_type&, K)>;
		// if default constructible, no need to store as member
		key_concater() noexcept = default;
		key_concater(const key_concat& concat) : concat(concat) {}

		key_type operator()(node_type* node) const {
			node_type* current = node;
			key_type key;
			while ((current->parent) != nullptr) {
				key = concat(key, current->key);
				current = current->parent;
			}
			return key;
		}
		func_type concat;
	};

	concat_type _concat;
	node_type* _root;
};

} // namespace ltr

#endif // LTR_TRIE
