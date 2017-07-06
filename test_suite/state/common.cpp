#include <boost/test/unit_test.hpp>

#include <state/common.hpp>

BOOST_AUTO_TEST_SUITE(StateCommon_test)

BOOST_AUTO_TEST_CASE(Lobby_detects_keyboard_in_gamepad_binding) {
	state::LobbyContext lobby{1u};
	lobby.num_players = 1u;
	lobby.players[0].use_gamepad = true;
	lobby.players[0].keys.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::W});
	
	std::size_t i;
	BOOST_REQUIRE(lobby.hasInconsistentProfile(i));
	BOOST_CHECK_EQUAL(i, 0u);
}

BOOST_AUTO_TEST_CASE(Lobby_detects_gamepad_button_in_keyboard_binding) {
	state::LobbyContext lobby{1u};
	lobby.num_players = 1u;
	lobby.players[0].use_gamepad = false;
	lobby.players[0].keys.map.set(rpg::PlayerAction::MoveN, {0u, 2u});
	
	std::size_t i;
	BOOST_REQUIRE(lobby.hasInconsistentProfile(i));
	BOOST_CHECK_EQUAL(i, 0u);
}

BOOST_AUTO_TEST_CASE(Lobby_detects_gamepad_axis_in_keyboard_binding) {
	state::LobbyContext lobby{1u};
	lobby.num_players = 1u;
	lobby.players[0].use_gamepad = false;
	lobby.players[0].keys.map.set(rpg::PlayerAction::MoveN, {0u, sf::Joystick::Axis::X, 10.f});
	
	std::size_t i;
	BOOST_REQUIRE(lobby.hasInconsistentProfile(i));
	BOOST_CHECK_EQUAL(i, 0u);
}

BOOST_AUTO_TEST_CASE(Lobby_accepts_consistent_bindings) {
	state::LobbyContext lobby{1u};
	lobby.num_players = 1u;
	lobby.players[0].filename = "bar";
	
	std::size_t i;
	BOOST_REQUIRE(!lobby.hasInconsistentProfile(i));
}

BOOST_AUTO_TEST_CASE(Lobby_only_checks_first_n_players_for_consisntenty) {
	state::LobbyContext lobby{1u};
	lobby.num_players = 0u;
	lobby.players[0].use_gamepad = true;
	lobby.players[0].keys.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::W});
	
	std::size_t i;
	BOOST_REQUIRE(!lobby.hasInconsistentProfile(i));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Lobby_detects_double_use_of_profiles) {
	state::LobbyContext lobby{3u};
	lobby.num_players = 3u;
	lobby.players[0].filename = "bar";
	lobby.players[1].filename = "foo";
	lobby.players[2].filename = "foo";
	
	std::size_t i;
	BOOST_REQUIRE(lobby.hasDoubleUsedProfile(i));
	BOOST_CHECK_EQUAL(i, 2u);
}

BOOST_AUTO_TEST_CASE(Lobby_accepts_single_use_per_profile) {
	state::LobbyContext lobby{3u};lobby.num_players = 3u;
	lobby.players[0].filename = "bar";
	lobby.players[1].filename = "foo";
	lobby.players[2].filename = "test";
	
	std::size_t i;
	BOOST_REQUIRE(!lobby.hasDoubleUsedProfile(i));
}

BOOST_AUTO_TEST_CASE(Lobby_only_checks_forst_n_players_for_multi_profiles) {
	state::LobbyContext lobby{3u};
	lobby.num_players = 2u;
	lobby.players[0].filename = "bar";
	lobby.players[1].filename = "foo";
	lobby.players[2].filename = "foo";
	
	std::size_t i;
	BOOST_REQUIRE(!lobby.hasDoubleUsedProfile(i));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Lobby_detects_unset_profiles) {
	state::LobbyContext lobby{3u};
	lobby.num_players = 3u;
	lobby.players[0].filename = "bar";
	lobby.players[1].filename = "";
	lobby.players[2].filename = "foo";
	
	std::size_t i;
	BOOST_REQUIRE(lobby.hasUnsetProfile(i));
	BOOST_CHECK_EQUAL(i, 1u);
}

BOOST_AUTO_TEST_CASE(Lobby_accepts_set_profiles) {
	state::LobbyContext lobby{3u};
	lobby.num_players = 3u;
	lobby.players[0].filename = "bar";
	lobby.players[1].filename = "test";
	lobby.players[2].filename = "foo";
	
	std::size_t i;
	BOOST_REQUIRE(!lobby.hasUnsetProfile(i));
}

