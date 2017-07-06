#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <game/script.hpp>
#include <game/lua.hpp>
#include <game/path.hpp>
#include <game/resources.hpp>

struct ScriptFixture {
	core::IdManager id_manager;

	core::LogContext log;
	game::ScriptManager script_manager;
	game::script_impl::Context context;

	// for LuaApi creation
	core::DungeonSystem dungeon;
	core::CameraSystem camera;
	core::MovementManager movement;
	core::CollisionManager collision;
	core::FocusManager focus;
	core::AnimationManager animation;
	core::RenderManager render;
	core::SoundManager sound;
	rpg::StatsManager stats;
	rpg::EffectManager effect_manager;
	rpg::ItemManager item;
	rpg::PerkManager perk;
	rpg::PlayerManager player;
	rpg::ProjectileManager projectile;
	rpg::ActionManager action;
	rpg::InputManager input;
	rpg::InteractManager interact;
	rpg::QuickslotManager quickslot;
	rpg::Session session;
	core::InputSender input_sender;
	rpg::ActionSender action_sender;
	rpg::ItemSender item_sender;
	game::PathSystem pathfinder;
	game::ScriptManager scriptman;

	utils::Script script;
	game::ScriptData& data;

	rpg::EffectTemplate effect;

	ScriptFixture()
		: id_manager{}
		, log{}
		, script_manager{}
		, context{log, script_manager}  // -- for LuaApi Creation
		, dungeon{}
		, camera{{320u, 240u}}
		, movement{}
		, collision{}
		, focus{}
		, animation{}
		, render{}
		, sound{}
		, stats{}
		, effect_manager{}
		, item{}
		, perk{}
		, player{}
		, projectile{}
		, action{}
		, input{}
		, interact{}
		, quickslot{}
		, session{id_manager, dungeon, camera, movement, collision, focus,
			  animation, render, sound, stats, effect_manager, item, perk,
			  player, projectile, action, input, interact, quickslot}
		, input_sender{}
		, action_sender{}
		, item_sender{}
		, pathfinder{log}
		, scriptman{}
		, script{}
		, data{script_manager.acquire(1u)}
		, effect{} {
		utils::bindAll(script);
		std::string luacode = 
			"called = '';\n"
			"args = {};\n"
			"\n"
			"onInit = function(self)\n"
			"	called = 'onInit';\n"
			"end\n"
			"onTeleport = function(self, src_scene, src_pos, dst_scene, dst_pos)\n"
			"	called = 'onTeleport'\n"
			"	args = {\n"
			"		src_scene = src_scene,\n"
			"		src_pos = src_pos,\n"
			"		dst_scene = dst_scene,\n"
			"		dst_pos = dst_pos\n"
			"	};\n"
			"end\n"
			"\n"
			"onObjectCollision = function(self, other, pos)\n"
			"	called = 'onObjectCollision';\n"
			"	args = {\n"
			"		other = other,\n"
			"		pos = pos\n"
			"	};\n"
			"end\n"
			"\n"
			"onTileCollision = function(self, pos)\n"
			"	called = 'onTileCollision';\n"
			"	args = {\n"
			"		pos = pos\n"
			"	};\n"
			"end\n"
			"\n"
			"onIdle = function(self)\n"
			"	called = 'onIdle';\n"
			"	args = {};\n"
			"end\n"
			"\n"
			"onTileLeft = function(self, pos)\n"
			"	called = 'onTileLeft';\n"
			"	args = {\n"
			"		pos = pos\n"
			"	};\n"
			"end\n"
			"\n"
			"onTileReached = function(self, pos)\n"
			"	called = 'onTileReached';\n"
			"	args = {\n"
			"		pos = pos\n"
			"	};\n"
			"end\n"
			"\n"
			"onGotFocus = function(self, target)\n"
			"	called = 'onGotFocus';\n"
			"	args = {\n"
			"		target = target\n"
			"	};\n"
			"end\n"
			"\n"
			"onLostFocus = function(self, target)\n"
			"	called = 'onLostFocus';\n"
			"	args = {\n"
			"		target = target\n"
			"	}\n"
			"end\n"
			"\n"
			"onWasFocused = function(self, observer)\n"
			"	called = 'onWasFocused';\n"
			"	args = {\n"
			"		observer = observer\n"
			"	};\n"
			"end\n"
			"\n"
			"onWasUnfocused = function(self, observer)\n"
			"	called = 'onWasUnfocused';\n"
			"	args = {\n"
			"		observer = observer\n"
			"	};\n"
			"end\n"
			"\n"
			"onEffectReceived = function(self, effect, causer)\n"
			"	called = 'onEffectReceived';\n"
			"	args = {\n"
			"		effect = effect.internal_name,\n"
			"		causer = causer\n"
			"	};\n"
			"end\n"
			"\n"
			"onEffectInflicted = function(self, effect, target)\n"
			"	called = 'onEffectInflicted';\n"
			"	args = {\n"
			"		effect = effect.internal_name,\n"
			"		target = target\n"
			"	};\n"
			"end\n"
			"\n"
			"onEffectFaded = function(self, effect)\n"
			"	called = 'onEffectFaded';\n"
			"	args = {\n"
			"		effect = effect.internal_name\n"
			"	};\n"
			"end\n"
			"\n"
			"onStatsReceived = function(self, life, mana, stamina, causer)\n"
			"	called = 'onStatsReceived';\n"
			"	args = {\n"
			"		life = life,\n"
			"		mana = mana,\n"
			"		stamina = stamina,\n"
			"		causer = causer\n"
			"	};\n"
			"end\n"
			"\n"
			"onStatsInflicted = function(self, life, mana, stamina, target)\n"
			"	called = 'onStatsInflicted';\n"
			"	args = {\n"
			"		life = life,\n"
			"		mana = mana,\n"
			"		stamina = stamina,\n"
			"		target = target\n"
			"	};\n"
			"end\n"
			"\n"
			"onEnemyKilled = function(self, target)\n"
			"	called = 'onEnemyKilled';\n"
			"	args = {\n"
			"		target = target\n"
			"	};\n"
			"end\n"
			"\n"
			"onDeath = function(self, enemy)\n"
			"	called = 'onDeath';\n"
			"	args = {\n"
			"		enemy = enemy\n"
			"	};\n"
			"end\n"
			"\n"
			"onSpawned = function(self, causer)\n"
			"	called = 'onSpawned';\n"
			"	args = {\n"
			"		causer = causer\n"
			"	};\n"
			"end\n"
			"\n"
			"onCausedSpawn = function(self, allied)\n"
			"	called = 'onCausedSpawn';\n"
			"	args = {\n"
			"		allied = allied\n"
			"	};\n"
			"end\n"
			"\n"
			"onFeedback = function(self, type)\n"
			"	called = 'onFeedback';\n"
			"	args = {\n"
			"		type = type\n"
			"	};\n"
			"end\n"
			"\n"
			"onPathFailed = function(self, pos)\n"
			"	called = 'onPathFailed';\n"
			"	args = {\n"
			"		pos = pos\n"
			"	};\n"
			"end\n"
			"\n"
			"onUpdate = function(self)\n"
			"	called = 'onUpdate';\n"
			"	args = {};\n"
			"end";
		BOOST_REQUIRE(script.loadFromMemory(luacode));
		
		core::ObjectID id{1u};
		movement.acquire(id);
		data.api = std::make_unique<game::LuaApi>(log, id, true, session,
			scriptman, input_sender, action_sender, item_sender, pathfinder);
		data.script = &script;
		
		effect.internal_name = "dummy";
	}

