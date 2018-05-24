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

	FocusFixture()
		: dummy_tileset{}
		, id_manager{}
		, log{}
		, focus_sender{}
		, focus_manager{}
		, dungeon_system{}
		, movement_manager{} {
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
			if (focus_manager.has(id)) {
				focus_manager.release(id);
			}
			movement_manager.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		focus_manager.cleanup();
		movement_manager.cleanup();
		// reset event senders
		focus_sender.clear();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}

	core::ObjectID add_object(
		sf::Vector2u const& pos, sf::Vector2i const& look, float sight) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		if (sight > 0.f) {
			auto& foc = focus_manager.acquire(id);
			foc.sight = sight;
			foc.fov = 120.f;
			foc.display_name = "foo";
		}
		auto& mve = movement_manager.acquire(id);
		mve.pos = sf::Vector2f{pos};
		mve.last_pos = mve.pos;
		mve.scene = 1u;
		mve.look = {1.f, 0.f};
		auto& dungeon = dungeon_system[1u];
		dungeon.getCell(pos).entities.push_back(id);

		return id;
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(focus_test)

BOOST_AUTO_TEST_CASE(getFocus_delivers_id_but_not_itself) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({2u, 1u}, {0, 1}, 5.f);
	
	auto focus = core::focus_impl::getFocus(id, dungeon, fixture.focus_manager, fixture.movement_manager);
	BOOST_CHECK_EQUAL(focus, second);
}

BOOST_AUTO_TEST_CASE(getFocus_ignores_unfocusables) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({2u, 1u}, {0, 1}, 0.f); // sight = 0 --> unfocusable
	
	auto focus = core::focus_impl::getFocus(id, dungeon, fixture.focus_manager, fixture.movement_manager);
	BOOST_CHECK_EQUAL(focus, 0u);
}

BOOST_AUTO_TEST_CASE(getFocus_returns_inactive) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	// make second inactive
	fixture.focus_manager.query(second).is_active = false;
	
	auto focus = core::focus_impl::getFocus(id, dungeon, fixture.focus_manager, fixture.movement_manager);
	BOOST_CHECK_EQUAL(focus, 0u);
}

BOOST_AUTO_TEST_CASE(getFocus_delivers_closest_id) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({2u, 1u}, {0, 1}, 5.f);
	auto third = fixture.add_object({3u, 1u}, {0, 1}, 5.f);
	
	auto focus = core::focus_impl::getFocus(id, dungeon, fixture.focus_manager, fixture.movement_manager);
	BOOST_CHECK_EQUAL(focus, second);
}

BOOST_AUTO_TEST_CASE(getFocus_ignores_out_of_sight_ids) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({7u, 1u}, {0, 1}, 5.f);
	
	auto focus = core::focus_impl::getFocus(id, dungeon, fixture.focus_manager, fixture.movement_manager);
	BOOST_CHECK_EQUAL(focus, 0u);
}

BOOST_AUTO_TEST_CASE(getFocus_ignores_out_of_fov_ids) {
	auto& fixture = Singleton<FocusFixture>::get();
	fixture.reset();

	auto const& dungeon = fixture.dungeon_system[1];
	auto id = fixture.add_object({1u, 1u}, {1, 0}, 5.f);
	auto second = fixture.add_object({1u, 2u}, {0, 1}, 5.f);
	
	auto focus = core::focus_impl::getFocus(id, dungeon, fixture.focus_manager, fixture.movement_manager);
	BOOST_CHECK_EQUAL(focus, 0u);
}

BOOST_AUTO_TEST_SUITE_END()
