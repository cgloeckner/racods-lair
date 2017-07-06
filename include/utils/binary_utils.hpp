#pragma once
#include <array>
#include <vector>
#include <SFML/Network/Packet.hpp>

#include <utils/enum_map.hpp>

namespace utils {

sf::Packet loadBinaryFile(std::string const & filename);
void saveBinaryFile(sf::Packet const & stream, std::string const & filename);

// --------------------------------------------------------------------

template <typename T, std::size_t N, typename Func>
void parse(sf::Packet& stream, std::array<T, N>& vector, Func lambda);

template <typename T, typename Func>
void parse(sf::Packet& stream, std::vector<T>& vector, Func lambda);

template <typename E, typename T, typename Func>
void parse(sf::Packet& stream, utils::EnumMap<E, T>& map, Func lambda);

// --------------------------------------------------------------------

template <typename T, std::size_t N, typename Func>
void dump(sf::Packet& stream, std::array<T, N> const & vector, Func lambda);

template <typename T, typename Func>
void dump(sf::Packet& stream, std::vector<T> const & vector, Func lambda);

template <typename E, typename T, typename Func>
void dump(sf::Packet& stream, utils::EnumMap<E, T> const & map, Func lambda);

} // ::utils

// include implementation details
#include <utils/binary_utils.inl>