	void reset() {
		data.is_active = true;
		script("onInit", data.api.get());
	}
};

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(script_test)

BOOST_AUTO_TEST_CASE(creation_triggers_onInit) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onInit");
}

BOOST_AUTO_TEST_CASE(teleport_triggers_onInit) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::TeleportEvent event;
	event.actor = fix.data.id;
	event.src_scene = 2u;
	event.src_pos = {3u, 4u};
	event.dst_scene = 5u;
	event.dst_pos = {6u, 7u};
	game::script_impl::onTeleport(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onTeleport");
	BOOST_CHECK_EQUAL(args["src_scene"].get<utils::SceneID>(), event.src_scene);
	BOOST_CHECK_VECTOR_EQUAL(args["src_pos"].get<sf::Vector2u>(), event.src_pos);
	BOOST_CHECK_EQUAL(args["dst_scene"].get<utils::SceneID>(), event.dst_scene);
	BOOST_CHECK_VECTOR_EQUAL(args["dst_pos"].get<sf::Vector2u>(), event.dst_pos);
}

BOOST_AUTO_TEST_CASE(object_collision_triggers_onObjectCollision) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::CollisionEvent event;
	event.actor = fix.data.id;
	event.collider = 2u;
	event.pos = {3u, 2u};
	game::script_impl::onCollision(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onObjectCollision");
	BOOST_CHECK_EQUAL(args["other"].get<int>(), event.collider);
	BOOST_CHECK_VECTOR_EQUAL(args["pos"].get<sf::Vector2u>(), event.pos);
}

