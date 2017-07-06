#include <boost/test/unit_test.hpp>

#include <utils/input_mapper.hpp>

void connectGamepad(utils::InputMapper& mapper, std::size_t gamepad_id) {
	sf::Event event;
	event.type = sf::Event::JoystickConnected;
	event.joystickConnect.joystickId = gamepad_id;
	mapper.pushEvent(event);
}

BOOST_AUTO_TEST_SUITE(InputMapper_test)

BOOST_AUTO_TEST_CASE(InputMapper_can_construct_InputAction_for_keyboard_key) {
	utils::InputAction input{sf::Keyboard::Key::Space};

	BOOST_REQUIRE_EQUAL(utils::InputAction::Key, input.type);
	BOOST_CHECK_EQUAL(sf::Keyboard::Key::Space, input.key.key);
}

BOOST_AUTO_TEST_CASE(InputMapper_can_construct_InputAction_for_gamepad_axis) {
	utils::InputAction input{3u, sf::Joystick::Axis::Y, -15.f};

	BOOST_REQUIRE_EQUAL(utils::InputAction::Axis, input.type);
	BOOST_CHECK_EQUAL(3u, input.axis.gamepad_id);
	BOOST_CHECK_EQUAL(sf::Joystick::Axis::Y, input.axis.axis);
	BOOST_CHECK_CLOSE(-15.f, input.axis.threshold, 0.0001f);
}

BOOST_AUTO_TEST_CASE(InputMapper_can_construct_InputAction_for_gamepad_button) {
	utils::InputAction input{3u, 2u};

	BOOST_REQUIRE_EQUAL(utils::InputAction::Button, input.type);
	BOOST_CHECK_EQUAL(3u, input.button.gamepad_id);
	BOOST_CHECK_EQUAL(2u, input.button.button);
}

BOOST_AUTO_TEST_CASE(InputMapper_can_compare_InputActions) {
	utils::InputAction btn{3u, 2u};
	utils::InputAction key{sf::Keyboard::Key::Space};
	utils::InputAction key2{sf::Keyboard::Key::Space};

	BOOST_CHECK(!(btn == key));
	BOOST_CHECK(btn == btn);
	BOOST_CHECK(key == key2);
}

BOOST_AUTO_TEST_CASE(InputMapper_KeyPressed_will_active_action) {
	sf::Event event;
	event.type = sf::Event::KeyPressed;
	event.key.code = sf::Keyboard::Key::Return;

	utils::InputAction action{sf::Keyboard::Key::Return};

	utils::InputMapper mapper;
	BOOST_REQUIRE(!mapper.isActive(action));
	mapper.pushEvent(event);
	BOOST_CHECK(mapper.isActive(action));
}

BOOST_AUTO_TEST_CASE(InputMapper_KeyReleased_will_deactivate_action) {
	sf::Event event;
	event.type = sf::Event::KeyPressed;
	event.key.code = sf::Keyboard::Key::Return;

	utils::InputAction action{sf::Keyboard::Key::Return};

	utils::InputMapper mapper;
	mapper.pushEvent(event);
	BOOST_REQUIRE(mapper.isActive(action));

	event.type = sf::Event::KeyReleased;
	mapper.pushEvent(event);
	BOOST_CHECK(!mapper.isActive(action));
}

BOOST_AUTO_TEST_CASE(
	InputMapper_too_little_JoystickMoved_will_not_active_action) {
	sf::Event event;
	event.type = sf::Event::JoystickMoved;
	event.joystickMove.joystickId = 2u;
	event.joystickMove.axis = sf::Joystick::Axis::Y;
	event.joystickMove.position = -5.f;

	utils::InputAction action{2u, sf::Joystick::Axis::Y, -15.f};
	utils::InputAction other{4u, sf::Joystick::Axis::Y, -15.f};

	utils::InputMapper mapper;
	connectGamepad(mapper, 2u);
	connectGamepad(mapper, 4u);
	BOOST_REQUIRE(!mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));
	mapper.pushEvent(event);
	BOOST_CHECK(!mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));  // other joystick won't be affected
}

BOOST_AUTO_TEST_CASE(
	InputMapper_strong_enough_JoystickMoved_will_activate_action) {
	sf::Event event;
	event.type = sf::Event::JoystickMoved;
	event.joystickMove.joystickId = 2u;
	event.joystickMove.axis = sf::Joystick::Axis::Y;
	event.joystickMove.position = -25.f;

	utils::InputAction action{2u, sf::Joystick::Axis::Y, -15.f};
	utils::InputAction other{4u, sf::Joystick::Axis::Y, -15.f};

	utils::InputMapper mapper;
	connectGamepad(mapper, 2u);
	connectGamepad(mapper, 4u);
	BOOST_REQUIRE(!mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));
	mapper.pushEvent(event);
	BOOST_CHECK(mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));  // other joystick won't be affected
}

