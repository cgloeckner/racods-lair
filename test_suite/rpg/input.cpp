#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/input.hpp>

struct InputFixture {
	sf::Texture dummy_texture;
	core::LogContext log;
	core::InputSender input_sender;
	rpg::ActionSender action_sender;

	core::DungeonSystem dungeon;
	core::MovementManager movement;
	core::FocusManager focus;
	rpg::InputManager input;

	rpg::input_impl::Context context;

	core::ObjectID actor;
	rpg::InputData& data;

	InputFixture()
		: dummy_texture{}
		, log{}
		, input_sender{}
		, action_sender{}
		, dungeon{}
		, movement{}
		, focus{}
		, input{}
		, context{log, input_sender, action_sender, dungeon, movement, focus}
		, actor{1u}
		, data{input.acquire(actor)} {
		// setup scene
		sf::Vector2u grid_size{4u, 4u};
		auto scene =
			dungeon.create(dummy_texture, grid_size, sf::Vector2f{1.f, 1.f});
		auto& d = dungeon[scene];
		for (auto y = 0u; y < grid_size.y; ++y) {
			for (auto x = 0u; x < grid_size.x; ++x) {
				auto& cell = d.getCell({x, y});
				if (x == 0u || x == grid_size.x - 1u || y == 0u ||
					y == grid_size.y - 1u) {
					cell.terrain = core::Terrain::Wall;
				} else {
					cell.terrain = core::Terrain::Floor;
				}
			}
		}

		// setup actor
		auto& move = movement.acquire(1u);
		move.pos = {1.f, 1.f};
		move.last_pos = move.pos;
		move.scene = scene;
		move.look = {1.f, 0.f};
		d.getCell({1u, 1u}).entities.push_back(1u);
		auto& f = focus.acquire(1u);
		// connect gamepad
		sf::Event event;
		std::size_t gamepad_id = 0u;
		event.type = sf::Event::JoystickConnected;
		event.joystickConnect.joystickId = gamepad_id;
		context.mapper.pushEvent(event);
		// setup gamepad controls
		data.keys.set(rpg::PlayerAction::Attack, {gamepad_id, 0u});
		data.keys.set(rpg::PlayerAction::Interact, {gamepad_id, 1u});
		data.keys.set(rpg::PlayerAction::UseSlot, {gamepad_id, 2u});
		data.keys.set(rpg::PlayerAction::PrevSlot, {gamepad_id, 3u});
		data.keys.set(rpg::PlayerAction::NextSlot, {gamepad_id, 4u});
		data.keys.set(rpg::PlayerAction::Pause, {gamepad_id, 5u});
		data.keys.set(rpg::PlayerAction::ToggleAutoLook, {gamepad_id, 6u});
		data.keys.set(rpg::PlayerAction::MoveN,
			{gamepad_id, sf::Joystick::Axis::Y, -25.f});
		data.keys.set(rpg::PlayerAction::MoveS,
			{gamepad_id, sf::Joystick::Axis::Y, 25.f});
		data.keys.set(rpg::PlayerAction::MoveW,
			{gamepad_id, sf::Joystick::Axis::X, -25.f});
		data.keys.set(rpg::PlayerAction::MoveE,
			{gamepad_id, sf::Joystick::Axis::X, 25.f});
		data.keys.set(rpg::PlayerAction::LookN,
			{gamepad_id, sf::Joystick::Axis::U, -25.f});
		data.keys.set(rpg::PlayerAction::LookS,
			{gamepad_id, sf::Joystick::Axis::U, 25.f});
		data.keys.set(rpg::PlayerAction::LookW,
			{gamepad_id, sf::Joystick::Axis::V, -25.f});
		data.keys.set(rpg::PlayerAction::LookE,
			{gamepad_id, sf::Joystick::Axis::V, 25.f});
	}

	void reset() {
		data.is_active = true;
		data.auto_look = true;
		data.cooldown = sf::Time::Zero;
		// reset event senders
		input_sender.clear();
		action_sender.clear();
		// reset input state and reconnect actor's gamepad
		context.mapper = utils::InputMapper{};
		sf::Event event;
		event.type = sf::Event::JoystickConnected;
		event.joystickConnect.joystickId = 0u;
		context.mapper.pushEvent(event);
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}

	void set(std::size_t gamepad_id, sf::Joystick::Axis axis, float threshold) {
		sf::Event event;
		event.type = sf::Event::JoystickMoved;
		event.joystickMove.joystickId = gamepad_id;
		event.joystickMove.axis = axis;
		event.joystickMove.position = threshold;
		context.mapper.pushEvent(event);
	}

	void set(std::size_t gamepad_id, std::size_t button, bool pressed) {
		sf::Event event;
		if (pressed) {
			event.type = sf::Event::JoystickButtonPressed;
		} else {
			event.type = sf::Event::JoystickButtonReleased;
		}
		event.joystickButton.joystickId = gamepad_id;
		event.joystickButton.button = button;
		context.mapper.pushEvent(event);
	}
};

