#pragma once
#include <utils/enum_map.hpp>
#include <utils/input_mapper.hpp>

namespace utils {

template <typename Action>
class Keybinding {
	template <typename T>
	friend bool operator==(Keybinding<T> const& lhs, Keybinding<T> const& rhs);
	template <typename T>
	friend bool operator!=(Keybinding<T> const& lhs, Keybinding<T> const& rhs);

  private:
	EnumMap<Action, InputAction> map;

  public:
	bool isUsed(InputAction input) const;
	InputAction const& get(Action action) const;
	Action const& get(InputAction input) const;
	void set(Action action, InputAction input);
	
	/// Returns either -1 or the gamepad's id
	int getGamepadId() const;
	
	/// Apply gamepad id to all actions
	/// @pre keybinding is already using a gamepad
	void setGamepadId(unsigned int id);

	/// Return all input actions that are used twice (or more often)
	std::vector<InputAction> getAmbiguousActions() const;

	/// Return all input actions that are used by both bindingss
	std::vector<InputAction> getCollisions(Keybinding const& other) const;
	
	/// Ignores gamepad ids
	bool isSimilar(Keybinding<Action> const & other) const;
};

// ---------------------------------------------------------------------------

template <typename Action>
bool operator==(Keybinding<Action> const& lhs, Keybinding<Action> const& rhs);

template <typename Action>
bool operator!=(Keybinding<Action> const& lhs, Keybinding<Action> const& rhs);

}  // ::utils

// include implementation details
#include <utils/keybinding.inl>
