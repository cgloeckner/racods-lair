#pragma once
#include <type_traits>
#include <memory>
#include <vector>
#include <string>
#include <future>
#include <SFML/Graphics/Rect.hpp>

namespace utils {

template <typename T, typename... Types>
struct pack_contains;

template <typename T, typename... Types>
struct pack_contains<T, T, Types...> : std::true_type {};

template <typename T, typename Head, typename... Types>
struct pack_contains<T, Head, Types...> : pack_contains<T, Types...> {};

template <typename T>
struct pack_contains<T> : std::false_type {};

// ---------------------------------------------------------------------------

template <typename T>
sf::Rect<T> enlarge(sf::Rect<T> const & lhs, sf::Rect<T> const & rhs);

// ---------------------------------------------------------------------------

template <typename C>
void reverse(C& container);

template <typename T>
void shuffle(std::vector<T>& container);

// ---------------------------------------------------------------------------

template <typename C, typename T>
auto find(C& container, T const& elem) -> decltype(std::begin(container));

template <typename C, typename Pred>
auto find_if(C& container, Pred pred) -> decltype(std::begin(container));

template <typename T>
std::size_t find_index(std::vector<T> const& container, T const& elem);

// ---------------------------------------------------------------------------

template <typename C, typename T>
bool contains(C const & container, T const& elem);

// ---------------------------------------------------------------------------

template <typename T>
bool pop(std::vector<T>& container, typename std::vector<T>::iterator i,
	bool stable = false);

template <typename T>
bool pop(std::vector<T>& container, T const& elem, bool stable = false);

template <typename T, typename Pred>
bool pop_if(std::vector<T>& container, Pred pred, bool stable = false);

// ---------------------------------------------------------------------------

template <typename T>
void append(std::vector<T>& target, std::vector<T> const& source);

// ---------------------------------------------------------------------------

template <typename T, typename Pred>
void remove_if(T& container, Pred pred);

// ---------------------------------------------------------------------------

template <typename Handle>
void split(std::string const & str, std::string const & token, Handle func);

// ---------------------------------------------------------------------------

template <typename T>
T& randomAt(std::vector<T>& container);

template <typename T>
T const & randomAt(std::vector<T> const & container);

// ---------------------------------------------------------------------------

/// Check whether the pathfinding is done
/// @param data TracerData to check for
/// @return true if path is available
template <typename T>
bool isReady(std::future<T> const & f);

} // ::utils

// include implementation details
#include <utils/algorithm.inl>
