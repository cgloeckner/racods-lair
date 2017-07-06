#include <boost/test/unit_test.hpp>

#include <utils/keybinding.hpp>

enum class MyAction { Idle, Attack, Cast, Die };

SET_ENUM_LIMITS(MyAction::Idle, MyAction::Die);

using MyBind = utils::Keybinding<MyAction>;

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Keybinding_test)

BOOST_AUTO_TEST_CASE(gamepad_related_binding_returns_non_negative_gamepad_id) {
	MyBind bind;
	for (auto action: utils::EnumRange<MyAction>{}) {
		bind.set(action, {2u, 3u}); // gamepad 2, button 3
	}
	BOOST_CHECK_EQUAL(bind.getGamepadId(), 2);
}

BOOST_AUTO_TEST_CASE(keyboard_related_binding_returns_negative_gamepad_id) {
	MyBind bind;
	for (auto action: utils::EnumRange<MyAction>{}) {
		bind.set(action, sf::Keyboard::Space);
	}
	BOOST_CHECK_EQUAL(bind.getGamepadId(), -1);
}

BOOST_AUTO_TEST_CASE(gamepad_id_can_be_changed_for_gamepad_related_bindings) {
	MyBind bind;
	for (auto action: utils::EnumRange<MyAction>{}) {
		bind.set(action, {2u, 3u}); // gamepad 2, button 3
	}
	bind.setGamepadId(1u);
	BOOST_CHECK_EQUAL(bind.getGamepadId(), 1);
}

BOOST_AUTO_TEST_CASE(changing_gamepad_is_ignored_for_keyboard_related_bindings) {
	MyBind bind;
	for (auto action: utils::EnumRange<MyAction>{}) {
		bind.set(action, sf::Keyboard::Space);
	}
	bind.setGamepadId(1u);
	BOOST_CHECK_EQUAL(bind.getGamepadId(), -1);
}

BOOST_AUTO_TEST_CASE(changing_gamepad_causes_assertion_to_fail_for_mixed_bindings) {
	MyBind bind;
	for (auto action: utils::EnumRange<MyAction>{}) {
		bind.set(action, sf::Keyboard::Space);
	}
	bind.set(MyAction::Idle, {2u, 3u});
	BOOST_CHECK_ASSERT(bind.setGamepadId(1u));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Keybinding_isUsed_is_working) {
	MyBind bind;
	utils::InputAction space{sf::Keyboard::Key::Space};

	BOOST_CHECK(!bind.isUsed(space));

	bind.set(MyAction::Attack, space);

	BOOST_CHECK(bind.isUsed(space));
	BOOST_CHECK(space == bind.get(MyAction::Attack));
}

BOOST_AUTO_TEST_CASE(Keybinding_get_InputAction_throws_only_if_action_not_set) {
	MyBind bind;
	utils::InputAction space{sf::Keyboard::Key::Space};

	BOOST_CHECK_THROW(bind.get(space), std::out_of_range);

	bind.set(MyAction::Attack, space);

	BOOST_CHECK(MyAction::Attack == bind.get(space));
}

BOOST_AUTO_TEST_CASE(
	Keybinding_getCollision_returns_none_only_if_bindings_are_disjoint) {
	MyBind a, b;

	BOOST_CHECK(a.getCollisions(b).empty());
	BOOST_CHECK(b.getCollisions(a).empty());

	a.set(MyAction::Attack, sf::Keyboard::Key::Space);
	b.set(MyAction::Attack, sf::Keyboard::Key::Return);

	BOOST_CHECK(a.getCollisions(b).empty());
	BOOST_CHECK(b.getCollisions(a).empty());

	b.set(MyAction::Cast, sf::Keyboard::Key::Space);

	auto c1 = a.getCollisions(b);
	auto c2 = b.getCollisions(a);

	BOOST_REQUIRE_EQUAL(1u, c1.size());
	BOOST_CHECK(c1[0] == utils::InputAction(sf::Keyboard::Key::Space));
	BOOST_CHECK(c1 == c2);
}

BOOST_AUTO_TEST_CASE(
	Keybinding_getCollision_ignores_gamepad_layouts) {
	MyBind a, b;

	BOOST_CHECK(a.getCollisions(b).empty());
	BOOST_CHECK(b.getCollisions(a).empty());

	a.set(MyAction::Attack, sf::Keyboard::Key::Space);
	b.set(MyAction::Attack, {0u, 0u});

	BOOST_CHECK(a.getCollisions(b).empty());
	BOOST_CHECK(b.getCollisions(a).empty());
}

BOOST_AUTO_TEST_CASE(
	Keybinding_set_an_action_twice_will_only_keep_the_second_input) {
	MyBind bind;
	bind.set(MyAction::Cast, sf::Keyboard::Space);
	bind.set(MyAction::Cast, sf::Keyboard::Return);
	utils::InputAction ret{sf::Keyboard::Return};

	BOOST_CHECK(ret == bind.get(MyAction::Cast));
}

BOOST_AUTO_TEST_CASE(
	Keybinding_getAmbiguousActions_returns_those_inputs_that_are_used_more_than_one_time) {
	MyBind bind;
	BOOST_CHECK(bind.getAmbiguousActions().empty());

	utils::InputAction space{sf::Keyboard::Key::Space};
	bind.set(MyAction::Attack, space);
	BOOST_CHECK(bind.getAmbiguousActions().empty());

	bind.set(MyAction::Cast, space);
	auto ambiguous = bind.getAmbiguousActions();
	BOOST_REQUIRE_EQUAL(1u, ambiguous.size());
	BOOST_CHECK(ambiguous[0] == space);
}

BOOST_AUTO_TEST_CASE(Keybinding_are_equal_if_all_actions_equal) {
	MyBind lhs, rhs;

	lhs.set(MyAction::Attack, {sf::Keyboard::Key::Space});
	lhs.set(MyAction::Cast, {sf::Keyboard::Key::Return});
	rhs.set(MyAction::Attack, {sf::Keyboard::Key::Space});
	rhs.set(MyAction::Cast, {sf::Keyboard::Key::Return});

	BOOST_CHECK((lhs == rhs));
}

BOOST_AUTO_TEST_CASE(Keybindings_are_unequal_if_all_actions_equal) {
	MyBind lhs, rhs;

	lhs.set(MyAction::Attack, {sf::Keyboard::Key::Space});
	lhs.set(MyAction::Cast, {sf::Keyboard::Key::Return});
	rhs.set(MyAction::Attack, {sf::Keyboard::Key::Return});
	rhs.set(MyAction::Cast, {sf::Keyboard::Key::Space});

	BOOST_CHECK(lhs != rhs);
}

BOOST_AUTO_TEST_CASE(Keybinding_can_be_equal_but_differ_in_gamepad_id) {
	MyBind lhs, rhs;

	lhs.set(MyAction::Attack, {0u, 2u});
	lhs.set(MyAction::Cast, {0u, 4u});
	rhs.set(MyAction::Attack, {5u, 2u});
	rhs.set(MyAction::Cast, {5u, 4u});

	BOOST_CHECK(lhs.isSimilar(rhs));
}

BOOST_AUTO_TEST_SUITE_END()
