#pragma once
#include <vector>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <utils/enum_map.hpp>

namespace utils {

struct Frame {
	sf::IntRect clip;
	sf::Vector2f origin;
	sf::Time duration;

	Frame();
	Frame(sf::IntRect const& clip, sf::Vector2f const& origin,
		sf::Time const& duration);
};

struct ActionFrames {
	std::vector<Frame> frames;
	sf::Time duration;  // total

	ActionFrames();

	/// Will emplace a new frame
	void append(sf::IntRect const& clip, sf::Vector2f const& origin,
		sf::Time const& duration);

	/// This will update the total duration as the sum of each frames' duration
	void refresh();
};

struct ActionState {
	sf::Time elapsed;
	std::size_t index;

	ActionState();
};

struct IntervalState {
	float current, min, max, speed;
	bool rise;
	int repeat;  // -1: loop, 0: how often

	IntervalState();
	IntervalState(float current);
};

// ---------------------------------------------------------------------------

/// Update a single action's animation state
/**
 *	@return true if action was animated until it's end
 */
bool updateActionState(ActionState& state, ActionFrames const& frames,
	sf::Time const& elapsed, bool& was_updated);

/// Update the interval animation state
/**
 *	@pre state.speed > 0.f
 *	@pre state.min < state.max
 *	@return true if action was animated until min or max (depending whether
 *rising or not)
 */
bool updateInterval(
	IntervalState& state, sf::Time const& elapsed, bool& was_updated);

}  // ::utils
