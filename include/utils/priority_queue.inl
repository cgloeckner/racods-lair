namespace utils {

template <typename T, typename K, typename H>
PriorityQueue<T, K, H>::PriorityQueue(H&& func)
	: data{nullptr}, lookup{nullptr}, size{0u}, func{std::move(func)} {
	data = new Node[func.range()]{};
	lookup = new std::size_t[func.range()]{};
}

template <typename T, typename K, typename H>
PriorityQueue<T, K, H>::~PriorityQueue() {
	delete[] data;
	delete[] lookup;
}

template <typename T, typename K, typename H>
PriorityQueue<T, K, H>::Node::Node()
	: value{}, key{} {}

template <typename T, typename K, typename H>
PriorityQueue<T, K, H>::Node::Node(T&& value, K&& key)
	: value{value}, key{key} {}

template <typename T, typename K, typename H>
void PriorityQueue<T, K, H>::bubble_up(std::size_t index) {
	// remove dangling element
	Node elem = std::move(data[index]);

	std::size_t parent = index / 2u;
	while (index > 0u && data[parent].key > elem.key) {
		// move parent to child position
		data[index] = std::move(data[parent]);
		update_lookup(index);
		index = parent;
		parent /= 2u;
	}

	// place dangling element
	data[index] = std::move(elem);
	update_lookup(index);
}

template <typename T, typename K, typename H>
void PriorityQueue<T, K, H>::bubble_down(std::size_t index) {
	Node elem = std::move(data[index]);

	while (index < size) {
		// determine greater children's index
		std::size_t child = index * 2u;
		if (child + 1u < size && data[child].key > data[child + 1u].key) {
			++child;
		}
		// that children becomes parent if heap condition violated
		if (child < size && elem.key > data[child].key) {
			data[index] = std::move(data[child]);
			update_lookup(index);
			index = child;
		} else {
			// finished
			break;
		}
	}

	data[index] = std::move(elem);
	update_lookup(index);
}

template <typename T, typename K, typename H>
void PriorityQueue<T, K, H>::update_lookup(std::size_t index) {
	lookup[func(data[index].value)] = index;
}

template <typename T, typename K, typename H>
std::size_t PriorityQueue<T, K, H>::query_lookup(T const& value) const {
	return lookup[func(value)];
}

template <typename T, typename K, typename H>
bool PriorityQueue<T, K, H>::empty() const {
	return size == 0u;
}

template <typename T, typename K, typename H>
void PriorityQueue<T, K, H>::insert(T value, K key) {
	// insert data at the end
	data[size] = Node{std::move(value), std::move(key)};
	bubble_up(size++);
}

template <typename T, typename K, typename H>
T PriorityQueue<T, K, H>::extract() {
	// extract data
	auto value = data[0].value;
	// replace with last element
	data[0] = std::move(data[--size]);
	bubble_down(0u);
	return value;
}

template <typename T, typename K, typename H>
void PriorityQueue<T, K, H>::decrease(T const& value, K key) {
	// decrease key
	auto index = query_lookup(value);
	data[index].key = std::move(key);
	bubble_up(index);
}

template <typename T, typename K, typename H>
void PriorityQueue<T, K, H>::clear() {
	size = 0u;
	for (std::size_t i = 0u; i < func.range(); ++i) {
		lookup[i] = 0u;
	}
}

}  // ::utils
