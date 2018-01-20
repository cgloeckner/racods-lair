#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <utils/math2d.hpp>
#include <rpg/balance.hpp>
#include <rpg/combat.hpp>
#include <rpg/item.hpp>
#include <game/factory.hpp>
#include <game/navigator.hpp>
#include <game/lua.hpp>

struct LuaPathDummy : game::PathSystem {
	bool called;

	LuaPathDummy(core::LogContext& log)
		: game::PathSystem{log}
		, called{false} {
	}

	std::future<game::Path> schedule(core::ObjectID actor, utils::SceneID scene, sf::Vector2u const& source,
		sf::Vector2u const& target) override {
		called = true;
		game::Path path;
		path.push_back(target);
		path.emplace_back();  // dummy as well
		path.push_back(source);
		std::promise<game::Path> p;
		p.set_value(path);
		return p.get_future();
	}
};

struct LuaFixture
	: utils::EventListener<core::InputEvent, rpg::ActionEvent, rpg::ItemEvent> {
	core::LogContext log;
	sf::Texture dummy;
	std::vector<core::ObjectID> objects;
	core::IdManager id_manager;

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
	game::AudioSystem audio;
	game::DungeonGenerator generator;
	game::NavigationSystem navigator;
	LuaPathDummy pathfinder;
	game::ScriptManager script;
	game::HudManager hud;
	game::Session session;
	game::ResourceCache cache;
	game::Mod mod;
	game::Factory factory;

	std::vector<core::InputEvent> input_events;
	std::vector<rpg::ActionEvent> action_events;
	std::vector<rpg::ItemEvent> item_events;

	game::PlayerTemplate player_tpl;
	game::BotTemplate bot_tpl;
	game::RoomTemplate room_tpl;
	rpg::SpriteTemplate sprite_tpl;
	rpg::EntityTemplate entity_tpl;
	rpg::ItemTemplate sword_tpl, bow_tpl, armor_tpl, potion_tpl;
	rpg::PerkTemplate fireball_tpl, heal_tpl;
	rpg::Keybinding keys_tpl;
	utils::Script script_dummy;

	LuaFixture()
		: utils::EventListener<core::InputEvent, rpg::ActionEvent,
			rpg::ItemEvent>{}
		, log{}
		, dummy{}
		, objects{}
		, id_manager{}
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
		, audio{log, movement.capacity(), item, player}
		, generator{log}
		, navigator{}
		, pathfinder{log}
		, script{}
		, hud{}
		, session{id_manager, dungeon, camera, movement, collision, focus,
			  animation, render, stats, effect_manager, item, perk,
			  player, projectile, action, input, interact, quickslot,
			  audio, generator, navigator, script, hud, pathfinder}
		, cache{}
		, mod{log, cache, ""}
		, factory{log, session, mod}
		, input_events{}
		, action_events{}
		, player_tpl{}
		, bot_tpl{}
		, room_tpl{}
		, sprite_tpl{}
		, entity_tpl{}
		, sword_tpl{}
		, bow_tpl{}
		, armor_tpl{}
		, potion_tpl{}
		, fireball_tpl{}
		, heal_tpl{}
		, keys_tpl{}
		, script_dummy{} {
		factory.bind<core::InputEvent>(*this);
		factory.bind<rpg::ActionEvent>(*this);
		factory.bind<rpg::ItemEvent>(*this);

		utils::bindAll(script_dummy);
		script_dummy.loadFromMemory("onInit = function(self)\nend\n");

		rpg::TilesetTemplate tileset;
		tileset.tileset_name = "demo";
		tileset.tilesize = {16u, 16u};
		tileset.floors.emplace_back(0u, 0u);  // texture offset
		tileset.walls.emplace_back(16u, 0u);  // texture offset
		tileset.tileset = &dummy;
		generator.rooms.push_back(&room_tpl);
		generator.settings.cell_size = 10u;
		thor::setRandomSeed(0); // note: make generation predictable
		game::BuildSettings settings;
		settings.path_width = 2u;
		auto id = factory.createDungeon(tileset, {30u, 10u}, settings);
		BOOST_REQUIRE_EQUAL(id, 1u);
	}

	core::ObjectID createPlayer(sf::Vector2u const& pos) {
		rpg::SpawnMetaData spawn;
		spawn.scene = 1u;
		spawn.pos = pos;
		spawn.direction = {1, 0};
		player_tpl.level = 12u;
		player_tpl.exp = rpg::getNextExp(player_tpl.level);
		player_tpl.attributes[rpg::Attribute::Strength] = 25u;
		player_tpl.attributes[rpg::Attribute::Dexterity] = 55u;
		player_tpl.attributes[rpg::Attribute::Wisdom] = 10u;
		auto id = factory.createPlayer(player_tpl, keys_tpl, spawn);
		objects.push_back(id);
		return id;
	}

	game::LuaApi& createBot(sf::Vector2u const& pos, bool hostile) {
		rpg::SpawnMetaData spawn;
		spawn.scene = 1u;
		spawn.pos = pos;
		spawn.direction = {1, 0};
		auto id = factory.createBot(bot_tpl, spawn, 1u, script_dummy, hostile);
		objects.push_back(id);
		auto& s = session.script.query(id);
		return *s.api.get();
	}

	void handle(core::InputEvent const& event) {
		input_events.push_back(event);
	}

	void handle(rpg::ActionEvent const& event) {
		action_events.push_back(event);
	}

	void handle(rpg::ItemEvent const& event) {
		item_events.push_back(event);
	}

	void update() {
		factory.update(sf::Time::Zero);

		dispatch<core::InputEvent>(*this);
		dispatch<rpg::ActionEvent>(*this);
		dispatch<rpg::ItemEvent>(*this);
	}

	void cleanup() {
		for (auto ptr : session.systems) {
			ptr->cleanup();
		}
	}

	void reset() {
		sprite_tpl = rpg::SpriteTemplate{};
		sprite_tpl.frameset = &dummy;
		sprite_tpl.torso[core::AnimationAction::Idle].frames.resize(1u);
		entity_tpl = rpg::EntityTemplate{};
		entity_tpl.sprite = &sprite_tpl;
		entity_tpl.max_sight = 5.f;
		entity_tpl.fov = 120.f;
		entity_tpl.display_name = "foo";
		entity_tpl.collide = true;
		bot_tpl = game::BotTemplate{};
		bot_tpl.entity = &entity_tpl;
		player_tpl = game::PlayerTemplate{};
		player_tpl.entity = &entity_tpl;
		sword_tpl = rpg::ItemTemplate{};
		sword_tpl.type = rpg::ItemType::Weapon;
		sword_tpl.slot = rpg::EquipmentSlot::Weapon;
		sword_tpl.melee = true;
		sword_tpl.two_handed = true;
		sword_tpl.damage[rpg::DamageType::Blade] = 10u;
		bow_tpl = rpg::ItemTemplate{};
		bow_tpl.type = rpg::ItemType::Weapon;
		bow_tpl.slot = rpg::EquipmentSlot::Weapon;
		bow_tpl.melee = false;
		bow_tpl.two_handed = true;
		bow_tpl.damage[rpg::DamageType::Bullet] = 15u;
		armor_tpl = rpg::ItemTemplate{};
		armor_tpl.type = rpg::ItemType::Armor;
		potion_tpl = rpg::ItemTemplate{};
		potion_tpl.type = rpg::ItemType::Potion;
		fireball_tpl = rpg::PerkTemplate{};
		fireball_tpl.type = rpg::PerkType::Enemy;
		fireball_tpl.damage[rpg::DamageType::Fire] = 10u;
		heal_tpl = rpg::PerkTemplate{};
		heal_tpl.type = rpg::PerkType::Allied;
		heal_tpl.revive = true;
		heal_tpl.recover[rpg::Stat::Life] = 20u;

		// reset dungeon
		auto& d = dungeon[1u];
		for (auto y = 0u; y < 10u; ++y) {
			for (auto x = 0u; x < 30u; ++x) {
				auto& c = d.getCell({x, y});
				c.trigger = nullptr;
				c.entities.clear();
			}
		}
		// reset objects
		for (auto id : objects) {
			for (auto ptr : session.systems) {
				ptr->tryRelease(id);
			}
		}
		objects.clear();
		id_manager.reset();
		factory.reset();
		pathfinder.called = false;
		// reset events
		update();
		cleanup();
		input_events.clear();
		action_events.clear();
		item_events.clear();
	}
};

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(lua_test)

