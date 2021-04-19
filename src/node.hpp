#pragma once

#ifndef LIB_TRIE_NODE
#define LIB_TRIE_NODE

#include <utility>

template<typename K,
		 typename V>
struct _Node {
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

	_Node& operator=(const _Node& other) {
		return _Node(other);
	}

	constexpr _Node& operator=(_Node&& other) = default;

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

#endif // LIB_TRIE_NODE