BOOST_AUTO_TEST_CASE(tile_collision_triggers_onTileCollision) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::CollisionEvent event;
	event.actor = fix.data.id;
	event.collider = 0u;
	event.pos = {2u, 4u};
	game::script_impl::onCollision(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onTileCollision");
	BOOST_CHECK_VECTOR_EQUAL(args["pos"].get<sf::Vector2u>(), event.pos);
}

BOOST_AUTO_TEST_CASE(idle_animation_triggers_onIdle) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	game::script_impl::onIdle(fix.context, fix.data);
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onIdle");
}

BOOST_AUTO_TEST_CASE(leaving_a_tile_triggers_onTileLeft) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::MoveEvent event;
	event.actor = fix.data.id;
	event.type = core::MoveEvent::Left;
	event.source = {2, 3};
	game::script_impl::onMove(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onTileLeft");
	BOOST_CHECK_VECTOR_EQUAL(args["pos"].get<sf::Vector2u>(), event.source);
}

BOOST_AUTO_TEST_CASE(reaching_a_tile_triggers_onTileReached) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::MoveEvent event;
	event.actor = fix.data.id;
	event.type = core::MoveEvent::Reached;
	event.target = {1, 2};
	game::script_impl::onMove(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onTileReached");
	BOOST_CHECK_VECTOR_EQUAL(args["pos"].get<sf::Vector2u>(), event.target);
}

BOOST_AUTO_TEST_CASE(focusing_triggers_onGotFocus) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::FocusEvent event;
	event.observer = fix.data.id;
	event.observed = 2u;
	event.type = core::FocusEvent::Gained;
	game::script_impl::onFocus(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onGotFocus");
	BOOST_CHECK_EQUAL(args["target"].get<int>(), event.observed);
}

BOOST_AUTO_TEST_CASE(unfocusing_triggers_onLostFocus) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::FocusEvent event;
	event.observer = fix.data.id;
	event.observed = 2u;
	event.type = core::FocusEvent::Lost;
	game::script_impl::onFocus(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onLostFocus");
	BOOST_CHECK_EQUAL(args["target"].get<int>(), event.observed);
}

BOOST_AUTO_TEST_CASE(being_focused_triggers_onGotFocus) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::FocusEvent event;
	event.observer = 2u;
	event.observed = fix.data.id;
	event.type = core::FocusEvent::Gained;
	game::script_impl::onFocus(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onWasFocused");
	BOOST_CHECK_EQUAL(args["observer"].get<int>(), event.observer);
}

