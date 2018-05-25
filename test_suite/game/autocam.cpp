#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <core/teleport.hpp>
#include <game/autocam.hpp>

struct AutoCamFixture {
	sf::Texture dummy_tileset;
	core::IdManager id_manager;
	std::vector<core::ObjectID> ids;

	core::LogContext log;
	core::MovementManager movement;
	core::DungeonSystem dungeon;
	core::CameraSystem camera;
	game::autocam_impl::Context context;
	
	utils::SceneID scene1, scene2;

	AutoCamFixture()
		: dummy_tileset{}
		, id_manager{}
		, ids{}
		, log{}
		, movement{}
		, dungeon{}
		, camera{{320u, 240u}}
		, context{log, movement, dungeon, camera} {
		//log.debug.add(std::cout);
		
		// add dungeons
		sf::Vector2u grid_size{30u, 30u};
		scene1 = dungeon.create(dummy_tileset, grid_size, sf::Vector2f{1.f, 1.f});
		scene2 = dungeon.create(dummy_tileset, grid_size, sf::Vector2f{1.f, 1.f});
		for (auto scene: {scene1, scene2}) {
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
		}
	}

	void reset() {
		// clear dungeons
		sf::Vector2u grid_size{30u, 30u};
		for (auto scene: {scene1, scene2}) {
			auto& d = dungeon[scene];
			for (auto y = 0u; y < grid_size.y; ++y) {
				for (auto x = 0u; x < grid_size.x; ++x) {
					auto& cell = d.getCell({x, y});
					cell.entities.clear();
				}
			}
		}
		// remove components
		for (auto id : ids) {
			movement.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		movement.cleanup();
		camera.clear();
		
		context.distance = 7.f;
		context.scenes.clear();
		context.clusters.clear();
		context.changed = false;
	}
	
	core::ObjectID create(sf::Vector2f const & pos, utils::SceneID scene) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& data = movement.acquire(id);
		core::spawn(dungeon[scene], data, pos);
		return id;
	}
};

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(autocam_test)

BOOST_AUTO_TEST_CASE(autocam_keeps_two_players_sharing_if_close_enough) {
	auto& fix = Singleton<AutoCamFixture>::get();
	fix.reset();
	
	auto obj1 = fix.create({2.f, 2.f}, fix.scene1);
	auto obj2 = fix.create({2.f + fix.context.distance - 1.f, 2.f}, fix.scene1);
	auto& cam = fix.camera.acquire();
	cam.objects.push_back(obj1);
	cam.objects.push_back(obj2);
	
	game::autocam_impl::onUpdate(fix.context);
	BOOST_REQUIRE(!fix.context.changed);
	
	BOOST_REQUIRE_EQUAL(fix.camera.size(), 1u);
	BOOST_REQUIRE_EQUAL(cam.objects.size(), 2u);
	BOOST_CHECK_EQUAL(cam.objects[0], obj1);
	BOOST_CHECK_EQUAL(cam.objects[1], obj2);
}

BOOST_AUTO_TEST_CASE(autocam_keeps_two_players_splitted_if_far_enough) {
	auto& fix = Singleton<AutoCamFixture>::get();
	fix.reset();
	
	auto obj1 = fix.create({2.f, 2.f}, fix.scene1);
	auto obj2 = fix.create({2.f + fix.context.distance, 2.f}, fix.scene1);
	auto& cam1 = fix.camera.acquire();
	auto& cam2 = fix.camera.acquire();
	cam1.objects.push_back(obj1);
	cam2.objects.push_back(obj2);
	
	game::autocam_impl::onUpdate(fix.context);
	BOOST_REQUIRE(!fix.context.changed);
	
	BOOST_REQUIRE_EQUAL(fix.camera.size(), 2u);
	BOOST_REQUIRE_EQUAL(cam1.objects.size(), 1u);
	BOOST_REQUIRE_EQUAL(cam2.objects.size(), 1u);
	BOOST_CHECK_EQUAL(cam1.objects[0], obj1);
	BOOST_CHECK_EQUAL(cam2.objects[0], obj2);
}

BOOST_AUTO_TEST_CASE(autocam_splits_two_players_if_too_far) {
	auto& fix = Singleton<AutoCamFixture>::get();
	fix.reset();
	
	auto obj1 = fix.create({2u, 2u}, fix.scene1);
	auto obj2 = fix.create({2.f + fix.context.distance, 2.f}, fix.scene1);
	auto& cam = fix.camera.acquire();
	cam.objects.push_back(obj1);
	cam.objects.push_back(obj2);
	
	game::autocam_impl::onUpdate(fix.context);
	BOOST_REQUIRE(fix.context.changed);
	
	BOOST_REQUIRE_EQUAL(fix.camera.size(), 2u);
	auto& first = fix.camera.query(obj1);
	auto& second = fix.camera.query(obj2);
	BOOST_REQUIRE_EQUAL(first.objects.size(), 1u);
	BOOST_REQUIRE_EQUAL(second.objects.size(), 1u);
	BOOST_CHECK_EQUAL(first.objects[0], obj1);
	BOOST_CHECK_EQUAL(second.objects[0], obj2);
}

