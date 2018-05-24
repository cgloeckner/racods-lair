#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/projectile.hpp>

struct ProjectileFixture {
	sf::Texture dummy_tileset;
	core::IdManager id_manager;
	core::LogContext log;
	core::DungeonSystem dungeon_system;

	rpg::CombatSender combat_sender;
	rpg::ProjectileSender projectile_sender;

	core::MovementManager movement_manager;
	core::CollisionManager collision_manager;
	rpg::ProjectileManager projectile_manager;

	rpg::projectile_impl::Context context;
	std::vector<core::ObjectID> objects;

	rpg::BulletTemplate bullet;
	rpg::ItemTemplate bow;
	rpg::PerkTemplate fireball;
	rpg::TrapTemplate trap;
	rpg::CombatMetaData meta_data;

	ProjectileFixture()
		: dummy_tileset{}
		, id_manager{}
		, log{}
		, dungeon_system{}
		, combat_sender{}
		, projectile_sender{}
		, movement_manager{}
		, collision_manager{}
		, projectile_manager{}
		, context{log, combat_sender, projectile_sender, projectile_manager,
			  movement_manager, collision_manager, dungeon_system}
		, objects{}
		, bullet{}
		, bow{}
		, fireball{}
		, trap{}
		, meta_data{} {
		// add a scenes
		auto scene = dungeon_system.create(
			dummy_tileset, sf::Vector2u{10u, 10u}, sf::Vector2f{1.f, 1.f});
		assert(scene == 1u);
		auto& dungeon = dungeon_system[1u];
		for (auto y = 1u; y < 10u; ++y) {
			for (auto x = 1u; x < 10u; ++x) {
				dungeon.getCell({x, y}).terrain = core::Terrain::Floor;
			}
		}
		// setup bullet template
		bullet.radius = 0.25f;

		meta_data.emitter = rpg::EmitterType::Trap;
		meta_data.primary = &bow;
		meta_data.perk = &fireball;
		meta_data.trap = &trap;
	}

	void reset() {
		auto& dungeon = dungeon_system[1u];
		// clear dungeon
		for (auto y = 0u; y < 10u; ++y) {
			for (auto x = 0u; x < 10u; ++x) {
				dungeon.getCell({x, y}).entities.clear();
			}
		}
		for (auto id : objects) {
			// remove components
			movement_manager.release(id);
			if (collision_manager.has(id)) {
				collision_manager.release(id);
			}
			if (projectile_manager.has(id)) {
				projectile_manager.release(id);
			}
		}
		objects.clear();
		// cleanup systems
		id_manager.reset();
		movement_manager.cleanup();
		collision_manager.cleanup();
		projectile_manager.cleanup();
		// reset event senders
		combat_sender.clear();
		projectile_sender.clear();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}

	core::ObjectID add_bullet(sf::Vector2f const& pos) {
		auto id = id_manager.acquire();
		objects.push_back(id);
		auto& move = movement_manager.acquire(id);
		move.scene = 1u;
		move.pos = pos;
		move.last_pos = pos;
		auto& cell = dungeon_system[move.scene].getCell(sf::Vector2u{pos});
		cell.entities.push_back(id);
		auto& coll = collision_manager.acquire(id);
		coll.is_projectile = true;
		coll.shape.radius = bullet.radius;
		auto& proj = projectile_manager.acquire(id);
		proj.bullet = &bullet;
		proj.meta_data = meta_data;
		return id;
	}

	core::ObjectID add_object(sf::Vector2f const& pos) {
		auto id = id_manager.acquire();
		objects.push_back(id);
		auto& move = movement_manager.acquire(id);
		move.scene = 1u;
		move.pos = pos;
		move.last_pos = pos;
		auto& cell = dungeon_system[move.scene].getCell(sf::Vector2u{pos});
		cell.entities.push_back(id);
		auto& c = collision_manager.acquire(id);
		c.shape.radius = 0.5f;
		return id;
	}
};

BOOST_AUTO_TEST_SUITE(projectile_test)