BOOST_AUTO_TEST_CASE(being_unfocused_triggers_onLostFocus) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	core::FocusEvent event;
	event.observer = 2u;
	event.observed = fix.data.id;
	event.type = core::FocusEvent::Lost;
	game::script_impl::onFocus(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onWasUnfocused");
	BOOST_CHECK_EQUAL(args["observer"].get<int>(), event.observer);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(receiving_effect_triggers_onEffectReceived) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::EffectEvent event;
	event.actor = fix.data.id;
	event.causer = 2u;
	event.effect = &fix.effect;
	event.type = rpg::EffectEvent::Add;
	game::script_impl::onEffect(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onEffectReceived");
	BOOST_CHECK_EQUAL(args["effect"].get<std::string>(), event.effect->internal_name);
	BOOST_CHECK_EQUAL(args["causer"].get<int>(), event.causer);
}

BOOST_AUTO_TEST_CASE(inflicting_effect_triggers_onEffectInflicted) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::EffectEvent event;
	event.actor = 2u;
	event.causer = fix.data.id;
	event.effect = &fix.effect;
	event.type = rpg::EffectEvent::Add;
	game::script_impl::onEffect(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onEffectInflicted");
	BOOST_CHECK_EQUAL(
		args["effect"].get<std::string>(), event.effect->internal_name);
	BOOST_CHECK_EQUAL(args["target"].get<int>(), event.actor);
}

BOOST_AUTO_TEST_CASE(fading_effect_triggers_onEffectFaded) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::EffectEvent event;
	event.actor = fix.data.id;
	event.causer = 0u;
	event.effect = &fix.effect;
	event.type = rpg::EffectEvent::Remove;
	game::script_impl::onEffect(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onEffectFaded");
	BOOST_CHECK_EQUAL(
		args["effect"].get<std::string>(), event.effect->internal_name);
}

BOOST_AUTO_TEST_CASE(receiving_stats_triggers_onStatsReceived) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::StatsEvent event;
	event.actor = fix.data.id;
	event.causer = 2u;
	event.delta[rpg::Stat::Life] = 13;
	event.delta[rpg::Stat::Mana] = 14;
	event.delta[rpg::Stat::Stamina] = 15;
	game::script_impl::onStats(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onStatsReceived");
	BOOST_CHECK_EQUAL(args["life"].get<int>(), event.delta[rpg::Stat::Life]);
	BOOST_CHECK_EQUAL(args["mana"].get<int>(), event.delta[rpg::Stat::Mana]);
	BOOST_CHECK_EQUAL(
		args["stamina"].get<int>(), event.delta[rpg::Stat::Stamina]);
	BOOST_CHECK_EQUAL(args["causer"].get<int>(), event.causer);
}

BOOST_AUTO_TEST_CASE(inflicting_stats_triggers_onStatsInflicted) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::StatsEvent event;
	event.actor = 2u;
	event.causer = fix.data.id;
	event.delta[rpg::Stat::Life] = 13;
	event.delta[rpg::Stat::Mana] = 14;
	event.delta[rpg::Stat::Stamina] = 15;
	game::script_impl::onStats(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onStatsInflicted");
	BOOST_CHECK_EQUAL(args["life"].get<int>(), event.delta[rpg::Stat::Life]);
	BOOST_CHECK_EQUAL(args["mana"].get<int>(), event.delta[rpg::Stat::Mana]);
	BOOST_CHECK_EQUAL(
		args["stamina"].get<int>(), event.delta[rpg::Stat::Stamina]);
	BOOST_CHECK_EQUAL(args["target"].get<int>(), event.actor);
}

BOOST_AUTO_TEST_CASE(killing_enemy_triggers_onEnemyKilled) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::DeathEvent event;
	event.actor = 2u;
	event.causer = fix.data.id;
	game::script_impl::onDeath(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onEnemyKilled");
	BOOST_CHECK_EQUAL(args["target"].get<int>(), event.actor);
}

BOOST_AUTO_TEST_CASE(becoming_killed_triggers_onDeath) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::DeathEvent event;
	event.actor = 1u;
	event.causer = 2u;
	game::script_impl::onDeath(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onDeath");
	BOOST_CHECK_EQUAL(args["enemy"].get<int>(), event.causer);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getting_respawned_triggers_onSpawned) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::SpawnEvent event;
	event.actor = fix.data.id;
	event.causer = 2u;
	game::script_impl::onSpawn(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onSpawned");
	BOOST_CHECK_EQUAL(args["causer"].get<int>(), event.causer);
}

BOOST_AUTO_TEST_CASE(respawn_somebody_triggers_onCausedSpawn) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::SpawnEvent event;
	event.actor = 2u;
	event.causer = fix.data.id;
	game::script_impl::onSpawn(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onCausedSpawn");
	BOOST_CHECK_EQUAL(args["allied"].get<int>(), event.actor);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(feedback_somebody_triggers_onFeedback) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	rpg::FeedbackEvent event;
	event.actor = fix.data.id;
	event.type = rpg::FeedbackType::NotEnoughMana;
	game::script_impl::onFeedback(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onFeedback");
	BOOST_CHECK(args["type"].get<rpg::FeedbackType>() == event.type);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(failed_pathfinding_causes_onPathFailed) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	game::PathFailedEvent event;
	event.actor = fix.data.id;
	event.pos = {1u, 2u};
	game::script_impl::onPathFailed(fix.context, event);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onPathFailed");
	BOOST_CHECK_VECTOR_EQUAL(args["pos"].get<sf::Vector2u>(), event.pos);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cycling_update_triggers_onUpdate) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	game::script_impl::onUpdate(fix.context, fix.data);
	auto args = fix.script.get("args");
	BOOST_REQUIRE_EQUAL(fix.script.get<std::string>("called"), "onUpdate");
}

/*
BOOST_AUTO_TEST_CASE(updates_do_not_take_place_each_frame) {
	auto& fix = Singleton<ScriptFixture>::get();
	fix.reset();

	game::script_impl::update(fix.context, sf::milliseconds(300u));
	BOOST_CHECK_EQUAL(fix.context.update_delay.asMilliseconds(), 100u);
	game::script_impl::update(fix.context, sf::milliseconds(80u));
	BOOST_CHECK_EQUAL(fix.context.update_delay.asMilliseconds(), 180u);
}
*/

BOOST_AUTO_TEST_SUITE_END()
