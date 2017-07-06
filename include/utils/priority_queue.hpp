#pragma once
#include <algorithm>

namespace utils {

// min-heap sorted
// H requires ctor(); size_t range() const; size_t operator()(T val) const;
template <typename T, typename K, typename H>
class PriorityQueue {
  private:
	struct Node {
		T value;
		K key;

		Node();
		Node(T&& value, K&& key);
	};

	Node* data;
	std::size_t* lookup;
	std::size_t size;
	H func;

	void bubble_up(std::size_t index);
	void bubble_down(std::size_t index);
	void update_lookup(std::size_t index);
	std::size_t query_lookup(T const& value) const;

  public:
	PriorityQueue(H&& func);
	~PriorityQueue();

	bool empty() const;
	void insert(T value, K key);
	T extract();
	void decrease(T const& value, K key);
	void clear();
};

}  // ::utils

// include implementation details
#include <utils/priority_queue.inl>
