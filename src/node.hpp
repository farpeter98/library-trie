#pragma once

#ifndef LTR_NODE
#define LTR_NODE

#include <utility>
#include <memory>

namespace ltr {

template<typename K,
		 typename V,
		 template <typename T> typename Alloc>
struct _Node {

	using allocator_type   = typename Alloc<_Node>;
	using allocator_traits = typename std::allocator_traits<allocator_type>;
	using key_type         = typename K;
	using value_type       = typename V;

	static allocator_type node_allocator;

	_Node* parent, * child, * prev, * next;
	K key;
	std::optional<value_type> value;

	constexpr _Node() noexcept(noexcept(K())) : parent(nullptr), prev(nullptr), next(nullptr), child(nullptr), key() {}
	constexpr _Node(_Node&& other) noexcept = default;

	// recursively create a deep copy of the node's subtree
	_Node(const _Node& other) : key(other.key), value(other.value), parent(nullptr),
		                        prev(nullptr), next(nullptr), child(nullptr)
	{
		if (other.child) {
			_Node* prev_child = new _Node(*(other.child));
			set_child(prev_child);

			_Node* n = other.child->next;
			while (n != nullptr) {
				_Node* copy = new _Node(*n);
				prev_child->set_next(copy);
				prev_child = copy;
				n = n->next;
			}
		}
	}

	constexpr _Node(const K& key) : key(key), value(), parent(nullptr),
		                            prev(nullptr), next(nullptr), child(nullptr) {}

	constexpr _Node(const K& key, const value_type& value) : key(key), value(std::in_place, value), parent(nullptr),
										                     prev(nullptr), next(nullptr), child(nullptr) {}

	constexpr _Node(K&& key, value_type&& value): key(std::exchange(key, 0)), value(std::in_place, std::move(value)), parent(nullptr),
										          prev(nullptr), next(nullptr), child(nullptr) {}

	// recursively destroy this node and its subtree
	~_Node() {
		_Node* n = child;
		while (n != nullptr) {
			_Node* tmp = n->next;
			delete n;
			n = tmp;
		}
	}


	_Node& operator=(const _Node& other) {
		return _Node(other);
	}

	constexpr _Node& operator=(_Node&& other) noexcept = default;

	// new overload for convenient use
	void* operator new (std::size_t size) {
		void* p = allocator_traits::allocate(node_allocator, 1);
		return p;
	}

	// delete overload for convenient use
	void operator delete (void * p) {
		allocator_traits::deallocate(node_allocator, static_cast<_Node*>(p), 1);
	}

	// should only be used if no children are present
	void set_child (_Node* other) {
		assert(!child);
		other->parent = this;
		this->child = other;
	}

	void set_next (_Node* other) {
		if (next) {
			this->next->prev = other;
			other->next = this->next;
		}
		other->prev = this;
		this->next = other;
		other->parent = this->parent;
	}

	void set_prev(_Node* other) {
		if (prev) {
			this->prev->next = other;
			other->prev = this->prev;
		}
		else if (parent)
			this->parent->child = other;

		other->next = this;
		this->prev = other;
	}

}; // struct _Node

// instantiate static allocator
template<typename K,
		 typename V,
		 template<typename T> typename Alloc>
_Node<K, V, Alloc>::allocator_type _Node<K, V, Alloc>::node_allocator;

} // namespace ltr

#endif // LTR_NODE
