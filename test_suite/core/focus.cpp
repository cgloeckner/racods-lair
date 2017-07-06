#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <core/focus.hpp>

struct FocusFixture {
	sf::Texture dummy_tileset;
	core::IdManager id_manager;
	std::vector<core::ObjectID> ids;

	core::LogContext log;
	core::FocusSender focus_sender;
	core::FocusManager focus_manager;
	core::DungeonSystem dungeon_system;
	core::MovementManager movement_manager;
	core::focus_impl::Context context;

	FocusFixture()
		: dummy_tileset{}
		, id_manager{}
		, log{}
		, focus_sender{}
		, focus_manager{}
		, dungeon_system{}
		, movement_manager{}
		, context{log, focus_sender, focus_manager, dungeon_system,
			  movement_manager} {
		// add a scenes
		auto scene = dungeon_system.create(
			dummy_tileset, sf::Vector2u{12u, 10u}, sf::Vector2f{1.f, 1.f});
		assert(scene == 1u);
		auto& dungeon = dungeon_system[1u];
		for (auto y = 1u; y < 10u; ++y) {
			for (auto x = 1u; x < 12u; ++x) {
				dungeon.getCell({x, y}).terrain = core::Terrain::Floor;
			}
		}
	}

	void reset() {
		auto& dungeon = dungeon_system[1u];
		// clear dungeon
		for (auto y = 0u; y < 10u; ++y) {
			for (auto x = 0u; x < 12u; ++x) {
				auto& cell = dungeon.getCell({x, y});
				cell.entities.clear();
				cell.terrain = core::Terrain::Floor;
			}
		}
		// remove components
		for (auto id : ids) {
			focus_manager.release(id);
			movement_manager.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		focus_manager.cleanup();
		movement_manager.cleanup();
		// reset event senders
		focus_sender.clear();
	}

	core::ObjectID add_object(
		sf::Vector2u const& pos, sf::Vector2i const& look, float sight) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& foc = focus_manager.acquire(id);
		foc.look = look;
		foc.sight = sight;
		if (sight > 0.f) {
			foc.display_name = "foo";
		}
		auto& mve = movement_manager.acquire(id);
		mve.pos = sf::Vector2f{pos};
		mve.target = pos;
		mve.scene = 1u;
		auto& dungeon = dungeon_system[1u];
		dungeon.getCell(pos).entities.push_back(id);

		// notify about object
		core::MoveEvent event;
		event.actor = id;
		event.target = pos;
		event.type = core::MoveEvent::Left;
		core::focus_impl::onMove(context, foc, event);

		return id;
	}

	core::InputEvent look_object(core::ObjectID id, sf::Vector2i const& look) {
		core::InputEvent event;
		event.actor = id;
		event.move = {0, 0};
		event.look = look;
		return event;
	}

	core::MoveEvent move_object(
		core::ObjectID id, sf::Vector2u const& pos, sf::Vector2i const& look) {
		// move object directly to target cell
		auto& move = movement_manager.query(id);
		auto prev = sf::Vector2u{move.pos};
		move.pos = sf::Vector2f{pos};
		move.target = pos;
		auto& focus = focus_manager.query(id);
		focus.look = look;
		auto& dungeon = dungeon_system[1];
		auto& src = dungeon.getCell(prev);
		auto& dst = dungeon.getCell(move.target);
		ASSERT(utils::pop(src.entities, id));
		dst.entities.push_back(id);
		// propagate
		core::MoveEvent event;
		event.actor = id;
		event.source = prev;
		event.target = move.target;
		event.type = core::MoveEvent::Left;
		return event;
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(focus_test)

BOOST_AUTO_TEST_CASE(traversal_finds_very_near_object) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({2u, 1u}, {0, 1}, 5.f);

	auto found = core::focus_impl::traverseCells(dungeon, {1u, 1u}, {1, 0}, 5.f,
		[&](core::ObjectID other, int) { return other != id; });
	BOOST_CHECK_EQUAL(found, second);
}

BOOST_AUTO_TEST_CASE(traversal_pass_walls) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	fixture.dungeon_system[1u].getCell({2u, 1u}).terrain = core::Terrain::Wall;

	auto found = core::focus_impl::traverseCells(dungeon, {1u, 1u}, {1, 0}, 5.f,
		[&](core::ObjectID other, int) { return other != id; });
	BOOST_CHECK_EQUAL(found, 0u);
}

