#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/interact.hpp>

struct InteractFixture {
	core::LogContext log;
	core::IdManager ids;
	std::vector<core::ObjectID> objects;

	core::InputSender input_sender;
	rpg::ItemSender item_sender;
	core::MovementManager movement;
	core::FocusManager focus;
	rpg::PlayerManager player;
	rpg::InteractManager interact;

	rpg::interact_impl::Context context;
	rpg::ItemTemplate foo, bar;

	InteractFixture()
		: log{}
		, ids{}
		, objects{}
		, input_sender{}
		, item_sender{}
		, movement{}
		, focus{}
		, player{}
		, interact{}
		, context{log, input_sender, item_sender, movement, focus, player}
		, foo{}
		, bar{} {}

	rpg::PlayerData& addPlayer(
		sf::Vector2f const& pos, rpg::PlayerID player_id) {
		auto id = ids.acquire();
		objects.push_back(id);
		auto& m = movement.acquire(id);
		m.pos = pos;
		m.target = sf::Vector2u{pos};
		m.look = {1, 0};
		auto& f = focus.acquire(id);
		auto& p = player.acquire(id);
		p.player_id = player_id;
		return p;
	}

	rpg::InteractData& addBarrier(sf::Vector2f const& pos) {
		auto id = ids.acquire();
		objects.push_back(id);
		auto& m = movement.acquire(id);
		m.pos = pos;
		m.target = sf::Vector2u{pos};
		auto& i = interact.acquire(id);
		i.type = rpg::InteractType::Barrier;
		return i;
	}

	rpg::InteractData& addCorpse(sf::Vector2f const& pos) {
		auto id = ids.acquire();
		objects.push_back(id);
		auto& m = movement.acquire(id);
		m.pos = pos;
		m.target = sf::Vector2u{pos};
		auto& i = interact.acquire(id);
		i.type = rpg::InteractType::Corpse;
		return i;
	}

	void reset() {
		for (auto id : objects) {
			movement.release(id);
			if (focus.has(id)) {
				focus.release(id);
			}
			if (player.has(id)) {
				player.release(id);
			}
			if (interact.has(id)) {
				interact.release(id);
			}
		}
		objects.clear();
		ids.reset();
		movement.cleanup();
		focus.cleanup();
		player.cleanup();
		interact.cleanup();

		input_sender.clear();
		item_sender.clear();
	}
};

BOOST_AUTO_TEST_SUITE(interact_test)

BOOST_AUTO_TEST_CASE(can_move_barrier_if_already_moving) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& player = fix.addPlayer({1.6f, 2.f}, 1u);
	auto& barrier = fix.addBarrier({2.f, 2.f});
	barrier.cooldown = rpg::interact_impl::BARRIER_MOVE_COOLDOWN;

	rpg::interact_impl::moveBarrier(fix.context, barrier, player.id);

	auto const& events = fix.context.input_sender.data();
	BOOST_CHECK(!events.empty());
}

BOOST_AUTO_TEST_CASE(cannot_move_barrier_if_too_far_away) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& player = fix.addPlayer({1.4f, 2.f}, 1u);
	auto& barrier = fix.addBarrier({3.f, 2.f});

	rpg::interact_impl::moveBarrier(fix.context, barrier, player.id);

	auto const& events = fix.context.input_sender.data();
	BOOST_CHECK(events.empty());
}

BOOST_AUTO_TEST_CASE(move_barrier_into_looking_direction) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& player = fix.addPlayer({1.6f, 2.f}, 1u);
	auto& barrier = fix.addBarrier({2.f, 2.f});

	rpg::interact_impl::moveBarrier(fix.context, barrier, player.id);

	auto const& events = fix.context.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, barrier.id);
	BOOST_CHECK_VECTOR_EQUAL(events[0].move, sf::Vector2i(1, 0));
	BOOST_CHECK_TIME_EQUAL(barrier.cooldown, rpg::interact_impl::BARRIER_MOVE_COOLDOWN);
}

BOOST_AUTO_TEST_CASE(move_barrier_into_movement_direction) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& player = fix.addPlayer({1.6f, 2.f}, 1u);
	auto& m = fix.movement.query(player.id);
	m.move = {-1, 1};
	auto& barrier = fix.addBarrier({2.f, 2.f});

	rpg::interact_impl::moveBarrier(fix.context, barrier, player.id);

	auto const& events = fix.context.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, barrier.id);
	BOOST_CHECK_VECTOR_EQUAL(events[0].move, sf::Vector2i(-1, 1));
	BOOST_CHECK_TIME_EQUAL(barrier.cooldown, rpg::interact_impl::BARRIER_MOVE_COOLDOWN);
}