BOOST_AUTO_TEST_SUITE(input_test)

BOOST_AUTO_TEST_CASE(gamepad_can_trigger_movement) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger input
	fix.set(0, sf::Joystick::Axis::X, -30.f);
	fix.set(0, sf::Joystick::Axis::Y, 30.f);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect move+look to (-1,1) without an action
	BOOST_CHECK(
		action.action == rpg::PlayerAction::ToggleAutoLook);  // aka idle
	BOOST_CHECK_VECTOR_CLOSE(event.move, sf::Vector2f(-1.f, 1.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(event.look, event.move, 0.0001f);
}

BOOST_AUTO_TEST_CASE(gamepad_can_trigger_looking) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger input
	fix.set(0, sf::Joystick::Axis::U, 30.f);
	fix.set(0, sf::Joystick::Axis::V, 30.f);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect look to (1,1) without a move or action
	BOOST_CHECK(
		action.action == rpg::PlayerAction::ToggleAutoLook);  // aka idle
	BOOST_CHECK_VECTOR_CLOSE(event.move, sf::Vector2f(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(event.look, sf::Vector2f(1.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(gamepad_can_trigger_strife) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger strife
	fix.set(0, sf::Joystick::Axis::X, -30.f);
	fix.set(0, sf::Joystick::Axis::Y, 30.f);
	fix.set(0, sf::Joystick::Axis::U, 30.f);
	fix.set(0, sf::Joystick::Axis::V, 30.f);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect move to (-1,1) and look to (1,1) without an action
	BOOST_CHECK(
		action.action == rpg::PlayerAction::ToggleAutoLook);  // aka idle
	BOOST_CHECK_VECTOR_EQUAL(event.move, sf::Vector2i(-1, 1));
	BOOST_CHECK_VECTOR_EQUAL(event.look, sf::Vector2i(1, 1));
}

BOOST_AUTO_TEST_CASE(gamepad_can_trigger_strife_without_explicitly_looking) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	fix.data.auto_look = false;
	// trigger strife
	fix.set(0, sf::Joystick::Axis::X, -30.f);
	fix.set(0, sf::Joystick::Axis::Y, 30.f);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect move to (-1,1) and no look or action
	BOOST_CHECK(
		action.action == rpg::PlayerAction::ToggleAutoLook);  // aka idle
	BOOST_CHECK_VECTOR_EQUAL(event.move, sf::Vector2i(-1, 1));
	auto const & move = fix.movement.query(fix.data.id);
	BOOST_CHECK_VECTOR_CLOSE(event.look, move.look, 0.0001f);
}

BOOST_AUTO_TEST_CASE(gamepad_can_trigger_pause) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger pause
	fix.set(0, 5u, true);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect pause but nothing else
	BOOST_CHECK(action.action == rpg::PlayerAction::Pause);
	BOOST_CHECK_VECTOR_EQUAL(event.move, sf::Vector2f());
	auto const & move = fix.movement.query(fix.data.id);
	BOOST_CHECK_VECTOR_CLOSE(event.look, move.look, 0.0001f);
}

BOOST_AUTO_TEST_CASE(gamepad_can_trigger_interact) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger interaction
	fix.set(0, 1u, true);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect interaction but nothing else
	BOOST_CHECK(action.action == rpg::PlayerAction::Interact);
	BOOST_CHECK_VECTOR_EQUAL(event.move, sf::Vector2i());
	auto const & move = fix.movement.query(fix.data.id);
	BOOST_CHECK_VECTOR_CLOSE(event.look, move.look, 0.0001f);
}

BOOST_AUTO_TEST_CASE(gamepad_can_disable_autolook) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger interaction
	fix.set(0, 6u, true);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect pure idle but disabled auto_look
	BOOST_CHECK(
		action.action == rpg::PlayerAction::ToggleAutoLook);  // aka idle
	BOOST_CHECK_VECTOR_EQUAL(event.move, sf::Vector2i());
	auto const & move = fix.movement.query(fix.data.id);
	BOOST_CHECK_VECTOR_CLOSE(event.look, move.look, 0.0001f);
	BOOST_CHECK(!fix.data.auto_look);
}

BOOST_AUTO_TEST_CASE(gamepad_can_enable_autolook) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	fix.data.auto_look = false;
	// trigger interaction
	fix.set(0, 6u, true);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect pure idle but enabled auto_look
	BOOST_CHECK(
		action.action == rpg::PlayerAction::ToggleAutoLook);  // aka idle
	BOOST_CHECK_VECTOR_EQUAL(event.move, sf::Vector2i());
	auto const & move = fix.movement.query(fix.data.id);
	BOOST_CHECK_VECTOR_CLOSE(event.look, move.look, 0.0001f);
	BOOST_CHECK(fix.data.auto_look);
}