BOOST_AUTO_TEST_CASE(Lobby_only_tests_first_n_profiles_for_unset_profiles) {
	state::LobbyContext lobby{3u};
	lobby.num_players = 2u;
	lobby.players[0].filename = "bar";
	lobby.players[1].filename = "foo";
	lobby.players[2].filename = "";
	
	std::size_t i;
	BOOST_REQUIRE(!lobby.hasUnsetProfile(i));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Lobby_detects_ambiguous_input) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 4u;
	lobby.players[1].keys.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::W});
	lobby.players[1].keys.map.set(rpg::PlayerAction::LookE, {sf::Keyboard::W});
	
	utils::InputAction a;
	std::size_t i;
	BOOST_REQUIRE(lobby.hasAmbiguousInput(a, i));
	BOOST_REQUIRE(a.type == utils::InputAction::Key);
	BOOST_CHECK(a.key.key == sf::Keyboard::W);
	BOOST_CHECK_EQUAL(i, 1u);
}

BOOST_AUTO_TEST_CASE(Lobby_accepts_non_ambigous_input) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 4u;
	
	utils::InputAction a;
	std::size_t i;
	BOOST_REQUIRE(!lobby.hasAmbiguousInput(a, i));
}

BOOST_AUTO_TEST_CASE(Lobby_only_tests_first_n_profiles_for_ambiguous_input) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 3u;
	lobby.players[3].keys.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::W});
	lobby.players[3].keys.map.set(rpg::PlayerAction::LookE, {sf::Keyboard::W});
	
	utils::InputAction a;
	std::size_t i;
	BOOST_REQUIRE(!lobby.hasAmbiguousInput(a, i));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Lobby_detects_shared_input) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 4u;
	lobby.players[1].keys.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::W});
	lobby.players[2].keys.map.set(rpg::PlayerAction::LookS, {sf::Keyboard::W});
	
	utils::InputAction a;
	std::size_t i, j;
	BOOST_REQUIRE(lobby.hasSharedInput(a, i, j));
	BOOST_REQUIRE(a.type == utils::InputAction::Key);
	BOOST_CHECK(a.key.key == sf::Keyboard::W);
	BOOST_CHECK_EQUAL(i, 1u);
	BOOST_CHECK_EQUAL(j, 2u);
}

BOOST_AUTO_TEST_CASE(Lobby_accepts_non_shared_input) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 4u;
	
	utils::InputAction a;
	std::size_t i, j;
	BOOST_REQUIRE(!lobby.hasSharedInput(a, i, j));
}

BOOST_AUTO_TEST_CASE(Lobby_only_checks_first_n_profiles_for_shared_input) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 3u;
	lobby.players[1].keys.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::W});
	lobby.players[3].keys.map.set(rpg::PlayerAction::LookS, {sf::Keyboard::W});
	
	utils::InputAction a;
	std::size_t i, j;
	BOOST_REQUIRE(!lobby.hasSharedInput(a, i, j));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Lobby_detects_shared_gamepads) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 4u;
	lobby.players[1].use_gamepad = true;
	lobby.players[1].gamepad_id = 2u;
	lobby.players[2].use_gamepad = true;
	lobby.players[2].gamepad_id = 2u;
	
	unsigned int id;
	BOOST_REQUIRE(lobby.hasSharedGamepad(id));
	BOOST_CHECK_EQUAL(id, 2u);
}

BOOST_AUTO_TEST_CASE(Lobby_accepts_non_shared_gamepads) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 4u;
	lobby.players[1].use_gamepad = true;
	lobby.players[1].gamepad_id = 2u;
	lobby.players[2].use_gamepad = true;
	lobby.players[2].gamepad_id = 3u;
	
	unsigned int id;
	BOOST_REQUIRE(!lobby.hasSharedGamepad(id));
}

BOOST_AUTO_TEST_CASE(Lobby_only_checks_first_n_players_for_shared_gamepads) {
	state::LobbyContext lobby{4u};
	lobby.num_players = 2u;
	lobby.players[1].use_gamepad = true;
	lobby.players[1].gamepad_id = 2u;
	lobby.players[2].use_gamepad = true;
	lobby.players[2].gamepad_id = 2u;
	
	unsigned int id;
	BOOST_REQUIRE(!lobby.hasSharedGamepad(id));
}

BOOST_AUTO_TEST_SUITE_END()
