#pragma once

#ifndef LTR_NODE
#define LTR_NODE

#include <utility>
#include <memory>
#include <optional>

namespace ltr {

template<typename K,
		 typename V,
		 template <typename T> typename Alloc>
struct _Node {

	using allocator_type   = Alloc<_Node>;
	using allocator_traits = std::allocator_traits<allocator_type>;
	using key_type         = K;
	using value_type       = V;

	static allocator_type node_allocator;

	_Node* parent, * child, * prev, * next;
	K key;
	std::optional<value_type> value;

	constexpr _Node() noexcept(noexcept(K())) : parent(nullptr), prev(nullptr), next(nullptr), child(nullptr), key() {}
	constexpr _Node(_Node&& other) = delete;

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

	constexpr _Node& operator=(const _Node& other) = delete;
	constexpr _Node& operator=(_Node&& other) = delete;

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

		other->parent = this->parent;
		other->next = this;
		this->prev = other;
	}

	// function to remove this node and all nodes whose
	// only purpose was being a branch to this node
	void remove_branch() {
		_Node* top = this;
		this->value.reset();
		// traverse towards root until found a node with having a sibling or a value
		while (top->parent && !(top->prev) && !(top->next) && !(top->value.has_value()))
			top = top->parent;

		// set subtree-to-delete as copy's child to set them to be deleted at end of scope
		_Node copy;
		// if top has a value only delete its children
		if (top->value.has_value()) {
			copy.child = top->child;
			top->child = nullptr;
		}
		// has right sibling
		else if (top->next) {
			// if top has 2 siblings, remove top from between
			if (top->prev) {
				top->prev->next = top->next;
				top->next->prev = top->prev;
				top->prev = nullptr;
			}
			// if only rightside sibling is present, set parent's child to it
			else {
				top->parent->child = top->next;
			}
			top->next = nullptr;
			copy.child = top;
		}
		// only left sibling
		else if(top->prev) {
			top->prev->next = nullptr;
			top->prev = nullptr;
			copy.child = top;
		}
		// top is root - can only occur if there was only 1 value present, hence top->child is always the node we came from
		else {
			copy.child = top->child;
		}
	}

}; // struct _Node

// instantiate static allocator
template<typename K,
		 typename V,
		 template<typename T> typename Alloc>
typename _Node<K, V, Alloc>::allocator_type _Node<K, V, Alloc>::node_allocator;

} // namespace ltr

#endif // LTR_NODE
