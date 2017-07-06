#include <stdexcept>
#include <iostream>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

namespace utils {

template <typename Id>
IdManager<Id>::IdManager(std::size_t n)
	: n{n}
	, i{1u}
	, recent{}
	, unused{} {
	recent.reserve(n / 100);
	unused.reserve(n);
}

template <typename Id>
Id IdManager<Id>::acquire() {
	if (unused.empty()) {
		if (i > n) {
			throw std::bad_alloc{};
		}
		return i++;
	}
	auto id = unused.back();
	unused.pop_back();
	return id;
}

template <typename Id>
void IdManager<Id>::release(Id id) {
	recent.push_back(id);
}

template <typename Id>
void IdManager<Id>::cleanup() {
	append(unused, recent);
	recent.clear();
}

template <typename Id>
void IdManager<Id>::reset() {
	i = 1u;
	recent.clear();
	unused.clear();
}

// ---------------------------------------------------------------------------

template <typename Id, typename T>
ComponentSystem<Id, T>::ComponentSystem(std::size_t n)
	: BaseSystem<Id>{}
	, n{n}
	, data{}
	, lookup{}
	, unused{} {
	data.reserve(n + 1);
	data.emplace_back();
	lookup.resize(n + 1, 0u);
}

template <typename Id, typename T>
ComponentSystem<Id, T>::~ComponentSystem() {}

template <typename Id, typename T>
T& ComponentSystem<Id, T>::acquire(Id id) {
	if (data.size() == n + 1) {
		throw std::bad_alloc{};
	}
	ASSERT(id > 0u);
	ASSERT(id <= n);
	ASSERT(lookup[id] == 0u);
	data.emplace_back();
	data.back().id = id;
	lookup[id] = data.size() - 1u;
	return data.back();
}

template <typename Id, typename T>
void ComponentSystem<Id, T>::release(Id id) {
	ASSERT(id > 0u);
	ASSERT(id <= n);
	ASSERT(lookup[id] > 0u);
	unused.push_back(id);
}

template <typename Id, typename T>
bool ComponentSystem<Id, T>::has(Id id) const {
	ASSERT(id > 0u);
	ASSERT(id <= n);
	return lookup[id] > 0u;
}

template <typename Id, typename T>
T const& ComponentSystem<Id, T>::query(Id id) const {
	ASSERT(id > 0u);
	ASSERT(id <= n);
	auto index = lookup[id];
	ASSERT(index > 0u);
	return data[index];
}

template <typename Id, typename T>
T& ComponentSystem<Id, T>::query(Id id) {
	ASSERT(id > 0u);
	ASSERT(id <= n);
	auto index = lookup[id];
	ASSERT(index > 0u);
	return data[index];
}

template <typename Id, typename T>
void ComponentSystem<Id, T>::tryRelease(Id id) {
	if (has(id)) {
		release(id);
	}
}

template <typename Id, typename T>
void ComponentSystem<Id, T>::cleanup() {
	for (auto& id : unused) {
		// look up indices
		auto index = lookup[id];
		if (index == 0u) {
			// id not used
			continue;
		}
		auto other = data.back().id;
		auto last = lookup[other];
		// replace with last element
		data[index] = std::move(data[last]);
		data.pop_back();
		// update lookup table
		lookup[other] = index;
		lookup[id] = 0u;
	}
	unused.clear();
}

template <typename Id, typename T>
std::size_t ComponentSystem<Id, T>::size() const {
	return data.size() - 1u;
}

template <typename Id, typename T>
std::size_t ComponentSystem<Id, T>::capacity() const {
	return n;
}

template <typename Id, typename T>
typename ComponentSystem<Id, T>::container::iterator ComponentSystem<Id, T>::begin() {
	return ++data.begin();
}

template <typename Id, typename T>
typename ComponentSystem<Id, T>::container::iterator ComponentSystem<Id, T>::end() {
	return data.end();
}

template <typename Id, typename T>
typename ComponentSystem<Id, T>::container::const_iterator ComponentSystem<Id, T>::begin() const {
	return ++data.begin();
}

template <typename Id, typename T>
typename ComponentSystem<Id, T>::container::const_iterator ComponentSystem<Id, T>::end() const {
	return data.end();
}

}  // ::utils