BOOST_AUTO_TEST_CASE(isHostile_returns_hostile_flag_on_bots) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();
	
	auto& bot = fix.createBot({1u, 2u}, true);
	auto& minion = fix.createBot({1u, 2u}, false);
	auto player_id = fix.createPlayer({4u, 2u});
	
	BOOST_CHECK(bot.isHostile(bot.id));
	BOOST_CHECK(!bot.isHostile(minion.id));
	BOOST_CHECK(!bot.isHostile(player_id));
}

BOOST_AUTO_TEST_CASE(getMove_returns_others_move_direction) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();
	
	auto& bot = fix.createBot({1u, 2u}, true);
	auto player_id = fix.createPlayer({4u, 2u});
	auto& player_move = fix.session.movement.query(player_id);
	player_move.move = {-1, 1};

	auto vector = bot.getMove(player_id);
	BOOST_CHECK_VECTOR_EQUAL(player_move.move, vector);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getPosition_returns_objects_tile_pos) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({1u, 2u}, true);
	auto pos = bot.getPosition(bot.id);
	BOOST_CHECK_VECTOR_EQUAL(pos, sf::Vector2u(1u, 2u));
}

BOOST_AUTO_TEST_CASE(getScene_returns_objects_scene_id) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({1u, 2u}, true);
	BOOST_CHECK_EQUAL(bot.getScene(bot.id), 1u);
}

