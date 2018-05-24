#include <utils/algorithm.hpp>
#include <utils/logger.hpp>

namespace utils {

std::string to_string(sf::Time const & time) {
	return std::to_string(time.asMilliseconds()) + "ms";
}

std::string to_string(sf::VideoMode const & mode) {
	return std::to_string(mode.width) + "x" + std::to_string(mode.height)
		+ ":" + std::to_string(mode.bitsPerPixel);
}

// --------------------------------------------------------------------

void Logger::add(std::ostream& stream) {
	streams.push_back(&stream);
}

void Logger::remove(std::ostream const & stream) {
	// workaround: cannot pop pointer directly because of const (-.-)
	pop_if(streams, [&stream](std::ostream* other) {
		return other == &stream;
	});
}

void Logger::clear() {
	streams.clear();
}

void Logger::flush() {
	for (auto& ptr : streams) {
		ptr->flush();
	}
}

Logger::iterator Logger::begin() {
	return streams.begin();
}

Logger::iterator Logger::end() {
	return streams.end();
}

} // ::utils

// --------------------------------------------------------------------

std::ostream& operator<<(std::ostream& lhs, sf::Color const& rhs) {
	return lhs << thor::toString(rhs);
}

std::ostream& operator<<(std::ostream& lhs, sf::Keyboard::Key const& rhs) {
	return lhs << thor::toString(rhs);
}

std::ostream& operator<<(std::ostream& lhs, sf::Mouse::Button const& rhs) {
	return lhs << thor::toString(rhs);
}

std::ostream& operator<<(std::ostream& lhs, sf::Joystick::Axis const& rhs) {
	return lhs << thor::toString(rhs);
}

std::ostream& operator<<(std::ostream& lhs, sf::Time const& rhs) {
	return lhs << utils::to_string(rhs);
}

std::ostream& operator<<(std::ostream& lhs, sf::VideoMode const& rhs) {
	return lhs << utils::to_string(rhs);
}
