#pragma once
#include <ostream>
#include <ctime>
#include <SFML/System/Time.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <Thor/Input.hpp>
#include <Thor/Graphics.hpp>

namespace utils {

// helper to get current date and time as string
struct {
	inline std::string operator()() const {
		// create a string with the current date and time
		time_t rawtime;
		struct tm* timeinfo;
		char buffer[80];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 80, "%X %x ", timeinfo);
		return std::string{buffer};
	}
} now;

// --------------------------------------------------------------------

std::string to_string(sf::Time const & time);
std::string to_string(sf::VideoMode const & mode);

// --------------------------------------------------------------------

class Logger {
  private:
	using container = std::vector<std::ostream*>;
	using iterator = container::iterator;
	
	container streams;

  public:
	void add(std::ostream& stream);
	void remove(std::ostream const & stream);
	void clear();
	void flush();
	
	iterator begin();
	iterator end();
};

} // ::utils

// --------------------------------------------------------------------

template <typename T>
utils::Logger& operator<<(utils::Logger& lhs, T const& rhs);

template <typename T>
std::ostream& operator<<(std::ostream& lhs, sf::Vector2<T> const& rhs);

template <typename T>
std::ostream& operator<<(std::ostream& lhs, sf::Vector3<T> const& rhs);

template <typename T>
std::ostream& operator<<(std::ostream& lhs, sf::Rect<T> const& rhs);

std::ostream& operator<<(std::ostream& lhs, sf::Color const& rhs);
std::ostream& operator<<(std::ostream& lhs, sf::Keyboard::Key const& rhs);
std::ostream& operator<<(std::ostream& lhs, sf::Mouse::Button const& rhs);
std::ostream& operator<<(std::ostream& lhs, sf::Joystick::Axis const& rhs);
std::ostream& operator<<(std::ostream& lhs, sf::Time const& rhs);
std::ostream& operator<<(std::ostream& lhs, sf::VideoMode const& rhs);

inline std::ostream& operator<<(std::ostream& lhs, decltype(utils::now) const& rhs) {
	return lhs << rhs();
}

// include implementation details
#include <utils/logger.inl>