BOOST_AUTO_TEST_CASE(traversal_finds_closest_object) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({2u, 1u}, {0, 1}, 5.f);
	fixture.add_object({3u, 1u}, {0, 1}, 5.f);

	auto found = core::focus_impl::traverseCells(dungeon, {1u, 1u}, {1, 0}, 5.f,
		[&](core::ObjectID other, int) { return other != id; });
	BOOST_CHECK_EQUAL(found, second);
}

BOOST_AUTO_TEST_CASE(traversal_finds_medium_ranged_object) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);

	auto found = core::focus_impl::traverseCells(dungeon, {1u, 1u}, {1, 0}, 5.f,
		[&](core::ObjectID other, int) { return other != id; });
	BOOST_CHECK_EQUAL(found, second);
}

BOOST_AUTO_TEST_CASE(traversal_finds_very_far_object) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({6u, 1u}, {0, 1}, 5.f);

	auto found = core::focus_impl::traverseCells(dungeon, {1u, 1u}, {1, 0}, 5.f,
		[&](core::ObjectID other, int) { return other != id; });
	BOOST_CHECK_EQUAL(found, second);
}

BOOST_AUTO_TEST_CASE(traversal_cannot_find_too_far_object) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	fixture.add_object({7u, 1u}, {0, 1}, 5.f);

	auto found = core::focus_impl::traverseCells(dungeon, {1u, 1u}, {1, 0}, 5.f,
		[&](core::ObjectID other, int) { return other != id; });
	BOOST_CHECK_EQUAL(found, 0u);
}

BOOST_AUTO_TEST_CASE(traversal_finds_object_on_diagonal) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 1}, 5.f);
	auto second = fixture.add_object({3u, 3u}, {0, 1}, 5.f);

	auto found = core::focus_impl::traverseCells(dungeon, {1u, 1u}, {1, 1}, 5.f,
		[&](core::ObjectID other, int) { return other != id; });
	BOOST_CHECK_EQUAL(found, second);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(trigger_looking_sets_dirtyflag) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	actor.look = {1, 0};

	// trigger looking
	auto event = fixture.look_object(id, {0, 1});
	core::focus_impl::onLook(fixture.context, actor, event);

	// assert dirtyflag
	BOOST_CHECK(actor.has_changed);
}

BOOST_AUTO_TEST_CASE(cannot_focus_via_overflowing_map_width) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({3u, 1u}, {1, 0}, 15.f);
	fixture.add_object({1u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	BOOST_CHECK_EQUAL(actor.focus, 0u);
}

BOOST_AUTO_TEST_CASE(cannot_focus_via_overflowing_map_height) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 3u}, {0, 1}, 15.f);
	fixture.add_object({1u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	BOOST_CHECK_EQUAL(actor.focus, 0u);
}

BOOST_AUTO_TEST_CASE(cannot_focus_via_underflowing_map_width) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {-1, 0}, 15.f);
	fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	BOOST_CHECK_EQUAL(actor.focus, 0u);
}

BOOST_AUTO_TEST_CASE(cannot_focus_via_underflowing_map_height) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {0, -1}, 15.f);
	fixture.add_object({1u, 3u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	BOOST_CHECK_EQUAL(actor.focus, 0u);
}

BOOST_AUTO_TEST_CASE(focus_is_reset_on_look) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);
	BOOST_CHECK_EQUAL(actor.focus, second);
	BOOST_REQUIRE_EQUAL(target.observers.size(), 1u);
	BOOST_CHECK_EQUAL(target.observers[0], id);

	// trigger looking away
	auto event = fixture.look_object(id, {0, 1});
	core::focus_impl::onLook(fixture.context, actor, event);

	// assert unfocused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
	BOOST_REQUIRE_EQUAL(focus.size(), 2u);
	BOOST_CHECK_EQUAL(focus[1].type, core::FocusEvent::Lost);
	BOOST_CHECK_EQUAL(focus[1].observer, id);
	BOOST_CHECK_EQUAL(focus[1].observed, second);
}

BOOST_AUTO_TEST_CASE(focus_is_set_on_look_if_object_in_sight) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {0, 1}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// trigger looking towards
	auto event = fixture.look_object(id, {1, 0});
	core::focus_impl::onLook(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_REQUIRE_EQUAL(focus.size(), 1u);
	BOOST_CHECK_EQUAL(actor.focus, second);
	BOOST_REQUIRE_EQUAL(target.observers.size(), 1u);
	BOOST_CHECK_EQUAL(target.observers[0], id);
	auto const& foc = focus[0];
	BOOST_CHECK_EQUAL(foc.type, core::FocusEvent::Gained);
	BOOST_CHECK_EQUAL(foc.observer, id);
	BOOST_CHECK_EQUAL(foc.observed, second);
}

