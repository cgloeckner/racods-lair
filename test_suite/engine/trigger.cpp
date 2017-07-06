#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/trigger.hpp>

struct TriggerFixture {
	core::IdManager id_manager;
	core::LogContext log;
	core::MovementManager movement;
	rpg::TeleportSender teleport_sender;
	rpg::TrapSender trap_sender;
	std::vector<core::ObjectID> ids;

	rpg::BulletCreator bullet_creator;
	rpg::trigger_impl::Context context;
	rpg::TriggerScene scene;
	rpg::TrapTemplate trap;
	bool bullet_created;

	TriggerFixture()
		: id_manager{}
		, log{}
		, movement{}
		, teleport_sender{}
		, trap_sender{}
		, ids{}
		, bullet_creator{[](core::ObjectID, std::string const &, sf::Vector2u const &, sf::Vector2i) {
		bullet_created = true; })}
		, context{log, teleport_sender, trap_sender, movement, bullet_creator}
		, scene{context} {
		trap.damage[rpg::DamageType::Blunt] = 5;
	}

	void reset() {
		bullet_created = false;
		for (auto id : ids) {
			movement.release(id);
		}
		ids.clear();
		movement.cleanup();
		id_manager = core::IdManager{};
		teleport_sender.clear();
		trap_sender.clear();
		// drop old triggrs
		scene.clear();
		// register triggers
		scene.add<rpg::TeleportTrigger>(
			{15u, 7u}, static_cast<utils::SceneID>(2u), sf::Vector2u{3u, 11u});
		scene.add<rpg::TrapTrigger>({14u, 9u}, "bullet/fireball",
			sf::Vector2u{1u, 1u}, sf::Vector2i{1, 0});
	}

	core::MovementData& add_object() {
		auto id = id_manager.acquire();
		ids.push_back(id);
		return movement.acquire(id);
	}

	core::MoveEvent move(core::ObjectID id, sf::Vector2u pos) {
		core::MoveEvent event;
		event.actor = id;
		event.type = core::MoveEvent::Reached;
		event.source = pos;
		event.target = pos;
		return event;
	}
};

BOOST_AUTO_TEST_SUITE(trigger_test)

BOOST_AUTO_TEST_CASE(teleportTrigger_triggers_teleportEvent) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto trigger = rpg::TeleportTrigger{1u, {3u, 27u}};
	trigger(fix.context, 17u);

	auto const& teleports = fix.teleport_sender.data<rpg::TeleportEvent>();
	BOOST_REQUIRE_EQUAL(1u, teleports.size());
	auto const& tele = teleports[0];
	BOOST_CHECK_EQUAL(tele.actor, 17u);
	BOOST_CHECK_EQUAL(tele.scene, 1u);
	BOOST_CHECK_VECTOR_EQUAL(tele.pos, sf::Vector2u(3u, 27u));
}

BOOST_AUTO_TEST_CASE(trapTrigger_triggers_trapEvent) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto trigger = rpg::TrapTrigger{"", {}, {}};
	trigger(fix.context, 17u);
	BOOST_CHECK(fix.bullet_created);
}

BOOST_AUTO_TEST_CASE(triggers_can_be_cleared) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto event = fix.move(data.id, {15u, 7u});
	fix.scene.clear();
	fix.scene.handle(event);

	// expect teleport event
	auto const& teleports = fix.teleport_sender.data<rpg::TeleportEvent>();
	BOOST_CHECK(teleports.empty());
	auto const& traps = fix.trap_sender.data<rpg::TrapEvent>();
	BOOST_CHECK(traps.empty());
}

BOOST_AUTO_TEST_CASE(teleport_is_trigger_activated_on_tile_reached) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto event = fix.move(data.id, {15u, 7u});
	fix.scene.handle(event);

	// expect teleport event
	auto const& teleports = fix.teleport_sender.data<rpg::TeleportEvent>();
	BOOST_REQUIRE_EQUAL(1u, teleports.size());
	auto const& traps = fix.trap_sender.data<rpg::TrapEvent>();
	BOOST_REQUIRE(traps.empty());
	auto const& tele = teleports[0];
	BOOST_CHECK_EQUAL(data.id, tele.actor);
	BOOST_CHECK_EQUAL(2u, tele.scene);
	BOOST_CHECK_VECTOR_EQUAL(sf::Vector2u(3u, 11u), tele.pos);
}

BOOST_AUTO_TEST_CASE(teleport_is_not_triggered_if_different_tile_reached) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto event = fix.move(data.id, {12u, 6u});
	fix.scene.handle(event);

	// expect no event
	auto const& teleports = fix.teleport_sender.data<rpg::TeleportEvent>();
	BOOST_REQUIRE(teleports.empty());
	auto const& traps = fix.trap_sender.data<rpg::TrapEvent>();
	BOOST_REQUIRE(traps.empty());
}

BOOST_AUTO_TEST_CASE(handling_tile_left_is_not_expected) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto event = fix.move(data.id, {15u, 7u});
	event.type = core::MoveEvent::Left;
	BOOST_REQUIRE_ASSERT(fix.scene.handle(event));
}

BOOST_AUTO_TEST_CASE(trap_is_triggered_on_tile_is_reached) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto event = fix.move(data.id, {14u, 9u});
	fix.scene.handle(event);

	// expect trap event
	auto const& teleports = fix.teleport_sender.data<rpg::TeleportEvent>();
	BOOST_REQUIRE(teleports.empty());
	auto const& traps = fix.trap_sender.data<rpg::TrapEvent>();
	BOOST_REQUIRE_EQUAL(traps.size(), 1u);
	BOOST_CHECK_EQUAL(traps[0].target, data.id);
	BOOST_CHECK_EQUAL(traps[0].trap->damage[rpg::DamageType::Blunt], 5);
}

BOOST_AUTO_TEST_CASE(durable_trigger_can_be_triggered_more_than_once) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto event = fix.move(data.id, {15u, 7u});
	fix.scene.handle(event);
	fix.scene.handle(event);

	// expect teleport event
	auto const& traps = fix.trap_sender.data<rpg::TrapEvent>();
	BOOST_REQUIRE(traps.empty());
	auto const& teleports = fix.teleport_sender.data<rpg::TeleportEvent>();
	BOOST_REQUIRE_EQUAL(2u, teleports.size());
	BOOST_CHECK_EQUAL(data.id, teleports[0].actor);
	BOOST_CHECK_EQUAL(2u, teleports[0].scene);
	BOOST_CHECK_VECTOR_EQUAL(sf::Vector2u(3u, 11u), teleports[0].pos);
	BOOST_CHECK_EQUAL(data.id, teleports[1].actor);
	BOOST_CHECK_EQUAL(2u, teleports[1].scene);
	BOOST_CHECK_VECTOR_EQUAL(sf::Vector2u(3u, 11u), teleports[1].pos);
}

BOOST_AUTO_TEST_CASE(non_durable_trigger_cannot_be_triggered_more_than_once) {
	auto& fix = Singleton<TriggerFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto event = fix.move(data.id, {14u, 9u});
	fix.scene.handle(event);
	fix.scene.handle(event);

	// expect ONE trap event
	auto const& traps = fix.trap_sender.data<rpg::TrapEvent>();
	BOOST_REQUIRE_EQUAL(traps.size(), 1u);
	BOOST_CHECK_EQUAL(traps[0].target, data.id);
	BOOST_CHECK_EQUAL(traps[0].trap->damage[rpg::DamageType::Blunt], 5);
}

BOOST_AUTO_TEST_SUITE_END()
