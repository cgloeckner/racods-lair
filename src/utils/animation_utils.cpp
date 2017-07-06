#include <utils/assert.hpp>
#include <utils/animation_utils.hpp>

namespace utils {

Frame::Frame() : clip{}, origin{}, duration{} {}

Frame::Frame(sf::IntRect const& clip, sf::Vector2f const& origin,
	sf::Time const& duration)
	: clip{clip}, origin{origin}, duration{duration} {}

ActionFrames::ActionFrames() : frames{}, duration{sf::Time::Zero} {}

void ActionFrames::append(sf::IntRect const& clip, sf::Vector2f const& origin,
	sf::Time const& duration) {
	frames.emplace_back(clip, origin, duration);
}

void ActionFrames::refresh() {
	auto total = sf::Time::Zero;
	for (auto const& frame : frames) {
		total += frame.duration;
	}
	duration = total;
}

// ---------------------------------------------------------------------------

ActionState::ActionState() : elapsed{sf::Time::Zero}, index{0u} {}

IntervalState::IntervalState() : IntervalState{-1.f} {}

IntervalState::IntervalState(float current)
	: current{current}
	, min{0.f}
	, max{1.f}
	, speed{0.001f}
	, rise{false}
	, repeat{0} {}

// ---------------------------------------------------------------------------

bool updateActionState(ActionState& state, ActionFrames const& frames,
	sf::Time const& elapsed, bool& was_updated) {
	bool overflow{false};
	was_updated = false;
	state.elapsed += elapsed;
	while (state.elapsed >= frames.frames[state.index].duration) {
		ASSERT(frames.frames[state.index].duration > sf::Time::Zero);
		was_updated = true;
		state.elapsed -= frames.frames[state.index].duration;
		++state.index;
		if (state.index >= frames.frames.size()) {
			state.index = 0u;
			overflow = true;
		}
	}
	return overflow;
}

bool updateInterval(
	IntervalState& state, sf::Time const& elapsed, bool& was_updated) {
	ASSERT(state.speed > 0.f);
	ASSERT(state.min < state.max);

	was_updated = false;
	if (state.repeat == 0) {
		// nothing to do
		return false;
	}
	was_updated = true;

	bool finished{false};
	auto delta = state.speed * elapsed.asMilliseconds();
	if (state.rise) {
		state.current += delta;
		if (state.current >= state.max) {
			state.current = state.max;
			finished = true;
		}
	} else {
		state.current -= delta;
		if (state.current <= state.min) {
			state.current = state.min;
			finished = true;
		}
	}

	if (finished) {
		state.rise = !state.rise;
		if (state.repeat > 0) {
			--state.repeat;
		}
	}

	return finished;
}

}  // ::utils