BOOST_AUTO_TEST_CASE(focus_is_not_set_on_look_if_no_object_in_sight) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {0, 1}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// trigger looking away
	auto event = fixture.look_object(id, {-1, 0});
	core::focus_impl::onLook(fixture.context, actor, event);

	// assert unfocused
	auto const& focus = fixture.focus_sender.data();
	BOOST_REQUIRE(focus.empty());
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
}

BOOST_AUTO_TEST_CASE(focus_over_diagonals_is_supported) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {0, 1}, 5.f);
	auto second = fixture.add_object({3u, 3u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// trigger looking away
	auto event = fixture.look_object(id, {1, 1});
	core::focus_impl::onLook(fixture.context, actor, event);

	// assert unfocused
	auto const& focus = fixture.focus_sender.data();
	BOOST_REQUIRE_EQUAL(focus.size(), 1u);
	BOOST_CHECK_EQUAL(actor.focus, second);
	BOOST_REQUIRE_EQUAL(target.observers.size(), 1u);
	BOOST_REQUIRE_EQUAL(target.observers[0], id);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(lose_focus_when_actor_moves_away_from_observed) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// move actor
	auto event = fixture.move_object(id, {1u, 2u}, {1, 0});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert unfocused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
	BOOST_REQUIRE_EQUAL(focus.size(), 2u);
	BOOST_CHECK_EQUAL(focus[1].type, core::FocusEvent::Lost);
	BOOST_CHECK_EQUAL(focus[1].observer, id);
	BOOST_CHECK_EQUAL(focus[1].observed, second);
}

BOOST_AUTO_TEST_CASE(lose_focus_when_observed_moves_away_from_actor) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// move observed
	auto event = fixture.move_object(second, {3u, 2u}, {1, 0});
	core::focus_impl::onMove(fixture.context, target, event);

	// assert unfocused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
	BOOST_REQUIRE_EQUAL(focus.size(), 2u);
	BOOST_CHECK_EQUAL(focus[1].type, core::FocusEvent::Lost);
	BOOST_CHECK_EQUAL(focus[1].observer, id);
	BOOST_CHECK_EQUAL(focus[1].observed, second);
}

BOOST_AUTO_TEST_CASE(actor_without_sight_cannot_gain_focus) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 2u}, {1, 0}, 0.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// move actor
	auto event = fixture.move_object(id, {1u, 1u}, {0, 1});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK(!focus.size());
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
}

BOOST_AUTO_TEST_CASE(inactive_actor_gain_focus) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 2u}, {1, 0}, 2.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	actor.is_active = false;
	auto& target = fixture.focus_manager.query(second);

	// move actor
	auto event = fixture.move_object(id, {1u, 1u}, {0, 1});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK(!focus.size());
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
}

BOOST_AUTO_TEST_CASE(target_without_sight_but_with_display_name_can_be_focused) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 2u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 0.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);
	target.display_name = "not empty";

	// move actor
	auto event = fixture.move_object(id, {1u, 1u}, {1, 0});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK_EQUAL(focus.size(), 1u);
	BOOST_CHECK_EQUAL(actor.focus, second);
	BOOST_CHECK_EQUAL(target.observers.size(), 1u);
}

BOOST_AUTO_TEST_CASE(target_without_sight_and_display_name_cannot_be_focused) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 2u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 0.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// move actor
	auto event = fixture.move_object(id, {1u, 1u}, {1, 0});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK(focus.empty());
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
}

BOOST_AUTO_TEST_CASE(inactive_target_cannot_be_focused) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 2u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 2.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);
	target.is_active = false;

	// move actor
	auto event = fixture.move_object(id, {1u, 1u}, {1, 0});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK(!focus.size());
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
}

BOOST_AUTO_TEST_CASE(gain_focus_when_actor_moves_towards_observed) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 2u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// move actor
	auto event = fixture.move_object(id, {1u, 1u}, {1, 0});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_REQUIRE_EQUAL(focus.size(), 1u);
	BOOST_CHECK_EQUAL(actor.focus, second);
	BOOST_REQUIRE_EQUAL(target.observers.size(), 1u);
	BOOST_CHECK_EQUAL(target.observers[0], id);
	BOOST_CHECK_EQUAL(focus[0].type, core::FocusEvent::Gained);
	BOOST_CHECK_EQUAL(focus[0].observer, id);
	BOOST_CHECK_EQUAL(focus[0].observed, second);
}

