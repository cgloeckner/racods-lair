#pragma once
#include <vector>
#include <functional>
#include <SFML/System/Time.hpp>

namespace utils {

template <typename T>
class DelaySystem {
  private:
	struct Node {
		T value;
		sf::Time delay;
	};

	std::vector<Node> wait;

  public:
	std::vector<T> ready;

	std::vector<Node> const& data() const;

	void reset();
	void push(T const& value, sf::Time const& delay);
	void operator()(sf::Time const& elapsed);
	void operator()(sf::Time const& elapsed, std::function<void(T const &)> handle);
};

}  // ::utils

// include implementation details
#include <utils/delay_system.inl>