BOOST_AUTO_TEST_CASE(gamepad_can_trigger_strife_and_attack) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger strife and attack
	fix.set(0, sf::Joystick::Axis::X, -30.f);
	fix.set(0, sf::Joystick::Axis::Y, 30.f);
	fix.set(0, sf::Joystick::Axis::U, 30.f);
	fix.set(0, sf::Joystick::Axis::V, 30.f);
	fix.set(0, 0u, true);
	// query input
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect move to (-1,1), look to (1,1) and attack
	BOOST_CHECK(action.action == rpg::PlayerAction::Attack);
	BOOST_CHECK_VECTOR_EQUAL(event.move, sf::Vector2i(-1, 1));
	BOOST_CHECK_VECTOR_EQUAL(event.look, sf::Vector2i(1, 1));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(move_is_forwarded) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger movement
	fix.set(0, sf::Joystick::Axis::X, 30.f);
	// trigger object update
	rpg::input_impl::updateInput(fix.context, fix.data, sf::milliseconds(50));
	// expect an outgoing event
	auto const& events = fix.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
}

BOOST_AUTO_TEST_CASE(look_is_forwarded) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger look
	fix.set(0u, sf::Joystick::Axis::U, 30.f);
	// trigger object update
	rpg::input_impl::updateInput(fix.context, fix.data, sf::milliseconds(50));
	// expect an outgoing event
	auto const& events = fix.input_sender.data();
	BOOST_CHECK_EQUAL(events.size(), 1u);
}

BOOST_AUTO_TEST_CASE(action_is_forwarded) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger movement
	fix.set(0u, sf::Joystick::Axis::X, 30.f);
	// trigger object update
	rpg::input_impl::updateInput(fix.context, fix.data, sf::milliseconds(50));
	// expect an outgoing event
	auto const& events = fix.action_sender.data();
	BOOST_CHECK_EQUAL(events.size(), 1u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(facing_without_moving_does_not_disable_autolook_state) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	// trigger looking
	fix.set(0u, sf::Joystick::Axis::U, -30.f);
	// trigger object update
	core::InputEvent event;
	rpg::ActionEvent action;
	rpg::input_impl::queryInput(fix.context, fix.data, event, action);
	// expect looking to (0,-1)
	BOOST_CHECK_VECTOR_EQUAL(event.look, sf::Vector2i(0, -1));
	// expect auto_look to be already enabled
	BOOST_CHECK(fix.data.auto_look);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(
	adjustMovement_will_rotate_movevector_clockwise_if_necessary_and_possible) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	sf::Vector2f move{1.f, -0.f};
	rpg::input_impl::adjustMovement(fix.context, fix.data, move);
	BOOST_CHECK_VECTOR_CLOSE(move, sf::Vector2f(1.f, 0.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	adjustMovement_will_rotate_movevector_counterclockwise_if_necessary_and_possible) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	sf::Vector2f move{-1.f, 1.f};
	rpg::input_impl::adjustMovement(fix.context, fix.data, move);
	BOOST_CHECK_VECTOR_CLOSE(move, sf::Vector2f(0.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(adjustMovement_will_drop_movevector_if_impossible) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	sf::Vector2f move{0.f, -1.f};
	rpg::input_impl::adjustMovement(fix.context, fix.data, move);
	BOOST_CHECK_VECTOR_CLOSE(move, sf::Vector2f(), 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(input_is_disabled_on_death) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	BOOST_REQUIRE(fix.data.is_active);
	rpg::input_impl::onDeath(fix.data);
	BOOST_REQUIRE(!fix.data.is_active);
}

BOOST_AUTO_TEST_CASE(disabled_input_can_forward_pause_action) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	rpg::input_impl::onDeath(fix.data);
	fix.set(0u, 5u, true);
	rpg::input_impl::updateInput(fix.context, fix.data, sf::Time::Zero);
	BOOST_CHECK_EQUAL(fix.action_sender.data().size(), 1u);
}

BOOST_AUTO_TEST_CASE(disabled_input_cannot_forward_any_action_but_pause) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	rpg::input_impl::onDeath(fix.data);
	fix.set(0u, 0u, true); // attack
	rpg::input_impl::updateInput(fix.context, fix.data, sf::Time::Zero);
	BOOST_CHECK(fix.action_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(disabled_input_cannot_forward_movement) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	rpg::input_impl::onDeath(fix.data);
	fix.set(0u, sf::Joystick::Axis::Y, 100.f);
	rpg::input_impl::updateInput(fix.context, fix.data, sf::Time::Zero);
	BOOST_CHECK(fix.input_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(disabled_input_cannot_forward_looking) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	rpg::input_impl::onDeath(fix.data);
	fix.set(0u, sf::Joystick::Axis::U, 100.f);
	rpg::input_impl::updateInput(fix.context, fix.data, sf::Time::Zero);
	BOOST_CHECK(fix.input_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(input_is_enabled_on_respawn) {
	auto& fix = Singleton<InputFixture>::get();
	fix.reset();

	fix.data.is_active = false;
	rpg::input_impl::onSpawn(fix.data);
	BOOST_REQUIRE(fix.data.is_active);
}

BOOST_AUTO_TEST_SUITE_END()