BOOST_AUTO_TEST_CASE(
	InputMapper_nearly_nulled_JoystickMoved_will_deactivate_action) {
	sf::Event event;
	event.type = sf::Event::JoystickMoved;
	event.joystickMove.joystickId = 3u;
	event.joystickMove.axis = sf::Joystick::Axis::Y;
	event.joystickMove.position = -45.f;

	utils::InputAction action{3u, sf::Joystick::Axis::Y, -15.f};
	utils::InputAction other{4u, sf::Joystick::Axis::Y, -15.f};

	utils::InputMapper mapper;
	connectGamepad(mapper, 3u);
	connectGamepad(mapper, 4u);
	mapper.pushEvent(event);
	BOOST_REQUIRE(mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));
	event.joystickMove.position = 0.001f;
	mapper.pushEvent(event);
	BOOST_CHECK(!mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));  // other joystick won't be affected
}

BOOST_AUTO_TEST_CASE(InputMapper_JoystickButtonPressed_will_activate_action) {
	sf::Event event;
	event.type = sf::Event::JoystickButtonPressed;
	event.joystickButton.joystickId = 3u;
	event.joystickButton.button = 5u;

	utils::InputAction action{3u, 5u};
	utils::InputAction other{4u, 5u};

	utils::InputMapper mapper;
	connectGamepad(mapper, 3u);
	connectGamepad(mapper, 4u);
	BOOST_REQUIRE(!mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));
	mapper.pushEvent(event);
	BOOST_CHECK(mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));  // other joystick won't be affected
}

BOOST_AUTO_TEST_CASE(
	InputMapper_JoystickButtonReleased_will_deactivate_action) {
	sf::Event event;
	event.type = sf::Event::JoystickButtonPressed;
	event.joystickButton.joystickId = 3u;
	event.joystickButton.button = 5u;

	utils::InputAction action{3u, 5u};
	utils::InputAction other{4u, 5u};

	utils::InputMapper mapper;
	connectGamepad(mapper, 3u);
	connectGamepad(mapper, 4u);
	mapper.pushEvent(event);
	BOOST_REQUIRE(mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));

	event.type = sf::Event::JoystickButtonReleased;
	mapper.pushEvent(event);
	BOOST_CHECK(!mapper.isActive(action));
	BOOST_CHECK(!mapper.isActive(other));  // other joystick won't be affected
}

BOOST_AUTO_TEST_CASE(
	InputMapper_isActive_returns_false_if_gamepad_is_not_connected) {
	utils::InputMapper mapper;

	sf::Event event;
	event.type = sf::Event::JoystickButtonPressed;
	event.joystickButton.joystickId = 3u;
	event.joystickButton.button = 5u;
	mapper.pushEvent(event);

	event.type = sf::Event::JoystickMoved;
	event.joystickMove.joystickId = 3u;
	event.joystickMove.axis = sf::Joystick::Axis::Y;
	event.joystickMove.position = -25.f;
	mapper.pushEvent(event);

	utils::InputAction axis{3u, sf::Joystick::Axis::Y, -15.f};
	utils::InputAction btn{3u, 5u};

	BOOST_CHECK(!mapper.isActive(axis));
	BOOST_CHECK(!mapper.isActive(btn));
}

BOOST_AUTO_TEST_CASE(InputMapper_connecting_gamepad_resets_its_stae) {
	utils::InputMapper mapper;

	sf::Event event;
	event.type = sf::Event::JoystickButtonPressed;
	event.joystickButton.joystickId = 3u;
	event.joystickButton.button = 5u;
	mapper.pushEvent(event);

	event.type = sf::Event::JoystickMoved;
	event.joystickMove.joystickId = 3u;
	event.joystickMove.axis = sf::Joystick::Axis::Y;
	event.joystickMove.position = -25.f;
	mapper.pushEvent(event);

	utils::InputAction axis{3u, sf::Joystick::Axis::Y, -15.f};
	utils::InputAction btn{3u, 5u};

	connectGamepad(mapper, 3u);  // connect resets gamepad's state
	BOOST_CHECK(!mapper.isActive(axis));
	BOOST_CHECK(!mapper.isActive(btn));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(similar_actions_can_have_different_gamepad_ids) {
	utils::InputAction lhs{2u, sf::Joystick::Axis::Y, -15.f};
	utils::InputAction rhs{0u, sf::Joystick::Axis::Y, -15.f};
	BOOST_CHECK(lhs.isSimilar(rhs));
}

BOOST_AUTO_TEST_SUITE_END()
