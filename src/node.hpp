#pragma once

#ifndef LIB_TRIE_NODE
#define LIB_TRIE_NODE

#include <utility>

template<typename K,
		 typename V,
		 typename Key_alloc>
struct _Node {
	static Key_alloc alloc;
	_Node* parent;
	K* key;
	std::optional<V> value;
	_Node* next;
	_Node* child;

	constexpr _Node() noexcept : parent(nullptr), next(nullptr), child(nullptr), key(nullptr) {}

	// recursively create a deep copy of the node's subtree
	_Node(const _Node& other) : value(other.value) {
		key = alloc.allocate(1);
		*key = other->key;
		if (other.child) {
			_Node* prev_child = new _Node(*(other.child));
			insert_child(*prev_child);

			while (_Node* n = other.child->next) {
				_Node* copy = new _Node(*n);
				prev_child->insert_next_sibling(*copy);
				prev_child = copy;
				n = n->next;
			}
		}
	}

	constexpr _Node(_Node&& other) = default;

	_Node(const K& key, const V& value) : _Node(), value(value) {
		this->key = alloc.allocate(1);
		*(this->key) = key;
	}

	_Node& operator=(const _Node& other) {
		return _Node(other);
	}

	constexpr _Node& operator=(_Node&& other) = default;

	//constexpr _Node(K&& key, V&& value): _Node(), key(Key_alloc(key)), value(std::in_place, std::move(value)) {}

	// recursively destroy this node and its subtree
	~_Node() {
		while (_Node* n = child) {
			alloc.deallocate(key, 1);
			_Node* tmp = n->next;
			delete n;
			n = tmp;
		}
	}

	// should only be used if no children are present
	void insert_child(_Node& other) {
		static_assert(child);
		other.parent = this;
		child = &other;
	}

	void insert_next_sibling(_Node& other) {
		if (next) {
			other.next = this->next;
		}
		other.parent = parent;
		next = &other;
	}

	// doesn't check if has element, returns this node if key not found
	_Node& find_child(const K& key) {
		if (child) {
			while (_Node* n = child) {
				if (*(n->key) == key)
					return *n;
				n = n->next;
			}
		}
		return *this;
	}
};

template<typename K,
		 typename V,
		 typename Key_alloc>
Key_alloc _Node<K, V, Key_alloc>::alloc = Key_alloc();

#endif // LIB_TRIE_NODE