BOOST_AUTO_TEST_CASE(projectile_cannot_hit_if_target_cannot_collide) {
	auto& fix = Singleton<ProjectileFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet({2.f, 2.75f});
	auto target = fix.add_object({2.f, 3.f});
	fix.collision_manager.release(target);
	fix.collision_manager.cleanup();
	auto& proj = fix.projectile_manager.query(bullet);
	BOOST_CHECK(
		!rpg::projectile_impl::canHit(fix.context, proj, {2.f, 2.75f}, target));
}

BOOST_AUTO_TEST_CASE(projectile_cannot_hit_if_target_is_projectile) {
	auto& fix = Singleton<ProjectileFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet({2.f, 2.75f});
	auto target = fix.add_object({2.f, 3.f});
	fix.projectile_manager.acquire(target);
	auto& proj = fix.projectile_manager.query(bullet);
	BOOST_CHECK(
		!rpg::projectile_impl::canHit(fix.context, proj, {2.f, 2.75f}, target));
}

BOOST_AUTO_TEST_CASE(projectile_cannot_hit_itself) {
	auto& fix = Singleton<ProjectileFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet({2.f, 2.75f});
	auto& proj = fix.projectile_manager.query(bullet);
	BOOST_CHECK(
		!rpg::projectile_impl::canHit(fix.context, proj, {2.f, 2.75f}, bullet));
}

BOOST_AUTO_TEST_CASE(projectile_cannot_hit_if_target_is_too_far_away) {
	auto& fix = Singleton<ProjectileFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet({2.f, 2.f});
	auto target = fix.add_object({2.f, 2.76f});
	auto& proj = fix.projectile_manager.query(bullet);
	BOOST_CHECK(
		!rpg::projectile_impl::canHit(fix.context, proj, {2.f, 2.f}, target));
}

BOOST_AUTO_TEST_CASE(projectile_can_hit_if_target_is_located_near_by) {
	auto& fix = Singleton<ProjectileFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet({2.f, 2.f});
	auto target = fix.add_object({2.f, 2.74f});
	auto& proj = fix.projectile_manager.query(bullet);
	BOOST_CHECK(
		rpg::projectile_impl::canHit(fix.context, proj, {2.f, 2.f}, target));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(projectile_hits_target_on_collision) {
	auto& fix = Singleton<ProjectileFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet({2.f, 2.f});
	auto target = fix.add_object({2.2f, 2.1f});
	core::CollisionEvent event;
	event.actor = bullet;
	event.collider = target;
	rpg::projectile_impl::onCollision(fix.context, event);
	auto const& events = fix.combat_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, bullet);
	BOOST_CHECK_EQUAL(events[0].target, target);
	BOOST_CHECK(events[0].meta_data.emitter == fix.meta_data.emitter);
	BOOST_CHECK_EQUAL(events[0].meta_data.primary, fix.meta_data.primary);
	BOOST_CHECK_EQUAL(events[0].meta_data.perk, fix.meta_data.perk);
	BOOST_CHECK_EQUAL(events[0].meta_data.trap, fix.meta_data.trap);
}

BOOST_AUTO_TEST_CASE(projectile_does_not_hits_target_that_should_be_ignored) {
	auto& fix = Singleton<ProjectileFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet({2.f, 2.f});
	auto target = fix.add_object({2.2f, 2.1f});
	fix.projectile_manager.query(bullet).ignore.push_back(target);
	core::CollisionEvent event;
	event.actor = bullet;
	event.collider = target;
	rpg::projectile_impl::onCollision(fix.context, event);
	BOOST_CHECK(fix.combat_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(projectile_destruction_is_triggered_if_nobody_is_hit) {
	auto& fix = Singleton<ProjectileFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet({2.f, 2.f});
	core::CollisionEvent event;
	event.actor = bullet;
	event.collider = 0u;
	rpg::projectile_impl::onCollision(fix.context, event);
	auto const& events = fix.projectile_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].id, bullet);
	BOOST_CHECK(events[0].type == rpg::ProjectileEvent::Destroy);
}

BOOST_AUTO_TEST_SUITE_END()