BOOST_AUTO_TEST_CASE(getDirection_returns_valid_direction) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({3u, 2u}, true);
	auto& other = fix.createBot({1u, 5u}, true);
	auto dir = bot.getDirection(other.id);
	auto dir2 = other.getDirection(bot.id);
	BOOST_CHECK_VECTOR_EQUAL(dir, sf::Vector2i(-1, 1));
	BOOST_CHECK_VECTOR_EQUAL(dir, sf::Vector2i(-dir2.x, -dir2.y));
}

BOOST_AUTO_TEST_CASE(getFocus_returns_actors_focus_if_set) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({0u, 0u}, true);
	auto& other = fix.createBot({1u, 0u}, true);
	auto focus = bot.getFocus();
	BOOST_CHECK_EQUAL(focus, other.id);
}

BOOST_AUTO_TEST_CASE(getFocus_returns_zero_if_no_focus) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({0u, 0u}, true);
	auto focus = bot.getFocus();
	BOOST_CHECK_EQUAL(focus, 0u);
}

BOOST_AUTO_TEST_CASE(getDistance_calculates_referring_to_beeline) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({3u, 5u});
	auto& move = fix.session.movement.query(id);
	move.pos.x -= 0.1f;
	move.pos.y -= 0.3f;
	auto& bot = fix.createBot({2u, 3u}, true);
	auto dist = bot.getDistance(id);
	auto expected = std::sqrt(utils::distance(move.pos, {2.f, 3.f}));
	BOOST_CHECK_CLOSE(dist, expected, 0.0001f);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getSight_returns_object_sight_range) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 3u}, true);
	BOOST_CHECK_CLOSE(bot.getSight(), fix.entity_tpl.max_sight, 0.0001f);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getEnemies_on_minion_returns_all_hostile_bots_in_sight) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto player_id = fix.createPlayer({2u, 3u});
	auto& actor = fix.createBot({2u, 2u}, false);
	auto& minion = fix.createBot({3u, 2u}, false);
	auto& enemy = fix.createBot({4u, 2u}, true);
	auto& other = fix.createBot({6u, 6u}, true);
	auto enemies = actor.getEnemies();
	BOOST_CHECK(utils::contains(enemies, minion.id));
	BOOST_CHECK(utils::contains(enemies, enemy.id));
	BOOST_CHECK(!utils::contains(enemies, player_id));
	BOOST_CHECK(!utils::contains(enemies, other.id));
}

BOOST_AUTO_TEST_CASE(
	getEnemies_on_hostile_returns_all_players_and_minions_in_sight) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& actor = fix.createBot({2u, 2u}, true);
	auto player_id = fix.createPlayer({2u, 3u});
	auto other_id = fix.createPlayer({5u, 7u});
	auto& minion = fix.createBot({2u, 4u}, false);
	auto& far = fix.createBot({7u, 7u}, false);
	auto& bot = fix.createBot({1u, 2u}, true);
	auto& p = fix.session.player.query(player_id);
	p.minions.push_back(minion.id);
	p.minions.push_back(far.id);
	auto enemies = actor.getEnemies();
	BOOST_CHECK(utils::contains(enemies, player_id));
	BOOST_CHECK(!utils::contains(enemies, other_id));
	BOOST_CHECK(utils::contains(enemies, minion.id));
	BOOST_CHECK(!utils::contains(enemies, far.id));
	BOOST_CHECK(!utils::contains(enemies, bot.id));
}

