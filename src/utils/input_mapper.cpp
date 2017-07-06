#include <cmath>
#include <Thor/Input.hpp>

#include <utils/input_mapper.hpp>

namespace utils {

Gamepad::Gamepad() : connected{false}, axis{}, buttons{} {}

// ---------------------------------------------------------------------------

InputAction::InputAction(sf::Keyboard::Key key) : type{InputAction::Key} {
	this->key.key = key;
}

InputAction::InputAction(
	std::size_t gamepad_id, sf::Joystick::Axis axis, float threshold)
	: type{InputAction::Axis} {
	this->axis.gamepad_id = gamepad_id;
	this->axis.axis = axis;
	this->axis.threshold = threshold;
}

InputAction::InputAction(std::size_t gamepad_id, std::size_t button)
	: type{InputAction::Button} {
	this->button.gamepad_id = gamepad_id;
	this->button.button = button;
}

InputAction::InputAction(sf::Event::JoystickMoveEvent const & move)
	: InputAction{move.joystickId, move.axis, move.position} {
}

InputAction::InputAction(sf::Event::JoystickButtonEvent const & button)
	: InputAction{button.joystickId, button.button} {
}

std::string InputAction::toString() const {
	switch (type) {
		case InputAction::Key:
			return thor::toString(key.key);
		
		case InputAction::Axis:
			return thor::toString(axis.axis) + (axis.threshold < 0 ? "-" : "+");
			
		case InputAction::Button:
			return std::to_string(button.button);
	}
}

bool InputAction::isSimilar(InputAction const & other) const {
	if (type != other.type) {
		return false;
	}
	switch (type) {
		case InputAction::Key:
			return key.key == other.key.key;
		case InputAction::Axis:
			return axis.axis == other.axis.axis &&
				std::signbit(axis.threshold) == std::signbit(other.axis.threshold);
		case InputAction::Button:
			return button.button == other.button.button;
	}
}

bool operator==(InputAction const& lhs, InputAction const& rhs) {
	if (lhs.type != rhs.type) {
		return false;
	}
	switch (lhs.type) {
		case InputAction::Key:
			return lhs.key.key == rhs.key.key;
		case InputAction::Axis:
			return lhs.axis.gamepad_id == rhs.axis.gamepad_id &&
				   lhs.axis.axis == rhs.axis.axis &&
				   lhs.axis.threshold == rhs.axis.threshold;
		case InputAction::Button:
			return lhs.button.gamepad_id == rhs.button.gamepad_id &&
				   lhs.button.button == rhs.button.button;
	}
}

bool operator!=(InputAction const& lhs, InputAction const& rhs) {
	if (lhs.type != rhs.type) {
		return true;
	}
	switch (lhs.type) {
		case InputAction::Key:
			return lhs.key.key != rhs.key.key;
		case InputAction::Axis:
			return lhs.axis.gamepad_id != rhs.axis.gamepad_id ||
				   lhs.axis.axis != rhs.axis.axis ||
				   lhs.axis.threshold != rhs.axis.threshold;
		case InputAction::Button:
			return lhs.button.gamepad_id != rhs.button.gamepad_id ||
				   lhs.button.button != rhs.button.button;
	}
}

// ---------------------------------------------------------------------------

InputMapper::InputMapper() : keyboard{}, gamepad{} {
	// query connected gamepads
	for (auto i = 0u; i < sf::Joystick::Count; ++i) {
		gamepad[i].connected = sf::Joystick::isConnected(i);
		/*
		if (gamepad[i].connected) {
			auto ident = sf::Joystick::getIdentification(i);
			log << "Gamepad #0 '" << ident.name << "' was detected"
				<< std::endl;
		}
		*/
	}
}

void InputMapper::handle(sf::Event::KeyEvent const& event, bool pressed) {
	if (event.code == -1) {
		// ignore unsupported key
		return;
	}
	keyboard[event.code] = pressed;
}

void InputMapper::handle(
	sf::Event::JoystickButtonEvent const& event, bool pressed) {
	gamepad[event.joystickId].buttons[event.button] = pressed;
}

void InputMapper::handle(sf::Event::JoystickMoveEvent const& event) {
	gamepad[event.joystickId].axis[event.axis] = event.position;
}

void InputMapper::handle(
	sf::Event::JoystickConnectEvent const& event, bool connected) {
	/*
	log << "Gamepad #" << event.joystickId << " was ";
	if (!connected) {
		log << "dis";
	}
	log << "connected" << std::endl;
	*/

	// reset gamepad and set connection state
	gamepad[event.joystickId] = Gamepad();
	gamepad[event.joystickId].connected = connected;
}

void InputMapper::reset(InputAction const & input) {
	switch (input.type) {
		case InputAction::Key:
			keyboard[input.key.key] = false;
			break;

		case InputAction::Button:
			gamepad[input.button.gamepad_id].buttons[input.button.button] = false;
			break;

		case InputAction::Axis:
			gamepad[input.axis.gamepad_id].axis[input.axis.axis] = 0.f;
			break;
	}
}

bool InputMapper::isActive(InputAction const& input) const {
	// check whether state is active
	bool active{false};
	switch (input.type) {
		case InputAction::Key:
			active = keyboard[input.key.key];
			break;

		case InputAction::Button: {
			auto const& pad = gamepad[input.button.gamepad_id];
			if (pad.connected) {
				active = pad.buttons[input.button.button];
			}
		} break;

		case InputAction::Axis: {
			auto const& pad = gamepad[input.axis.gamepad_id];
			if (pad.connected) {
				float value = pad.axis[input.axis.axis];
				if (value * input.axis.threshold > 0) {
					// active = thresholds have same sign
					active = std::abs(value) >= std::abs(input.axis.threshold);
				}
			}
		} break;
	}
	return active;
}

void InputMapper::pushEvent(sf::Event const& event) {
	switch (event.type) {
		case sf::Event::KeyPressed:
			handle(event.key, true);
			break;

		case sf::Event::KeyReleased:
			handle(event.key, false);
			break;

		case sf::Event::JoystickButtonPressed:
			handle(event.joystickButton, true);
			break;

		case sf::Event::JoystickButtonReleased:
			handle(event.joystickButton, false);
			break;

		case sf::Event::JoystickMoved:
			handle(event.joystickMove);
			break;

		case sf::Event::JoystickConnected:
			handle(event.joystickConnect, true);
			break;

		case sf::Event::JoystickDisconnected:
			handle(event.joystickConnect, false);
			break;

		default:
			// nothing to do
			break;
	}
}

}  // ::utils
