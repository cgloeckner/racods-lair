#pragma once
#include <algorithm>
#include <stdexcept>
#include <Thor/Math.hpp>

namespace utils {

template <typename T>
sf::Rect<T> enlarge(sf::Rect<T> const & lhs, sf::Rect<T> const & rhs) {
	sf::Vector2<T> topleft, bottomright;
	topleft.x = std::min(lhs.left, rhs.left);
	topleft.y = std::min(lhs.top, rhs.top);
	bottomright.x = std::max(lhs.left + lhs.width, rhs.left + rhs.width);
	bottomright.y = std::max(lhs.top + lhs.height, rhs.top + rhs.height);
	return {topleft.x, topleft.y, bottomright.x - topleft.x, bottomright.y - topleft.y};
}

// ---------------------------------------------------------------------------

template <typename C>
void reverse(C& container) {
	std::reverse(std::begin(container), std::end(container));
}

template <typename T>
void shuffle(std::vector<T>& container) {
	auto n = container.size();
	for (auto k = 0u; k < n; ++k) {
		auto i = thor::random(0u, n - 1u);
		auto j = thor::random(0u, n - 1u);
		std::swap(container[i], container[j]);
	}
}

// ---------------------------------------------------------------------------

template <typename C, typename T>
auto find(C& container, T const& elem) -> decltype(std::begin(container)) {
	return std::find(std::begin(container), std::end(container), elem);
}

template <typename C, typename Pred>
auto find_if(C& container, Pred pred) -> decltype(std::begin(container)) {
	return std::find_if(std::begin(container), std::end(container), pred);
}

template <typename T>
std::size_t find_index(std::vector<T> const& container, T const& elem) {
	return std::distance(std::begin(container), find(container, elem));
}

// ---------------------------------------------------------------------------

template <typename T>
bool pop(std::vector<T>& container, typename std::vector<T>::iterator i,
	bool stable) {
	auto end = std::end(container);
	if (i == end) {
		return false;
	}
	// move element to container's back
	if (stable) {
		// swap hand over hand
		auto j = i;
		while (++j != end) {
			std::swap(*i, *j);
			++i;
		}
	} else {
		// swap immediately
		auto last = std::prev(end);
		std::swap(*i, *last);
	}
	container.pop_back();
	return true;
}

template <typename T>
bool pop(std::vector<T>& container, T const& elem, bool stable) {
	return pop(container, find(container, elem), stable);
}

template <typename T, typename Pred>
bool pop_if(std::vector<T>& container, Pred pred, bool stable) {
	return pop(container, find_if(container, pred), stable);
}

// ---------------------------------------------------------------------------

template <typename C, typename T>
bool contains(C const & container, T const& elem) {
	return find(container, elem) != end(container);
}

// ---------------------------------------------------------------------------

template <typename T>
void append(std::vector<T>& target, std::vector<T> const& source) {
	target.reserve(target.size() + source.size());
	for (auto const& elem : source) {
		target.push_back(elem);
	}
}

// ---------------------------------------------------------------------------

template <typename T, typename Pred>
void remove_if(T& container, Pred pred) {
	auto begin = std::begin(container);
	auto new_end = std::remove_if(begin, std::end(container), pred);
	auto new_size = std::distance(begin, new_end);
	container.resize(new_size);
}

// --------------------------------------------------------------------

template <typename Handle>
void split(std::string const & str, std::string const & token, Handle func) {
	std::size_t pos{0u}, prev{0u};
	do {
		pos = str.find(token, prev);
		func(str.substr(prev, pos-prev));
		prev = pos + token.size();
	} while (pos != std::string::npos);
}

// --------------------------------------------------------------------

template <typename T>
T& randomAt(std::vector<T>& container) {
	return container[thor::random(0u, container.size()-1u)];
}

template <typename T>
T const & randomAt(std::vector<T> const & container) {
	return container[thor::random(0u, container.size()-1u)];
}

}  // ::utils