BOOST_AUTO_TEST_CASE(getAllies_on_minion_returns_players_minions_in_sight) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto player_id = fix.createPlayer({2u, 3u});
	auto& actor = fix.createBot({2u, 2u}, false);
	auto& minion = fix.createBot({3u, 2u}, false);
	auto& bot = fix.createBot({4u, 2u}, true);
	auto& other = fix.createBot({6u, 6u}, true);
	auto& far = fix.createBot({7u, 7u}, false);
	auto& p = fix.session.player.query(player_id);
	p.minions.push_back(minion.id);
	p.minions.push_back(far.id);
	auto allies = actor.getAllies();
	BOOST_CHECK(!utils::contains(allies, bot.id));
	BOOST_CHECK(utils::contains(allies, minion.id));
	BOOST_CHECK(utils::contains(allies, player_id));
	BOOST_CHECK(!utils::contains(allies, other.id));
	BOOST_CHECK(!utils::contains(allies, far.id));
}

BOOST_AUTO_TEST_CASE(getAllies_on_hostile_returns_all_hostiles_in_sight) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& actor = fix.createBot({2u, 2u}, true);
	auto& bot = fix.createBot({2u, 3u}, true);
	auto& out = fix.createBot({5u, 7u}, true);
	auto player_id = fix.createPlayer({2u, 4u});
	auto& minion = fix.createBot({1u, 2u}, false);
	auto& p = fix.session.player.query(player_id);
	p.minions.push_back(minion.id);
	auto allies = actor.getAllies();
	BOOST_CHECK(utils::contains(allies, bot.id));
	BOOST_CHECK(!utils::contains(allies, out.id));
	BOOST_CHECK(!utils::contains(allies, player_id));
	BOOST_CHECK(!utils::contains(allies, minion.id));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getStats_returns_entire_StatsData) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto const& stats = bot.getStats();
	BOOST_CHECK(&stats == &fix.session.stats.query(bot.id));
}

BOOST_AUTO_TEST_CASE(isAlive_works_for_characters) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto& other = fix.createBot({3u, 2u}, true);
	auto& st = fix.session.stats.query(other.id);
	st.stats[rpg::Stat::Life] = 0u;
	BOOST_REQUIRE(!bot.isAlive(other.id));
	st.stats[rpg::Stat::Life] = 2u;
	BOOST_REQUIRE(bot.isAlive(other.id));
}

BOOST_AUTO_TEST_CASE(isAlive_works_for_all_objects) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	// not alive because there is no such stats component
	// (because that object doesn't exist)
	BOOST_REQUIRE(!bot.isAlive(bot.id + 1u));
}

BOOST_AUTO_TEST_CASE(getWeaponDamage_calls_combat_impl_function) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto damage = bot.getWeaponDamage(fix.sword_tpl);
	auto expect = rpg::combat_impl::getWeaponDamage(
		fix.session.stats.query(bot.id), fix.sword_tpl);
	for (auto const& pair : damage) {
		BOOST_REQUIRE_EQUAL(pair.second, expect[pair.first]);
	}
}

BOOST_AUTO_TEST_CASE(getPerkDamage_calls_combat_impl_function) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto damage = bot.getPerkDamage(fix.fireball_tpl);
	auto expect =
		rpg::combat_impl::getPerkDamage(fix.session.perk.query(bot.id),
			fix.session.stats.query(bot.id), fix.fireball_tpl);
	for (auto const& pair : damage) {
		BOOST_REQUIRE_EQUAL(pair.second, expect[pair.first]);
	}
}

BOOST_AUTO_TEST_CASE(getPerkRecovery_calls_combat_impl_function) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto recover = bot.getPerkRecovery(fix.heal_tpl);
	auto expect =
		rpg::combat_impl::getPerkRecovery(fix.session.perk.query(bot.id),
			fix.session.stats.query(bot.id), fix.heal_tpl);
	for (auto const& pair : recover) {
		BOOST_REQUIRE_EQUAL(pair.second, expect[pair.first]);
	}
}

BOOST_AUTO_TEST_CASE(getEquipment_returns_item_in_slot) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto& item = fix.item.query(bot.id);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.bow_tpl;
	auto ptr = bot.getEquipment(rpg::EquipmentSlot::Weapon);
	BOOST_CHECK_EQUAL(ptr, &fix.bow_tpl);
}

