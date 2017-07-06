#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

namespace utils {

template <typename Action>
bool Keybinding<Action>::isUsed(InputAction input) const {
	for (auto const& pair : map) {
		if (pair.second == input) {
			return true;
		}
	}
	return false;
}

template <typename Action>
InputAction const& Keybinding<Action>::get(Action action) const {
	return map[action];
}

template <typename Action>
Action const& Keybinding<Action>::get(InputAction input) const {
	for (auto const& pair : map) {
		if (pair.second == input) {
			return pair.first;
		}
	}
	throw std::out_of_range("input action not bound");
}

template <typename Action>
void Keybinding<Action>::set(Action action, InputAction input) {
	map[action] = input;
}

template <typename Action>
int Keybinding<Action>::getGamepadId() const {
	// note: if gamepad is used, all actions share the same gamepad
	auto& pair = *map.begin();
	switch (pair.second.type) {
		case utils::InputAction::Button:
			return static_cast<int>(pair.second.button.gamepad_id);
			
		case utils::InputAction::Axis:
			return static_cast<int>(pair.second.axis.gamepad_id);
			
		default:
			return -1;
	}
}

template <typename Action>
void Keybinding<Action>::setGamepadId(unsigned int id) {
	bool first{true};
	for (auto& pair: map) {
		switch (pair.second.type) {
			case utils::InputAction::Button:
				pair.second.button.gamepad_id = id;
				break;
				
			case utils::InputAction::Axis:
				pair.second.axis.gamepad_id = id;
				break;
				
			default:
				// note: if gamepad-related this is never reached
				// but if keyboard-related this is reached at the first iteration
				ASSERT(first);
				return;
		}
		first = false;
	}
}

template <typename Action>
std::vector<InputAction> Keybinding<Action>::getAmbiguousActions() const {
	std::vector<InputAction> used, invalid;
	used.reserve(map.size());
	invalid.reserve(map.size());
	for (auto const& pair : map) {
		if (pair.second == InputAction{}) {
			// not assigned
			continue;
		}
		if (contains(used, pair.second)) {
			invalid.push_back(pair.second);
		} else {
			used.push_back(pair.second);
		}
	}
	return invalid;
}

template <typename Action>
std::vector<InputAction> Keybinding<Action>::getCollisions(
	Keybinding const& other) const {
	std::vector<InputAction> shared;
	if (map.begin()->second.type != InputAction::Key) {
		// ignore gamepad layouts
		return shared;
	}
	shared.reserve(map.size());
	for (auto const& pair : map) {
		if (pair.second == InputAction{}) {
			// not assigned
			continue;
		}
		if (other.isUsed(pair.second)) {
			shared.push_back(pair.second);
		}
	}
	return shared;
}

template <typename Action>
bool Keybinding<Action>::isSimilar(Keybinding<Action> const & other) const {
	for (auto const & pair: map) {
		auto const & rhs = other.map[pair.first];
		if (!pair.second.isSimilar(rhs)) {
			// different layout
			return false;
		}
	}
	return true;
}

// ---------------------------------------------------------------------------

template <typename Action>
bool operator==(Keybinding<Action> const& lhs, Keybinding<Action> const& rhs) {
	return lhs.map == rhs.map;
}

template <typename Action>
bool operator!=(Keybinding<Action> const& lhs, Keybinding<Action> const& rhs) {
	return lhs.map != rhs.map;
}

}  // ::utils
