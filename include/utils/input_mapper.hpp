#pragma once
#include <array>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <utils/enum_map.hpp>

SET_ENUM_LIMITS(sf::Joystick::Axis::X, sf::Joystick::Axis::PovY);
SET_ENUM_LIMITS(sf::Keyboard::Key::A, sf::Keyboard::Key::Pause);

namespace utils {

// A generalized input action
struct InputAction {
	enum { Key, Button, Axis } type;

	struct KeyState {
		sf::Keyboard::Key key;
	};

	struct ButtonState {
		std::size_t gamepad_id, button;
	};

	struct AxisState {
		std::size_t gamepad_id;
		sf::Joystick::Axis axis;
		float threshold;
	};

	union {
		KeyState key;
		ButtonState button;
		AxisState axis;
	};

	InputAction() = default;
	InputAction(sf::Keyboard::Key key);
	InputAction(
		std::size_t gamepad_id, sf::Joystick::Axis axis, float threshold);
	InputAction(std::size_t gamepad_id, std::size_t button);
	
	InputAction(sf::Event::JoystickMoveEvent const & move);
	InputAction(sf::Event::JoystickButtonEvent const & button);
	
	std::string toString() const;
	
	// ignores gamepad id and absolute value threshold (sign only)
	bool isSimilar(InputAction const & other) const;
};

bool operator==(InputAction const& lhs, InputAction const& rhs);
bool operator!=(InputAction const& lhs, InputAction const& rhs);

// ---------------------------------------------------------------------------

struct Gamepad {
	bool connected;
	EnumMap<sf::Joystick::Axis, float> axis;
	std::array<bool, sf::Joystick::ButtonCount> buttons;

	Gamepad();
};

// Holds the entire input state of this machine
class InputMapper {
  private:
	EnumMap<sf::Keyboard::Key, bool> keyboard;
	std::array<Gamepad, sf::Joystick::Count> gamepad;

	void handle(sf::Event::KeyEvent const& event, bool pressed);
	void handle(sf::Event::JoystickButtonEvent const& event, bool pressed);
	void handle(sf::Event::JoystickMoveEvent const& event);
	void handle(sf::Event::JoystickConnectEvent const& event, bool connected);

  public:
	InputMapper();

	void pushEvent(sf::Event const& event);
	void reset(InputAction const & input);
	bool isActive(InputAction const& input) const;
};

}  // ::utils