BOOST_AUTO_TEST_CASE(move_is_stopped_on_collision) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& player = fix.addPlayer({1.6f, 2.f}, 1u);
	auto& m = fix.movement.query(player.id);
	m.move = {-1, 1};
	auto& barrier = fix.addBarrier({2.f, 2.f});
	rpg::interact_impl::moveBarrier(fix.context, barrier, player.id);
	BOOST_REQUIRE_TIME_EQUAL(barrier.cooldown, rpg::interact_impl::BARRIER_MOVE_COOLDOWN);
	
	// trigger collision
	rpg::interact_impl::onCollision(fix.context, barrier);
	BOOST_CHECK_TIME_EQUAL(barrier.cooldown, sf::Time::Zero);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(non_player_cannot_loot) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& player = fix.addPlayer({1.f, 1.f}, 1u);
	fix.player.release(player.id);
	fix.player.cleanup();
	auto& corpse = fix.addCorpse({2.f, 2.f});
	corpse.loot.resize(1u);
	corpse.loot[0].emplace_back(fix.foo, 5u);
	corpse.loot[0].emplace_back(fix.bar, 3u);

	rpg::interact_impl::lootCorpse(fix.context, corpse, player.id);

	auto const& events = fix.context.item_sender.data();
	BOOST_CHECK(events.empty());

	BOOST_CHECK_EQUAL(corpse.loot[0].size(), 2u);
}

BOOST_AUTO_TEST_CASE(cannot_loot_empty_corpse) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& player = fix.addPlayer({1.f, 1.f}, 1u);
	auto& corpse = fix.addCorpse({2.f, 2.f});
	corpse.loot.resize(1u);

	rpg::interact_impl::lootCorpse(fix.context, corpse, player.id);

	auto const& events = fix.context.item_sender.data();
	BOOST_CHECK(events.empty());
	BOOST_CHECK(corpse.loot[0u].empty());
}

BOOST_AUTO_TEST_CASE(player_can_loot_corpse) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& player = fix.addPlayer({1.f, 1.f}, 1u);
	auto& corpse = fix.addCorpse({2.f, 2.f});
	corpse.loot.resize(1u);
	corpse.loot[0].emplace_back(fix.foo, 5u);
	corpse.loot[0].emplace_back(fix.bar, 3u);

	rpg::interact_impl::lootCorpse(fix.context, corpse, player.id);

	auto const& events = fix.context.item_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].actor, player.id);
	BOOST_CHECK_EQUAL(events[0].item, &fix.foo);
	BOOST_CHECK_EQUAL(events[0].quantity, 5u);
	BOOST_CHECK_EQUAL(events[1].actor, player.id);
	BOOST_CHECK_EQUAL(events[1].item, &fix.bar);
	BOOST_CHECK_EQUAL(events[1].quantity, 3u);

	BOOST_CHECK(corpse.loot[0].empty());
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(moving_barrier_might_stop_on_update) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& barrier = fix.addBarrier({2.f, 2.f});
	barrier.cooldown = rpg::interact_impl::BARRIER_MOVE_COOLDOWN;
	rpg::interact_impl::onUpdate(fix.context, barrier, rpg::interact_impl::BARRIER_MOVE_COOLDOWN);
	BOOST_REQUIRE_TIME_EQUAL(barrier.cooldown, sf::Time::Zero);

	auto const& events = fix.context.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, barrier.id);
	BOOST_CHECK_VECTOR_EQUAL(events[0].move, sf::Vector2i(0, 0));
}

BOOST_AUTO_TEST_CASE(moving_barrier_can_continue_on_update) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& barrier = fix.addBarrier({2.f, 2.f});
	barrier.cooldown = rpg::interact_impl::BARRIER_MOVE_COOLDOWN;
	rpg::interact_impl::onUpdate(fix.context, barrier, sf::milliseconds(10));

	auto const& events = fix.context.input_sender.data();
	BOOST_REQUIRE(events.empty());
}

BOOST_AUTO_TEST_CASE(standing_barrier_cannot_stop_on_update) {
	auto& fix = Singleton<InteractFixture>::get();
	fix.reset();

	auto& barrier = fix.addBarrier({2.f, 2.f});
	barrier.cooldown = sf::Time::Zero;
	rpg::interact_impl::onUpdate(fix.context, barrier, sf::seconds(1.f));
	BOOST_REQUIRE_TIME_EQUAL(barrier.cooldown, sf::Time::Zero);

	auto const& events = fix.context.input_sender.data();
	BOOST_REQUIRE(events.empty());
}

BOOST_AUTO_TEST_SUITE_END()