BOOST_AUTO_TEST_CASE(autocam_splits_two_players_if_different_scene) {
	auto& fix = Singleton<AutoCamFixture>::get();
	fix.reset();
	
	auto obj1 = fix.create({2u, 2u}, fix.scene1);
	auto obj2 = fix.create({2u, 2u}, fix.scene2);
	auto& cam = fix.camera.acquire();
	cam.objects.push_back(obj1);
	cam.objects.push_back(obj2);
	
	game::autocam_impl::onUpdate(fix.context);
	BOOST_REQUIRE(fix.context.changed);
	
	BOOST_REQUIRE_EQUAL(fix.camera.size(), 2u);
	auto& first = fix.camera.query(obj1);
	auto& second = fix.camera.query(obj2);
	BOOST_REQUIRE_EQUAL(first.objects.size(), 1u);
	BOOST_REQUIRE_EQUAL(first.objects.size(), 1u);
	BOOST_CHECK_EQUAL(first.objects[0], obj1);
	BOOST_CHECK_EQUAL(second.objects[0], obj2);
}

BOOST_AUTO_TEST_CASE(autocam_joins_two_players_if_close_enough) {
	auto& fix = Singleton<AutoCamFixture>::get();
	fix.reset();
	
	auto obj1 = fix.create({2u, 2u}, fix.scene1);
	auto obj2 = fix.create({2.f + fix.context.distance, 2.f}, fix.scene1);
	auto& cam1 = fix.camera.acquire();
	auto& cam2 = fix.camera.acquire();
	cam1.objects.push_back(obj1);
	cam2.objects.push_back(obj2);
	
	game::autocam_impl::onUpdate(fix.context);
	BOOST_REQUIRE(fix.context.changed);
	
	BOOST_REQUIRE_EQUAL(fix.camera.size(), 1u);
	auto& cam = fix.camera.query(obj1);
	BOOST_REQUIRE_EQUAL(cam.objects.size(), 2u);
	BOOST_CHECK_EQUAL(cam.objects[0], obj1);
	BOOST_CHECK_EQUAL(cam.objects[1], obj2);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(autocam_splits_some_players_who_are_too_far_away) {
	auto& fix = Singleton<AutoCamFixture>::get();
	fix.reset();
	
	auto obj1 = fix.create({2u, 2u}, fix.scene1);
	auto obj2 = fix.create({2u, 2.f + fix.context.distance}, fix.scene1);
	auto obj3 = fix.create({2.f + fix.context.distance, 2.f}, fix.scene1);
	auto obj4 = fix.create({2u + fix.context.distance, 2u + fix.context.distance - 1.f}, fix.scene1);
	auto& cam = fix.camera.acquire();
	cam.objects.push_back(obj1);
	cam.objects.push_back(obj2);
	cam.objects.push_back(obj3);
	cam.objects.push_back(obj4);
	
	game::autocam_impl::onUpdate(fix.context);
	BOOST_REQUIRE(fix.context.changed);
	
	auto& first = fix.camera.query(obj1);
	auto& second = fix.camera.query(obj3);
	BOOST_REQUIRE_EQUAL(fix.camera.size(), 2u);
	BOOST_REQUIRE_EQUAL(first.objects.size(), 2u);
	BOOST_REQUIRE_EQUAL(second.objects.size(), 2u);
	BOOST_CHECK_EQUAL(first.objects[0], obj1);
	BOOST_CHECK_EQUAL(first.objects[1], obj2);
	BOOST_CHECK_EQUAL(second.objects[0], obj3);
	BOOST_CHECK_EQUAL(second.objects[1], obj4);
}

BOOST_AUTO_TEST_CASE(autocam_joins_some_players_who_are_close_enougth) {
	auto& fix = Singleton<AutoCamFixture>::get();
	fix.reset();
	
	auto obj1 = fix.create({2u, 2u}, fix.scene1);
	auto obj2 = fix.create({2u, 2.f + fix.context.distance}, fix.scene1);
	auto obj3 = fix.create({2.f + fix.context.distance, 2.f}, fix.scene1);
	auto obj4 = fix.create({2u + fix.context.distance, 2u + fix.context.distance - 1.f}, fix.scene1);
	auto& cam1 = fix.camera.acquire();
	auto& cam2 = fix.camera.acquire();
	auto& cam3 = fix.camera.acquire();
	auto& cam4 = fix.camera.acquire();
	cam1.objects.push_back(obj1);
	cam2.objects.push_back(obj2);
	cam3.objects.push_back(obj3);
	cam4.objects.push_back(obj4);
	
	game::autocam_impl::onUpdate(fix.context);
	BOOST_REQUIRE(fix.context.changed);
	
	BOOST_REQUIRE_EQUAL(fix.camera.size(), 2u);
	auto& first = fix.camera.query(obj1);
	auto& second = fix.camera.query(obj3);
	BOOST_REQUIRE_EQUAL(first.objects.size(), 2u);
	BOOST_REQUIRE_EQUAL(second.objects.size(), 2u);
	BOOST_CHECK_EQUAL(first.objects[0], obj1);
	BOOST_CHECK_EQUAL(first.objects[1], obj2);
	BOOST_CHECK_EQUAL(second.objects[0], obj3);
	BOOST_CHECK_EQUAL(second.objects[1], obj4);
}

BOOST_AUTO_TEST_SUITE_END()