BOOST_AUTO_TEST_CASE(getWeapons_returns_all_weapon_type_items) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto& item = fix.item.query(bot.id);
	rpg::item_impl::addItem(item, fix.sword_tpl, 2u);
	rpg::item_impl::addItem(item, fix.bow_tpl, 1u);
	rpg::item_impl::addItem(item, fix.armor_tpl, 2u);
	rpg::item_impl::addItem(item, fix.potion_tpl, 7u);
	auto weapons = bot.getWeapons();
	BOOST_REQUIRE_EQUAL(weapons.size(), 2u);
	BOOST_CHECK_EQUAL(weapons[0].item, &fix.sword_tpl);
	BOOST_CHECK_EQUAL(weapons[0].quantity, 2u);
	BOOST_CHECK_EQUAL(weapons[1].item, &fix.bow_tpl);
	BOOST_CHECK_EQUAL(weapons[1].quantity, 1u);
}

BOOST_AUTO_TEST_CASE(getArmors_returns_all_armor_type_items) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto& item = fix.item.query(bot.id);
	rpg::item_impl::addItem(item, fix.sword_tpl, 2u);
	rpg::item_impl::addItem(item, fix.bow_tpl, 1u);
	rpg::item_impl::addItem(item, fix.armor_tpl, 2u);
	rpg::item_impl::addItem(item, fix.potion_tpl, 7u);
	auto armors = bot.getArmors();
	BOOST_REQUIRE_EQUAL(armors.size(), 1u);
	BOOST_CHECK_EQUAL(armors[0].item, &fix.armor_tpl);
	BOOST_CHECK_EQUAL(armors[0].quantity, 2u);
}

BOOST_AUTO_TEST_CASE(getPotions_returns_all_armor_type_items) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto& item = fix.item.query(bot.id);
	rpg::item_impl::addItem(item, fix.sword_tpl, 2u);
	rpg::item_impl::addItem(item, fix.bow_tpl, 1u);
	rpg::item_impl::addItem(item, fix.armor_tpl, 2u);
	rpg::item_impl::addItem(item, fix.potion_tpl, 7u);
	auto potions = bot.getPotions();
	BOOST_REQUIRE_EQUAL(potions.size(), 1u);
	BOOST_CHECK_EQUAL(potions[0].item, &fix.potion_tpl);
	BOOST_CHECK_EQUAL(potions[0].quantity, 7u);
}

BOOST_AUTO_TEST_CASE(getPerk_returns_all_learned_perks) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto& perk = fix.perk.query(bot.id);
	perk.perks.emplace_back(fix.fireball_tpl, 3u);
	perk.perks.emplace_back(fix.heal_tpl, 8u);
	auto perks = bot.getPerks();
	BOOST_REQUIRE_EQUAL(perks.size(), 2u);
	BOOST_CHECK_EQUAL(perks[0].perk, &fix.fireball_tpl);
	BOOST_CHECK_EQUAL(perks[0].level, 3u);
	BOOST_CHECK_EQUAL(perks[1].perk, &fix.heal_tpl);
	BOOST_CHECK_EQUAL(perks[1].level, 8u);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(navigation_triggers_pathfinding_request) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.navigate({4u, 5u});
	bot.update(sf::Time::Zero);
	BOOST_REQUIRE(fix.pathfinder.called);
}

BOOST_AUTO_TEST_CASE(navigation_stops_previous_movement) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.navigate({4u, 5u});
	fix.update();
	auto const & events = fix.input_events;
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, bot.id);
	BOOST_CHECK_VECTOR_EQUAL(events[0].move, sf::Vector2i());
}

BOOST_AUTO_TEST_CASE(isPathTarget_returns_true_if_target_is_at_paths_end) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.navigate({4u, 5u});
	bot.update(sf::Time::Zero);  // trigger
	bot.update(sf::Time::Zero);  // wait
	BOOST_REQUIRE(bot.hasPath());
	BOOST_REQUIRE(!bot.tracer.getPath().empty());
	BOOST_CHECK(bot.isPathTarget({4u, 5u}));
}

BOOST_AUTO_TEST_CASE(look_triggers_input_event) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.look({1, -1});
	fix.update();
	BOOST_REQUIRE_EQUAL(fix.input_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.input_events[0].actor, bot.id);
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].look, sf::Vector2i(1, -1));
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].move, sf::Vector2i(0, 0));
}