BOOST_AUTO_TEST_CASE(gain_focus_when_observed_moves_towards_actor) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 2u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// move observed
	auto event = fixture.move_object(second, {3u, 2u}, {-1, 0});
	core::focus_impl::onMove(fixture.context, target, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_CHECK_EQUAL(actor.focus, second);
	BOOST_REQUIRE_EQUAL(target.observers.size(), 1u);
	BOOST_CHECK_EQUAL(target.observers[0], id);
	BOOST_REQUIRE_EQUAL(focus.size(), 2u);
	BOOST_CHECK_EQUAL(focus[1].type, core::FocusEvent::Gained);
	BOOST_CHECK_EQUAL(focus[1].observer, id);
	BOOST_CHECK_EQUAL(focus[1].observed, second);
}

BOOST_AUTO_TEST_CASE(both_remain_focus_when_moving_within_sight) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {-1, 0}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// move actor
	auto event = fixture.move_object(id, {2u, 1u}, {1, 0});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_REQUIRE_EQUAL(focus.size(), 2u);  // only the previous focus gains
	BOOST_CHECK_EQUAL(actor.focus, second);
	BOOST_REQUIRE_EQUAL(target.observers.size(), 1u);
	BOOST_CHECK_EQUAL(target.observers[0], id);
	BOOST_CHECK_EQUAL(target.focus, id);
	BOOST_REQUIRE_EQUAL(actor.observers.size(), 1u);
	BOOST_CHECK_EQUAL(actor.observers[0], second);
}

BOOST_AUTO_TEST_CASE(both_dont_gain_focus_when_moving_out_of_sight) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({8u, 1u}, {-1, 0}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);

	// move actor
	auto event = fixture.move_object(id, {2u, 1u}, {1, 0});
	core::focus_impl::onMove(fixture.context, actor, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_REQUIRE(focus.empty());
	BOOST_CHECK_EQUAL(actor.focus, 0u);
	BOOST_CHECK(target.observers.empty());
	BOOST_CHECK_EQUAL(target.focus, 0u);
	BOOST_CHECK(actor.observers.empty());
}

BOOST_AUTO_TEST_CASE(
	sight_gets_blocked_if_another_object_steps_in_line_of_sight) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({4u, 1u}, {-1, 0}, 5.f);
	auto third = fixture.add_object({3u, 2u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);
	auto& block = fixture.focus_manager.query(third);

	// move third
	auto event = fixture.move_object(third, {3u, 1u}, {0, 1});
	core::focus_impl::onMove(fixture.context, block, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_REQUIRE_EQUAL(focus.size(), 6u);
	BOOST_CHECK_EQUAL(actor.focus, third);
	BOOST_CHECK(target.observers.empty());
	BOOST_CHECK_EQUAL(target.focus, third);
	BOOST_CHECK(actor.observers.empty());
	BOOST_CHECK_EQUAL(block.observers.size(), 2u);
}

BOOST_AUTO_TEST_CASE(
	sight_gets_reset_if_blocking_object_steps_out_of_line_of_sights) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({4u, 1u}, {-1, 0}, 5.f);
	auto third = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	auto& actor = fixture.focus_manager.query(id);
	auto& target = fixture.focus_manager.query(second);
	auto& block = fixture.focus_manager.query(third);

	// move third
	auto event = fixture.move_object(third, {3u, 2u}, {0, 1});
	core::focus_impl::onMove(fixture.context, block, event);

	// assert focused
	auto const& focus = fixture.focus_sender.data();
	BOOST_REQUIRE_EQUAL(focus.size(), 10u);
	BOOST_CHECK_EQUAL(actor.focus, second);
	BOOST_REQUIRE_EQUAL(target.observers.size(), 1u);
	BOOST_CHECK_EQUAL(target.observers[0], id);
	BOOST_CHECK_EQUAL(target.focus, id);
	BOOST_REQUIRE_EQUAL(actor.observers.size(), 1u);
	BOOST_CHECK_EQUAL(actor.observers[0], second);
	BOOST_CHECK(block.observers.empty());
}

BOOST_AUTO_TEST_SUITE_END()
