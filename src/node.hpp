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

	using allocator_traits = typename std::allocator_traits<Alloc<_Node>>;
	static Alloc<_Node> node_allocator;


	_Node* parent;
	K key;
	std::optional<V> value;
	_Node* prev;
	_Node* next;
	_Node* child;


	//root node's gonna have a key, which will have to be accounted for
	constexpr _Node() noexcept : parent(nullptr), prev(nullptr), next(nullptr), child(nullptr), key() {}

	constexpr _Node(_Node&& other) = default;

	// recursively create a deep copy of the node's subtree
	_Node(const _Node& other) : key(other.key), value(other.value) {
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

	_Node(const K& key, const V& value) : key(key), value(std::in_place, value), parent(nullptr),
										  prev(nullptr), next(nullptr), child(nullptr) {}

	constexpr _Node(K&& key, V&& value): key(std::exchange(key, 0)), value(std::in_place, std::move(value)), parent(nullptr),
										 prev(nullptr), next(nullptr), child(nullptr) {
	}


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

	constexpr _Node& operator=(_Node&& other) = default;

	// new overload for convenient use
	void* operator new (std::size_t size) {
		void* p = allocator_traits::allocate(node_allocator, 1);
		return p;
	}


	_Node& operator=(const _Node& other) {
		return _Node(other);
	}

	constexpr _Node& operator=(_Node&& other) = default;

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
		child = other;
	}

	void set_next (_Node* other) {
		if (next) {
			other->next = this->next;
			this->next->prev = other;
		}
		other.parent = parent;
		next = other;
	}

	// doesn't check if has element, returns this node if key not found
	_Node& find_child (const K& key) {
		if (_Node* n = child) {
			while (n != nullptr) {
				if (n->key == key)
					return n;
				n = n->next;
			}
		}
		return *this;
	}

};

// instantiate static allocator
template<typename K,
		 typename V,
		 template<typename T> typename Alloc>
Alloc<typename _Node<K, V, Alloc>> _Node<K, V, Alloc>::node_allocator;

} // namespace ltr


#endif // LTR_NODE