BOOST_AUTO_TEST_CASE(lookTowards_triggers_input_event) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.lookTowards({5u, 1u});
	fix.update();
	BOOST_REQUIRE_EQUAL(fix.input_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.input_events[0].actor, bot.id);
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].look, sf::Vector2i(1, -1));
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].move, sf::Vector2i(0, 0));
}

BOOST_AUTO_TEST_CASE(look_will_reset_pathtracer) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.navigate({4u, 5u});
	bot.update(sf::Time::Zero);  // trigger
	bot.update(sf::Time::Zero);  // wait
	BOOST_REQUIRE(bot.hasPath());
	bot.look({1, -1});
	fix.update();
	BOOST_CHECK_EQUAL(fix.input_events.size(), 3u); // pathfind triggers stop and move
	BOOST_CHECK(!bot.hasPath());
}

BOOST_AUTO_TEST_CASE(move_triggers_input_event) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.move({1, -1});
	fix.update();
	BOOST_REQUIRE_EQUAL(fix.input_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.input_events[0].actor, bot.id);
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].move, sf::Vector2i(1, -1));
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].look, sf::Vector2i(0, 0));
}

BOOST_AUTO_TEST_CASE(moveTowards_triggers_input_event) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.moveTowards({5u, 1u});
	fix.update();
	BOOST_REQUIRE_EQUAL(fix.input_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.input_events[0].actor, bot.id);
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].move, sf::Vector2i(1, -1));
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].look, sf::Vector2i(0, 0));
}

BOOST_AUTO_TEST_CASE(move_will_reset_pathtracer) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.navigate({4u, 5u});
	bot.update(sf::Time::Zero);  // trigger
	bot.update(sf::Time::Zero);  // wait
	BOOST_REQUIRE(bot.hasPath());
	bot.move({1, -1});
	fix.update();
	BOOST_CHECK_EQUAL(fix.input_events.size(), 3u); // pathfind trigger stop and move
	BOOST_CHECK(!bot.hasPath());
}

BOOST_AUTO_TEST_CASE(stop_triggers_input_event) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.stop();
	fix.update();
	BOOST_REQUIRE_EQUAL(fix.input_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.input_events[0].actor, bot.id);
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].look, sf::Vector2i(0, 0));
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].move, sf::Vector2i(0, 0));
}

BOOST_AUTO_TEST_CASE(stop_resets_path_tracer) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.navigate({4u, 5u});
	bot.update(sf::Time::Zero);
	BOOST_CHECK(bot.tracer.isRunning());
	bot.stop();
	BOOST_CHECK(!bot.tracer.isRunning());
}

BOOST_AUTO_TEST_CASE(attack_triggers_input_event) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.attack();
	fix.update();
	BOOST_REQUIRE_EQUAL(fix.action_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.action_events[0].actor, bot.id);
	BOOST_CHECK(!fix.action_events[0].idle);
	BOOST_CHECK(fix.action_events[0].action == rpg::PlayerAction::Attack);
}

BOOST_AUTO_TEST_CASE(useItem_triggers_item_event) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto& item = fix.item.query(bot.id);
	rpg::item_impl::addItem(item, fix.bow_tpl, 1u);
	bot.useItem(fix.bow_tpl);
	fix.update();
	BOOST_REQUIRE_EQUAL(fix.item_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.item_events[0].actor, bot.id);
	BOOST_CHECK(fix.item_events[0].type == rpg::ItemEvent::Use);
	BOOST_CHECK_EQUAL(fix.item_events[0].item, &fix.bow_tpl);
	BOOST_CHECK(fix.item_events[0].slot == fix.bow_tpl.slot);
}

BOOST_AUTO_TEST_CASE(usePerk_triggers_quickslot_use) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	auto& perk = fix.perk.query(bot.id);
	perk.perks.emplace_back(fix.fireball_tpl, 1u);
	bot.usePerk(fix.fireball_tpl);
	fix.update();
	BOOST_REQUIRE_EQUAL(fix.action_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.action_events[0].actor, bot.id);
	BOOST_CHECK(fix.action_events[0].action == rpg::PlayerAction::UseSlot);
}

BOOST_AUTO_TEST_CASE(cannot_usePerk_if_not_available) {
	auto& fix = Singleton<LuaFixture>::get();
	fix.reset();

	auto& bot = fix.createBot({2u, 2u}, true);
	bot.usePerk(fix.fireball_tpl);
	fix.update();
	BOOST_CHECK(fix.action_events.empty());
}

BOOST_AUTO_TEST_SUITE_END()
