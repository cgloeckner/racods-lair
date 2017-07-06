#include <stdexcept>

namespace utils {

template <typename T, std::size_t N, typename Func>
void parse(sf::Packet& stream, std::array<T, N>& array, Func lambda) {
	for (auto& elem: array) {
		if (stream.endOfPacket()) {
			throw std::runtime_error{"Unexpected end of binary stream"};
		}
		lambda(elem);
	}
}

template <typename T, typename Func>
void parse(sf::Packet& stream, std::vector<T>& vector, Func lambda) {
	sf::Uint64 n;
	stream >> n;
	vector.clear();
	vector.resize(n);
	for (auto& elem: vector) {
		if (stream.endOfPacket()) {
			throw std::runtime_error{"Unexpected end of binary stream"};
		}
		lambda(elem);
	}
}

template <typename E, typename T, typename Func>
void parse(sf::Packet& stream, utils::EnumMap<E, T>& map, Func lambda) {
	for (auto& pair: map) {
		if (stream.endOfPacket()) {
			throw std::runtime_error{"Unexpected end of binary stream"};
		}
		lambda(pair.second);
	}
}

// --------------------------------------------------------------------

template <typename T, std::size_t N, typename Func>
void dump(sf::Packet& stream, std::array<T, N> const & array, Func lambda) {
	for (auto const & elem: array) {
		lambda(elem);
	}
}

template <typename T, typename Func>
void dump(sf::Packet& stream, std::vector<T> const & vector, Func lambda) {
	stream << static_cast<sf::Uint64>(vector.size());
	for (auto const & elem: vector) {
		lambda(elem);
	}
}

template <typename E, typename T, typename Func>
void dump(sf::Packet& stream, utils::EnumMap<E, T> const & map, Func lambda) {
	for (auto const & pair: map) {
		lambda(pair.second);
	}
}

} // ::utils
