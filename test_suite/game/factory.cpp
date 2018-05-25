#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <core/render.hpp>
#include <rpg/balance.hpp>
#include <rpg/item.hpp>
#include <rpg/perk.hpp>
#include <game/session.hpp>
#include <game/script.hpp>
#include <game/factory.hpp>
#include <game/navigator.hpp>
#include <game/path.hpp>

struct FactoryFixture : utils::EventListener<core::InputEvent,
	rpg::ActionEvent, rpg::ItemEvent, rpg::PerkEvent, rpg::StatsEvent,
	rpg::SpawnEvent, game::PowerupEvent> {
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
	utils::LightingSystem lighting;
	core::render_impl::Context render_context;

	rpg::StatsManager stats;
	rpg::EffectManager effect;
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
	game::NavigationSystem navigation;
	//game::ScriptManager script;
	game::HudManager hud;

	game::Session session;
	game::PathSystem pathfinder;
	std::vector<core::InputEvent> input_events;
	std::vector<rpg::ActionEvent> action_events;
	std::vector<rpg::ItemEvent> item_events;
	std::vector<rpg::StatsEvent> stats_events;
	std::vector<game::PowerupEvent> powerup_events;
	std::vector<rpg::SpawnEvent> respawn_events;

	game::ResourceCache cache;
	game::Mod mod;
	game::Factory factory;

	rpg::EntityTemplate entity, entity2, gem;
	rpg::SpriteTemplate sprite;
	rpg::BulletTemplate bullet;
	rpg::TrapTemplate trap;
	rpg::Keybinding keys;
	game::BotTemplate bot;
	game::RoomTemplate room;
	game::PlayerTemplate player_res;
	//game::AiScript ai_script;

	FactoryFixture()
		: utils::EventListener<core::InputEvent, rpg::ActionEvent,
			rpg::ItemEvent, rpg::PerkEvent, rpg::StatsEvent,
			rpg::SpawnEvent, game::PowerupEvent>{}
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
		, lighting{{320, 180}, dummy}
		, render_context{log, render, animation, movement,
			focus, dungeon, camera, lighting}
		, stats{}
		, effect{}
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
		, navigation{}
		//, script{}
		, hud{}
		, session{id_manager, dungeon, camera, movement, collision, focus,
			  animation, render, stats, effect, item, perk, player,
			  projectile, action, input, interact, quickslot, audio,
			  generator, navigation, /*script,*/ hud, pathfinder}
		, pathfinder{log}
		, input_events{}
		, action_events{}
		, item_events{}
		, stats_events{}
		, powerup_events{}
		, cache{}
		, mod{log, cache, ""}
		, factory{log, session, mod}
		, entity{}
		, entity2{}
		, gem{}
		, sprite{}
		, bullet{}
		, trap{}
		, keys{}
		, bot{}
		, room{}
		, player_res{}
		//, ai_script{}
		{
		//log.debug.add(std::cout);
		factory.bind<core::InputEvent>(*this);
		factory.bind<rpg::ActionEvent>(*this);
		factory.bind<rpg::ItemEvent>(*this);
		factory.bind<rpg::SpawnEvent>(*this);

		std::string code = 
			"called = false\n"
			"onInit = function(self, hostile)\n"
			"	called = true\n"
			"end\n"
			"onSpawn = function(self, hostile)\n"
			"end";
		//ai_script.loadFromMemory(code);
		// create demos scene
		rpg::TilesetTemplate tileset;
		tileset.tileset_name = "demo";
		tileset.tilesize = {16u, 16u};
		tileset.floors.emplace_back(0u, 0u);  // texture offset
		tileset.walls.emplace_back(16u, 0u);  // texture offset
		tileset.tileset = &dummy;
		sf::Vector2u pos;
		for (pos.y = 2u; pos.y <= 8u; ++pos.y) {
			for (pos.x = 3u; pos.x <= 7u; ++pos.x) {
				room.create(pos);
			}
		}
		generator.rooms.push_back(&room);
		generator.settings.cell_size = 10;
		generator.settings.room_density = 1.f;
		generator.settings.deadend_density =  0.f;
		thor::setRandomSeed(0u); // note: make generation predictable
		game::BuildSettings settings;
		settings.path_width = 2u;
		settings.random_transform = false;
		auto id = factory.createDungeon(tileset, {30u, 10u}, settings);
		BOOST_REQUIRE_EQUAL(id, 1u);
	}

	void handle(core::InputEvent const& event) {
		input_events.push_back(event);
	}

	void handle(rpg::ActionEvent const& event) {
		action_events.push_back(event);
	}

	void handle(rpg::ItemEvent const& event) { item_events.push_back(event); }

	void handle(rpg::StatsEvent const& event) { stats_events.push_back(event); }
	
	void handle(rpg::SpawnEvent const& event) { respawn_events.push_back(event); }
	
	void handle(game::PowerupEvent const& event) { powerup_events.push_back(event); }

	void update() {
		factory.update(sf::Time::Zero);

		dispatch<core::InputEvent>(*this);
		dispatch<rpg::ActionEvent>(*this);
		dispatch<rpg::ItemEvent>(*this);
		dispatch<rpg::StatsEvent>(*this);
		dispatch<rpg::SpawnEvent>(*this);
		dispatch<game::PowerupEvent>(*this);
	}

	void cleanup() {
		for (auto ptr : session.systems) {
			ptr->cleanup();
		}
	}
	
	void onCharacterDied(core::ObjectID id) {
		rpg::DeathEvent event;
		event.actor = id;
		factory.handle(event);
	}
	
	void onCharacterSpawned(core::ObjectID id, core::ObjectID causer) {
		rpg::SpawnEvent event;
		event.actor = id;
		event.causer = causer;
		event.respawn = true;
		factory.handle(event);
	}

	void reset() {
		// reset script
		//ai_script.set("called", false);
		// reset resources
		sprite = rpg::SpriteTemplate{};
		bullet = rpg::BulletTemplate{};
		trap = rpg::TrapTemplate{};
		keys = rpg::Keybinding{};
		entity = rpg::EntityTemplate{};
		entity2 = rpg::EntityTemplate{};
		gem = rpg::EntityTemplate{};
		bot = game::BotTemplate{};
		player_res = game::PlayerTemplate{};
		sprite.frameset = &dummy;
		sprite.torso[core::AnimationAction::Idle].frames.resize(1u);
		bullet.radius = 1.f;
		bullet.entity = &entity;
		entity.sprite = &sprite;
		entity2.sprite = &sprite;
		bot.entity = &entity;
		player_res.entity = &entity;
		trap.bullet.bullet = &bullet;
		trap.bullet.color = sf::Color::Cyan;
		entity2.light = std::make_unique<utils::Light>();
		entity2.light->color = sf::Color::Red;
		entity2.light->intensity = 128u;
		entity2.light->cast_shadow = true;
		entity2.light->lod = 5u;
		gem.sprite = &sprite;
		// reset dungeon
		auto& d = dungeon[1u];
		for (auto y = 0u; y < 10u; ++y) {
			for (auto x = 0u; x < 30u; ++x) {
				auto& c = d.getCell({x, y});
				c.trigger = nullptr;
				c.entities.clear();
				c.ambiences.clear();
			}
		}
		// reset objects
		//for (auto id : objects) {
		// note: powerups are not added to the objects list
		for (auto id = 1u; id < session.movement.capacity(); ++id) {
			for (auto ptr : session.systems) {
				ptr->tryRelease(id);
			}
		}
		objects.clear();
		id_manager.reset();
		factory.reset();
		factory.blood_texture = nullptr;
		factory.gem_tpl = &gem;
		// reset events
		update();
		cleanup();
		input_events.clear();
		action_events.clear();
		item_events.clear();
		stats_events.clear();
		powerup_events.clear();
		respawn_events.clear();
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(factory_test)

BOOST_AUTO_TEST_CASE(dungeon_is_built_on_creation) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();

	// room has no own floor cells in this example
	std::string expected =
		"                              \n"
		"  #######   #######   ####### \n"
		"  #.....#   #.....#   #.....# \n"
		"  #.....#####.....#####.....# \n"
		"  #.........................# \n"
		"  #.........................# \n"
		"  #.....#####.....#####.....# \n"
		"  #.....#   #.....#   #.....# \n"
		"  #.....#   #.....#   #.....# \n"
		"  #######   #######   ####### \n";
	
	std::string found;
	auto const& d = fix.dungeon[1u];
	for (auto y = 0u; y < 10u; ++y) {
		for (auto x = 0u; x < 30u; ++x) {
			auto const& c = d.getCell({x, y});
			switch (c.terrain) {
				case core::Terrain::Void:
					found += ' ';
					break;
				case core::Terrain::Wall:
					found += '#';
					break;
				case core::Terrain::Floor:
					found += '.';
					break;
			}
		}
		found += "\n";
	}
	BOOST_CHECK_EQUAL(expected, found);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(object_can_be_lighted) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity2, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.render.has(id));
	auto& r = fix.render.query(id);
	BOOST_REQUIRE(r.light != nullptr);
	BOOST_CHECK_COLOR_EQUAL(r.light->color, sf::Color::Red);
	BOOST_CHECK_EQUAL(r.light->intensity, 128u);
	BOOST_CHECK(r.light->cast_shadow);
	BOOST_CHECK_EQUAL(r.light->lod, 5u);
	BOOST_CHECK_COLOR_EQUAL(r.blood_color, sf::Color::Transparent);
}

BOOST_AUTO_TEST_CASE(object_can_have_blood_color) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	fix.entity2.blood_color = sf::Color::Magenta;
	auto id = fix.factory.createObject(fix.entity2, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.render.has(id));
	auto& r = fix.render.query(id);
	BOOST_REQUIRE(r.light != nullptr);
	BOOST_CHECK_COLOR_EQUAL(r.light->color, sf::Color::Red);
	BOOST_CHECK_EQUAL(r.light->intensity, 128u);
	BOOST_CHECK(r.light->cast_shadow);
	BOOST_CHECK_EQUAL(r.light->lod, 5u);
	BOOST_CHECK_COLOR_EQUAL(r.blood_color, sf::Color::Magenta);
}

BOOST_AUTO_TEST_CASE(minimal_object_has_only_movement_render_and_animation_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_CHECK(fix.movement.has(id));
	BOOST_CHECK(!fix.collision.has(id));
	BOOST_CHECK(!fix.focus.has(id));
	BOOST_CHECK(fix.render.has(id));
	BOOST_CHECK(fix.animation.has(id));
	BOOST_CHECK(!fix.audio.has(id));
}

BOOST_AUTO_TEST_CASE(object_is_spawned_correctly) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.max_speed = 15.f;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.movement.has(id));
	auto const& data = fix.movement.query(id);
	BOOST_CHECK_EQUAL(data.scene, spawn.scene);
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f{spawn.pos}, 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(data.look, spawn.direction);
	BOOST_CHECK_CLOSE(data.max_speed, 15.f, 0.0001f);

	auto const& dungeon = fix.dungeon[1u];
	auto const& cell = dungeon.getCell(sf::Vector2u{spawn.pos});
	BOOST_CHECK(utils::contains(cell.entities, id));
}

BOOST_AUTO_TEST_CASE(flying_object_is_spawned_moving) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.max_speed = 15.f;
	fix.entity.flying = true;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);
	
	auto const & move = fix.movement.query(id);
	BOOST_CHECK_VECTOR_EQUAL(move.move, sf::Vector2i())

	BOOST_REQUIRE(fix.animation.has(id));
	auto const & data = fix.animation.query(id);
	BOOST_CHECK(data.flying);
}

BOOST_AUTO_TEST_CASE(object_with_sight_has_focus_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.max_sight = 2.5f;
	fix.entity.display_name = "foo";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.focus.has(id));
	auto const& m_data = fix.movement.query(id);
	auto const& f_data = fix.focus.query(id);
	BOOST_CHECK_VECTOR_EQUAL(m_data.look, spawn.direction);
	BOOST_CHECK_EQUAL(f_data.display_name, "foo");
	BOOST_CHECK_CLOSE(f_data.sight, 2.5f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(object_with_sight_requires_display_name) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.max_sight = 2.5f;
	fix.entity.display_name = "";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(fix.factory.createObject(fix.entity, spawn));
}

BOOST_AUTO_TEST_CASE(aabb_collideable_object_has_collision_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.shape.is_aabb = true;
	fix.entity.shape.radius = 0.f;
	fix.entity.size = {1.f, 2.f};
	fix.entity.is_projectile = false;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.collision.has(id));
	auto const& data = fix.collision.query(id);
	BOOST_CHECK(!data.is_projectile);
	BOOST_CHECK(data.shape.is_aabb);
	BOOST_CHECK_GT(data.shape.radius, 0.f); // means it was updated
	BOOST_CHECK_VECTOR_CLOSE(data.shape.size, fix.entity.size);
}

BOOST_AUTO_TEST_CASE(circle_collideable_object_has_collision_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.shape.is_aabb = false;
	fix.entity.shape.radius = 3.f;
	fix.entity.is_projectile = false;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.collision.has(id));
	auto const& data = fix.collision.query(id);
	BOOST_CHECK(!data.is_projectile);
	BOOST_CHECK(!data.shape.is_aabb);
	BOOST_CHECK_CLOSE(data.shape.radius, 3.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(object_with_animated_legs_has_animation_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.sprite.legs.frames.resize(1u);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.animation.has(id));
	auto const& data = fix.animation.query(id);
	BOOST_CHECK_EQUAL(
		data.tpl.legs[core::SpriteLegLayer::Base], &fix.sprite.legs);
}

BOOST_AUTO_TEST_CASE(object_with_animated_sprite_has_animation_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.sprite.torso[core::AnimationAction::Melee].frames.resize(1u);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.animation.has(id));
	auto const& data = fix.animation.query(id);
	BOOST_CHECK_EQUAL(
		data.tpl.torso[core::SpriteTorsoLayer::Base], &fix.sprite.torso);
}

BOOST_AUTO_TEST_CASE(object_with_soundeffects_has_sound_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	sf::SoundBuffer tmp;
	fix.entity.sounds[core::default_value<core::SoundAction>()].emplace_back("test", &tmp);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.audio.has(id));
	auto const& data = fix.audio.query(id);
	for (auto const& pair : data.sfx) {
		for (auto const & node: fix.entity.sounds[pair.first]) {
			BOOST_CHECK(utils::contains(pair.second, node.second));
		}
	}
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(object_with_interact_type_is_interactable) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	fix.entity.max_sight = 0.f;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	BOOST_REQUIRE(fix.interact.has(id));
}

BOOST_AUTO_TEST_CASE(interactable_is_required_to_be_collideable) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = false;
	fix.entity.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	fix.entity.max_sight = 0.f;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(fix.factory.createObject(fix.entity, spawn));
}

BOOST_AUTO_TEST_CASE(interactable_is_required_to_have_zero_sight_radius) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	fix.entity.max_sight = 1.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(fix.factory.createObject(fix.entity, spawn));
}

BOOST_AUTO_TEST_CASE(interactable_is_minimal_object_with_interact_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	fix.entity.max_sight = 0.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	BOOST_CHECK(fix.movement.has(id));
	BOOST_CHECK(fix.collision.has(id));
	BOOST_CHECK(fix.render.has(id));
	BOOST_REQUIRE(fix.focus.has(id));
	{
		auto const & data = fix.focus.query(id);
		BOOST_CHECK_CLOSE(data.sight, 0.f, 0.00001f);
		BOOST_CHECK_EQUAL(data.display_name, "obstacle");
	}
	BOOST_REQUIRE(fix.interact.has(id));
	{
		auto const& data = fix.interact.query(id);
		BOOST_CHECK(data.type == rpg::InteractType::Barrier);
	}
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(createBullet_creates_respawn_event_for_that_bullet) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::ItemTemplate weapon;
	weapon.bullet.bullet = &fix.bullet;
	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Weapon;
	meta.primary = &weapon;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
	fix.update();
	
	BOOST_REQUIRE_EQUAL(fix.respawn_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.respawn_events[0].actor, id);
}

BOOST_AUTO_TEST_CASE(bullet_by_weapon_can_be_colored) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::ItemTemplate weapon;
	weapon.bullet.bullet = &fix.bullet;
	weapon.bullet.color = sf::Color::Magenta;
	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Weapon;
	meta.primary = &weapon;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
	fix.update();
	
	BOOST_REQUIRE(fix.render.has(id));
	auto& r = fix.render.query(id);
	BOOST_CHECK_COLOR_EQUAL(r.torso[core::SpriteTorsoLayer::Base].getColor(), weapon.bullet.color);
}

BOOST_AUTO_TEST_CASE(bullet_by_perk_can_be_colored) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::PerkTemplate spell;
	spell.bullet.bullet = &fix.bullet;
	spell.bullet.color = sf::Color::Magenta;
	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Perk;
	meta.perk = &spell;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
	fix.update();
	
	BOOST_REQUIRE(fix.render.has(id));
	auto& r = fix.render.query(id);
	BOOST_CHECK_COLOR_EQUAL(r.torso[core::SpriteTorsoLayer::Base].getColor(), spell.bullet.color);
}

BOOST_AUTO_TEST_CASE(bullet_by_trap_can_be_colored) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
	fix.update();
	
	BOOST_REQUIRE(fix.render.has(id));
	auto& r = fix.render.query(id);
	BOOST_CHECK_COLOR_EQUAL(r.torso[core::SpriteTorsoLayer::Base].getColor(), fix.trap.bullet.color);
}

BOOST_AUTO_TEST_CASE(bullet_can_be_created_by_weapon) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::ItemTemplate weapon;
	weapon.bullet.bullet = &fix.bullet;
	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Weapon;
	meta.primary = &weapon;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
}

BOOST_AUTO_TEST_CASE(bullet_can_be_created_by_perk) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::PerkTemplate spell;
	spell.bullet.bullet = &fix.bullet;
	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Perk;
	meta.perk = &spell;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
}

BOOST_AUTO_TEST_CASE(bullet_can_be_created_by_trap) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
}

BOOST_AUTO_TEST_CASE(bullet_requires_to_be_collideable) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = false;
	fix.entity.is_projectile = true;
	fix.entity.max_sight = 0.f;
	fix.entity.display_name.clear();

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(fix.factory.createBullet(meta, spawn, 0u));
}

BOOST_AUTO_TEST_CASE(bullet_requires_to_be_a_projectile) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = false;
	fix.entity.max_sight = 0.f;
	fix.entity.display_name.clear();

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(fix.factory.createBullet(meta, spawn, 0u));
}

BOOST_AUTO_TEST_CASE(bullet_requires_to_be_unfocusable) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = false;
	fix.entity.max_sight = 1.f;

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(fix.factory.createBullet(meta, spawn, 0u));
}

BOOST_AUTO_TEST_CASE(bullet_has_suitable_component_Data) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.render.has(id));
	{
		auto const& data = fix.render.query(id);
		BOOST_CHECK(data.layer == core::ObjectLayer::Top);
	}
	
	BOOST_REQUIRE(fix.animation.has(id));
	{
		auto const & data = fix.animation.query(id);
		BOOST_CHECK_EQUAL(data.tpl.torso[core::SpriteTorsoLayer::Base], &fix.entity.sprite->torso);
	}

	BOOST_REQUIRE(fix.collision.has(id));
	{
		auto const& data = fix.collision.query(id);
		BOOST_CHECK(data.is_projectile);
		BOOST_CHECK_CLOSE(data.shape.radius, fix.bullet.radius, 0.0001f);
	}

	BOOST_REQUIRE(fix.projectile.has(id));
	{
		auto const& data = fix.projectile.query(id);
		BOOST_CHECK_EQUAL(data.bullet, &fix.bullet);
		BOOST_CHECK(data.meta_data.emitter == rpg::EmitterType::Trap);
		BOOST_CHECK(data.meta_data.primary == nullptr);
		BOOST_CHECK(data.meta_data.secondary == nullptr);
		BOOST_CHECK(data.meta_data.perk == nullptr);
		BOOST_CHECK_EQUAL(data.meta_data.trap, &fix.trap);
	}
}

BOOST_AUTO_TEST_CASE(bullet_spawndata_is_renewed_if_owner_given) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "obstacle";

	auto owner = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(owner);
	BOOST_REQUIRE(fix.movement.has(owner));
	{
		auto& data = fix.movement.query(owner);
		data.pos.x += 0.25f;
		data.pos.y -= 0.3f;
	}

	spawn.pos = {10u, 7u};
	spawn.direction = {0, -1};
	fix.entity.max_sight = 0.f;
	fix.entity.display_name.clear();
	fix.entity.is_projectile = true;

	auto id = fix.factory.createBullet(meta, spawn, owner);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.movement.has(id));
	{
		auto const& data = fix.movement.query(id);
		BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(5.f, 5.f), 0.0001f);
		BOOST_CHECK_VECTOR_EQUAL(data.look, sf::Vector2f(1, 0));
	}
}

BOOST_AUTO_TEST_CASE(bullet_ignores_its_owner) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "obstacle";

	auto owner = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(owner);
	fix.entity.max_sight = 0.f;
	fix.entity.display_name.clear();

	auto id = fix.factory.createBullet(meta, spawn, owner);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.projectile.has(id));
	{
		auto const& data = fix.projectile.query(id);
		BOOST_CHECK_EQUAL(data.owner, owner);
		BOOST_CHECK(utils::contains(data.ignore, owner));
	}
	BOOST_REQUIRE(fix.collision.has(id));
	{
		auto const& data = fix.collision.query(id);
		BOOST_CHECK(utils::contains(data.ignore, owner));
	}
}

BOOST_AUTO_TEST_CASE(bullet_moves_automatically_after_spawn) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);

	fix.update();
	BOOST_REQUIRE_EQUAL(fix.input_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.input_events[0].actor, id);
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].move, sf::Vector2i(1, 0));
	BOOST_CHECK_VECTOR_EQUAL(fix.input_events[0].look, sf::Vector2i(1, 0));
}

// ---------------------------------------------------------------------------
/*
BOOST_AUTO_TEST_CASE(bot_requires_to_be_collideable) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = false;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true));
}

BOOST_AUTO_TEST_CASE(bot_requires_to_be_focusable) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 0.f;
	fix.entity.display_name.clear();

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(
		fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true));
}

BOOST_AUTO_TEST_CASE(default_bot_has_suitable_roleplaying_related_components) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.display_name = "bar";
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.bot.display_name = "foo";  // bot name overrides entity name
	fix.bot.color = sf::Color::Red;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.focus.has(id));
	{
		auto const& data = fix.focus.query(id);
		BOOST_CHECK_EQUAL(data.display_name, "foo");
	}
	BOOST_REQUIRE(fix.render.has(id));
	{
		auto const& data = fix.render.query(id);
		BOOST_CHECK(data.layer == core::ObjectLayer::Top);
		auto const & legs = data.legs[core::SpriteLegLayer::Base];
		auto const & torso = data.torso[core::SpriteTorsoLayer::Base];
		BOOST_CHECK_COLOR_EQUAL(legs.getColor(), fix.bot.color);
		BOOST_CHECK_COLOR_EQUAL(torso.getColor(), fix.bot.color);
	}
	BOOST_CHECK(fix.quickslot.has(id));
	BOOST_CHECK(fix.effect.has(id));
	BOOST_CHECK(fix.action.has(id));
	BOOST_CHECK(fix.item.has(id));
	BOOST_CHECK(fix.perk.has(id));
}

BOOST_AUTO_TEST_CASE(bot_stats_component_is_automatically_initialized) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.display_name = "foo";
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";
	fix.bot.attributes[rpg::Attribute::Strength] = 0.5f;
	fix.bot.attributes[rpg::Attribute::Dexterity] = 0.3f;
	fix.bot.attributes[rpg::Attribute::Wisdom] = 0.2f;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.stats.has(id));
	auto const& data = fix.stats.query(id);
	BOOST_CHECK_EQUAL(data.attributes[rpg::Attribute::Strength], game::MIN_BOT_ATTRIB + 25);
	BOOST_CHECK_EQUAL(data.attributes[rpg::Attribute::Dexterity], game::MIN_BOT_ATTRIB + 15);
	BOOST_CHECK_EQUAL(data.attributes[rpg::Attribute::Wisdom], game::MIN_BOT_ATTRIB + 10);
	BOOST_CHECK_EQUAL(data.level, 10u);
	BOOST_CHECK_EQUAL(
		data.stats[rpg::Stat::Life], data.properties[rpg::Property::MaxLife]);
	BOOST_CHECK_EQUAL(
		data.stats[rpg::Stat::Mana], data.properties[rpg::Property::MaxMana]);
	BOOST_CHECK_EQUAL(data.stats[rpg::Stat::Stamina],
		data.properties[rpg::Property::MaxStamina]);
}

BOOST_AUTO_TEST_CASE(bot_stats_component_is_affected_by_defense_and_damage_boni) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.display_name = "foo";
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";
	fix.bot.properties[rpg::Property::MeleeBase] = 2.4f;
	fix.bot.defense[rpg::DamageType::Blade] = 1.5f;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 2u, fix.ai_script, true);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.stats.has(id));
	auto const& data = fix.stats.query(id);
	BOOST_CHECK_EQUAL(data.prop_boni[rpg::Property::MeleeBase], rpg::getPropertyValue(2.4f, 2u));
	BOOST_CHECK_CLOSE(data.base_def[rpg::DamageType::Blade], 1.5f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(bot_can_have_items) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	rpg::ItemTemplate sword, botw, potion;

	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";
	fix.bot.items.emplace_back("sword", 1u, &sword);
	fix.bot.items.emplace_back("botw", 1u, &botw);
	fix.bot.items.emplace_back("potion", 3u, &potion);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.item.has(id));
	auto const& data = fix.item.query(id);
	rpg::hasItem(data, sword, 1u);
	rpg::hasItem(data, botw, 1u);
	rpg::hasItem(data, potion, 3u);
}

BOOST_AUTO_TEST_CASE(bot_can_have_perks) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	rpg::PerkTemplate fireball, heal;

	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";
	fix.bot.entity = &fix.entity;
	fix.bot.perks.emplace_back("fireball", 1.05f, &fireball);
	fix.bot.perks.emplace_back("heal", 0.5f, &heal);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.perk.has(id));
	auto const& data = fix.perk.query(id);
	BOOST_CHECK_EQUAL(rpg::getPerkLevel(data, fireball), 11u);
	BOOST_CHECK_EQUAL(rpg::getPerkLevel(data, heal), 5u);
}

BOOST_AUTO_TEST_CASE(bot_has_script_component) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.script.has(id));
	auto const& data = fix.script.query(id);
	BOOST_REQUIRE(data.api != nullptr);
	BOOST_CHECK_EQUAL(data.api->id, id);
	BOOST_CHECK_EQUAL(&data.api->session, &fix.session);
	BOOST_CHECK_EQUAL(&data.api->input_sender, &fix.factory);
	BOOST_CHECK_EQUAL(&data.api->action_sender, &fix.factory);
	BOOST_CHECK_EQUAL(data.script, &fix.ai_script);
	BOOST_CHECK(fix.ai_script.get<bool>("called") == true);
}
*/

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(player_requires_to_be_collideable) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(fix.factory.createPlayer(fix.player_res, fix.keys, spawn));
}

BOOST_AUTO_TEST_CASE(player_requires_to_be_focusable) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	BOOST_CHECK_ASSERT(
		fix.factory.createPlayer(fix.player_res, fix.keys, spawn));
}

BOOST_AUTO_TEST_CASE(player_has_suitable_component_data) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = game::PLAYER_LIGHT_RADIUS + 10.f; // is overridden!
	fix.entity.display_name = "foo";
	fix.player_res.display_name = "bar";
	fix.player_res.level = 12u;
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attrib_points = 3u;
	fix.player_res.perk_points = 1u;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.focus.has(id));
	{
		auto const& data = fix.focus.query(id);
		BOOST_CHECK_CLOSE(data.sight, game::PLAYER_LIGHT_RADIUS, 0.0001f);
		BOOST_CHECK_EQUAL(data.display_name, "bar");
	}
	BOOST_REQUIRE(fix.render.has(id));
	{
		auto const& data = fix.render.query(id);
		BOOST_CHECK(data.layer == core::ObjectLayer::Top);
	}
	BOOST_CHECK(fix.effect.has(id));
	BOOST_CHECK(fix.action.has(id));
	BOOST_REQUIRE(fix.stats.has(id));
	{
		auto const& data = fix.stats.query(id);
		BOOST_CHECK_EQUAL(data.level, 12u);
		BOOST_CHECK_EQUAL(data.attributes[rpg::Attribute::Strength], 25u);
		BOOST_CHECK_EQUAL(data.attributes[rpg::Attribute::Dexterity], 55u);
		BOOST_CHECK_EQUAL(data.attributes[rpg::Attribute::Wisdom], 10u);
	}
	BOOST_REQUIRE(fix.player.has(id));
	{
		auto const& data = fix.player.query(id);
		BOOST_CHECK_EQUAL(data.attrib_points, 3u);
		BOOST_CHECK_EQUAL(data.perk_points, 1u);
		BOOST_CHECK_EQUAL(data.exp, fix.player_res.exp);
		BOOST_CHECK_EQUAL(data.next_exp, rpg::getNextExp(13u));
		BOOST_CHECK_EQUAL(data.player_id, 1u);
	}
	BOOST_REQUIRE(fix.hud.has(id));
	{
		auto const& data = fix.hud.query(id);
		BOOST_CHECK(data.hud != nullptr);
	}
}

BOOST_AUTO_TEST_CASE(player_has_highlight_sprite) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = game::PLAYER_LIGHT_RADIUS + 10.f; // is overridden!
	fix.entity.display_name = "foo";
	fix.player_res.display_name = "bar";
	fix.player_res.level = 12u;
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attrib_points = 3u;
	fix.player_res.perk_points = 1u;
	BOOST_CHECK(fix.entity.light == nullptr);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn, sf::Color::Red);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.render.has(id));
	auto const & data = fix.render.query(id);
	BOOST_REQUIRE(data.highlight != nullptr);
	BOOST_CHECK_COLOR_EQUAL(data.highlight->getColor(), sf::Color::Red);
}

BOOST_AUTO_TEST_CASE(player_has_light_despite_entity_has_light) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = game::PLAYER_LIGHT_RADIUS + 10.f; // is overridden!
	fix.entity.display_name = "foo";
	fix.player_res.display_name = "bar";
	fix.player_res.level = 12u;
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attrib_points = 3u;
	fix.player_res.perk_points = 1u;
	BOOST_CHECK(fix.entity.light == nullptr);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.render.has(id));
	auto const & data = fix.render.query(id);
	BOOST_REQUIRE(data.light != nullptr);
}

BOOST_AUTO_TEST_CASE(player_light_overrides_entity_light_settings) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = game::PLAYER_LIGHT_RADIUS + 10.f; // is overridden!
	fix.entity.display_name = "foo";
	fix.entity.light = std::make_unique<utils::Light>();
	fix.entity.light->radius = game::PLAYER_LIGHT_RADIUS + 10.f;
	fix.entity.light->intensity = 255u - game::PLAYER_LIGHT_INTENSITY;
	fix.entity.light->color = sf::Color::Black;
	fix.player_res.display_name = "bar";
	fix.player_res.level = 12u;
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attrib_points = 3u;
	fix.player_res.perk_points = 1u;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.render.has(id));
	auto const & data = fix.render.query(id);
	BOOST_REQUIRE(data.light != nullptr);
	BOOST_CHECK_EQUAL(data.light->radius, game::PLAYER_LIGHT_RADIUS);
	BOOST_CHECK_EQUAL(data.light->intensity, game::PLAYER_LIGHT_INTENSITY);
	BOOST_CHECK_COLOR_EQUAL(data.light->color, game::PLAYER_LIGHT_COLOR);
}

BOOST_AUTO_TEST_CASE(player_has_given_keybinding) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";
	fix.player_res.level = 12u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;
	fix.keys.map.set(rpg::PlayerAction::Attack, {1u, 0u});
	fix.keys.map.set(rpg::PlayerAction::Interact, {1u, 1u});
	fix.keys.map.set(rpg::PlayerAction::UseSlot, {1u, 2u});
	fix.keys.map.set(rpg::PlayerAction::PrevSlot, {1u, 3u});
	fix.keys.map.set(rpg::PlayerAction::NextSlot, {1u, 4u});
	fix.keys.map.set(rpg::PlayerAction::Pause, {1u, 5u});
	fix.keys.map.set(rpg::PlayerAction::ToggleAutoLook, {1u, 6u});
	fix.keys.map.set(
		rpg::PlayerAction::MoveN, {1u, sf::Joystick::Axis::Y, -25.f});
	fix.keys.map.set(
		rpg::PlayerAction::MoveS, {1u, sf::Joystick::Axis::Y, 25.f});
	fix.keys.map.set(
		rpg::PlayerAction::MoveW, {1u, sf::Joystick::Axis::X, -25.f});
	fix.keys.map.set(
		rpg::PlayerAction::MoveE, {1u, sf::Joystick::Axis::X, 25.f});
	fix.keys.map.set(
		rpg::PlayerAction::LookN, {1u, sf::Joystick::Axis::U, -25.f});
	fix.keys.map.set(
		rpg::PlayerAction::LookS, {1u, sf::Joystick::Axis::U, 25.f});
	fix.keys.map.set(
		rpg::PlayerAction::LookW, {1u, sf::Joystick::Axis::V, -25.f});
	fix.keys.map.set(
		rpg::PlayerAction::LookE, {1u, sf::Joystick::Axis::V, 25.f});

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.input.has(id));
	{
		auto const& data = fix.input.query(id);
		BOOST_REQUIRE(fix.keys.map == data.keys);
	}
}

BOOST_AUTO_TEST_CASE(player_has_unique_camera) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";
	fix.player_res.level = 12u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);

	for (auto const& cam : fix.camera) {
		if (utils::contains(cam->objects, id)) {
			BOOST_REQUIRE_EQUAL(cam->objects.size(), 1u);
		}
	}
}

BOOST_AUTO_TEST_CASE(player_can_have_items) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";
	fix.player_res.level = 12u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;

	rpg::ItemTemplate sword, potion;
	sword.type = rpg::ItemType::Weapon;
	fix.player_res.inventory.emplace_back("sword", 1u, &sword);
	potion.type = rpg::ItemType::Potion;
	fix.player_res.inventory.emplace_back("potion", 7u, &potion);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);

	BOOST_REQUIRE(fix.item.has(id));
	auto const& data = fix.item.query(id);
	auto const& weapons = data.inventory[rpg::ItemType::Weapon];
	auto const& potions = data.inventory[rpg::ItemType::Potion];
	BOOST_REQUIRE_EQUAL(weapons.size(), 1u);
	BOOST_CHECK_EQUAL(weapons[0].item, &sword);
	BOOST_CHECK_EQUAL(weapons[0].quantity, 1u);
	BOOST_REQUIRE_EQUAL(potions.size(), 1u);
	BOOST_CHECK_EQUAL(potions[0].item, &potion);
	BOOST_CHECK_EQUAL(potions[0].quantity, 7u);
}

BOOST_AUTO_TEST_CASE(player_can_equip_items) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";
	fix.player_res.level = 12u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;

	rpg::ItemTemplate sword;
	sword.type = rpg::ItemType::Weapon;
	sword.slot = rpg::EquipmentSlot::Weapon;
	fix.player_res.inventory.emplace_back("sword", 1u, &sword);
	fix.player_res.equip_ptr[rpg::EquipmentSlot::Weapon] = &sword;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);
	fix.update();

	auto const& events = fix.item_events;
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id);
	BOOST_CHECK(events[0].type == rpg::ItemEvent::Use);
	BOOST_CHECK_EQUAL(events[0].item, &sword);
	BOOST_CHECK(events[0].slot == rpg::EquipmentSlot::Weapon);
}

BOOST_AUTO_TEST_CASE(player_can_have_perk) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";
	fix.player_res.level = 12u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;

	rpg::PerkTemplate fireball;
	fix.player_res.perks.emplace_back("perk", 2u, &fireball);

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);

	auto const& data = fix.perk.query(id);
	BOOST_REQUIRE_EQUAL(data.perks.size(), 1u);
	BOOST_CHECK_EQUAL(data.perks[0].perk, &fireball);
	BOOST_CHECK_EQUAL(data.perks[0].level, 2u);
}

BOOST_AUTO_TEST_CASE(player_can_have_quickslots) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";
	fix.player_res.level = 12u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;

	rpg::ItemTemplate sword;
	rpg::PerkTemplate fireball;
	fix.player_res.inventory.emplace_back("sword", 1u, &sword);
	fix.player_res.perks.emplace_back("fireball", 2u, &fireball);
	fix.player_res.slot_id = 2u;
	std::get<1>(fix.player_res.slots[0u]) = "fireball";
	std::get<3>(fix.player_res.slots[0u]) = &fireball;
	std::get<0>(fix.player_res.slots[2u]) = "sword";
	std::get<2>(fix.player_res.slots[2u]) = &sword;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);

	auto const& data = fix.quickslot.query(id);
	BOOST_CHECK_EQUAL(data.slot_id, 2u);
	BOOST_CHECK_EQUAL(data.slots[0u].perk, &fireball);
	BOOST_CHECK_EQUAL(data.slots[2u].item, &sword);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(destroyed_object_has_no_components) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	fix.factory.destroyObject(id);
	fix.cleanup();

	// test some components
	BOOST_CHECK(!fix.movement.has(id));
	BOOST_CHECK(!fix.render.has(id));
}

BOOST_AUTO_TEST_CASE(destroyed_object_is_released_from_scene) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	fix.factory.destroyObject(id);

	auto const& dungeon = fix.dungeon[spawn.scene];
	auto const& cell = dungeon.getCell(sf::Vector2u{spawn.pos});
	BOOST_CHECK(!utils::contains(cell.entities, id));
}

BOOST_AUTO_TEST_CASE(destroying_object_twice_within_a_frame_will_ignore_second_release) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createObject(fix.entity, spawn);
	fix.objects.push_back(id);

	fix.factory.destroyObject(id); // killed by player 1
	fix.factory.destroyObject(id); // killed by player 2
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(bullet_stops_on_explosion) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;
	fix.entity.max_sight = 0.f;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
	auto& data = fix.movement.query(id);
	data.pos.x += 0.3f;

	fix.factory.onBulletExploded(id);
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(5.3f, 5.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(data.move, sf::Vector2i());
}

BOOST_AUTO_TEST_CASE(bullet_loses_collision_component_on_explosion) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;
	fix.entity.max_sight = 0.f;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
	fix.factory.onBulletExploded(id);
	fix.cleanup();

	BOOST_CHECK(!fix.collision.has(id));
}

BOOST_AUTO_TEST_CASE(bullet_is_destroyed_after_delay) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.is_projectile = true;
	fix.entity.max_sight = 0.f;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	rpg::CombatMetaData meta;
	meta.emitter = rpg::EmitterType::Trap;
	meta.trap = &fix.trap;

	auto id = fix.factory.createBullet(meta, spawn, 0u);
	fix.objects.push_back(id);
	fix.factory.onBulletExploded(id);

	fix.cleanup();

	BOOST_CHECK(fix.movement.has(id));
	fix.factory.update(sf::milliseconds(game::FADE_DELAY - 1u));
	fix.cleanup();

	BOOST_CHECK(fix.movement.has(id));
	fix.factory.update(sf::milliseconds(1u));
	fix.cleanup();

	BOOST_CHECK(!fix.movement.has(id));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(character_is_moved_to_bottom_layer_on_death) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	auto& data = fix.render.query(id);
	data.layer = core::ObjectLayer::Top;

	fix.onCharacterDied(id);
	BOOST_CHECK(data.layer == core::ObjectLayer::Bottom);
}

BOOST_AUTO_TEST_CASE(character_loses_collision_component_on_death) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	fix.onCharacterDied(id);
	fix.cleanup();

	BOOST_CHECK(!fix.collision.has(id));
}

BOOST_AUTO_TEST_CASE(character_cannot_be_focused_after_death) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.focus.query(id).has_changed = false;
	fix.objects.push_back(id);
	fix.onCharacterDied(id);

	auto const& data = fix.focus.query(id);
	BOOST_REQUIRE(!data.is_active);
	BOOST_CHECK(data.has_changed);
}

BOOST_AUTO_TEST_CASE(character_fov_is_disabled_after_death) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.focus.query(id).has_changed = false;
	fix.objects.push_back(id);
	fix.onCharacterDied(id);

	auto& data = fix.render.query(id);
	core::render_impl::updateObject(fix.render_context, data);
	BOOST_CHECK_CLOSE(data.fov.getRadius(), 0.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(character_is_stopped_on_death) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	auto& body = fix.movement.query(id);
	body.pos.x += 0.3f;
	body.pos.y += 0.2f;
	body.move = {1, 1};
	fix.onCharacterDied(id);
	fix.update();

	BOOST_REQUIRE_EQUAL(fix.input_events.size(), 1u);
	BOOST_CHECK_EQUAL(fix.input_events[0].actor, id);
	BOOST_REQUIRE_VECTOR_EQUAL(fix.input_events[0].move, sf::Vector2i());
}

BOOST_AUTO_TEST_CASE(character_death_causes_blood_if_entity_if_textures_and_color_are_provided) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";
	fix.factory.blood_texture = &fix.dummy;
	fix.entity.blood_color = sf::Color::Yellow;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	auto& data = fix.render.query(id);
	data.layer = core::ObjectLayer::Top;

	fix.onCharacterDied(id);
	auto& blood = fix.dungeon[1u].getCell(sf::Vector2u{spawn.pos}).ambiences;
	BOOST_REQUIRE_EQUAL(blood.size(), 1u);
	BOOST_CHECK_EQUAL(blood[0].getTexture(), &fix.dummy);
}

BOOST_AUTO_TEST_CASE(character_death_does_not_cause_blood_if_no_textures_are_provided) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	auto& data = fix.render.query(id);
	data.layer = core::ObjectLayer::Top;

	fix.onCharacterDied(id);
	auto& blood = fix.dungeon[1u].getCell(sf::Vector2u{spawn.pos}).ambiences;
	BOOST_CHECK(blood.empty());
}

BOOST_AUTO_TEST_CASE(powerup_can_be_created_on_character_death) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	fix.onCharacterDied(id);
	thor::setRandomSeed(536304u); // manipulate RNG to generate a powerup :3
	fix.update();

	auto& trigger = fix.dungeon[spawn.scene].getCell(sf::Vector2u{spawn.pos}).trigger;
	BOOST_REQUIRE(trigger != nullptr);
}

BOOST_AUTO_TEST_CASE(powerup_cannot_be_spawned_if_another_trigger_is_placed) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};
	
	// create another trigger to block
	fix.factory.createPowerup(*fix.factory.gem_tpl, spawn, game::PowerupType::Life);
	auto const & cell = fix.dungeon[spawn.scene].getCell(sf::Vector2u{spawn.pos});
	BOOST_REQUIRE(cell.trigger != nullptr);
	
	auto result = game::factory_impl::canHoldPowerup(fix.session, spawn.scene, spawn.pos);
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(powerup_cannot_be_spawned_if_not_a_floor_tile) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {0u, 0u};
	spawn.direction = {1, 0};
	
	auto const & cell = fix.dungeon[spawn.scene].getCell(sf::Vector2u{spawn.pos});
	BOOST_REQUIRE(cell.terrain != core::Terrain::Floor);
	
	auto result = game::factory_impl::canHoldPowerup(fix.session, spawn.scene, spawn.pos);
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(powerup_cannot_be_spawned_outside_dungeon) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {-1.f, -1.f};
	spawn.direction = {1, 0};
	
	BOOST_REQUIRE(!fix.dungeon[spawn.scene].has(sf::Vector2u{spawn.pos}));
	
	auto result = game::factory_impl::canHoldPowerup(fix.session, spawn.scene, spawn.pos);
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(powerup_cannot_be_spawned_if_a_collideable_non_bullet_is_placed_there) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};
	
	// create another trigger to block
	fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	
	auto result = game::factory_impl::canHoldPowerup(fix.session, spawn.scene, spawn.pos);
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(powerup_can_be_released_through_release_event) {
	auto& fix = Singleton<FactoryFixture>::get();
	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	fix.factory.createPowerup(*fix.factory.gem_tpl, spawn, game::PowerupType::Life);
	
	auto& trigger = fix.dungeon[spawn.scene].getCell(sf::Vector2u{spawn.pos}).trigger;
	auto powerup = dynamic_cast<game::PowerupTrigger*>(trigger.get());
	BOOST_REQUIRE(powerup != nullptr);
	game::ReleaseEvent release;
	release.actor = powerup->getId();
	fix.factory.handle(release);
	fix.update();
	
	BOOST_CHECK(fix.movement.has(release.actor));
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(respawn_event_is_forwarded_after_handled_at_factory) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	auto& data = fix.render.query(id);
	data.layer = core::ObjectLayer::Top;

	fix.onCharacterDied(id);
	fix.cleanup();
	fix.update();
	fix.respawn_events.clear(); // 'cause creation will create a spawn event, too
	fix.onCharacterSpawned(id, 0u);
	fix.update();
	
	auto const & events = fix.respawn_events;
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id);
}

BOOST_AUTO_TEST_CASE(respawn_does_not_reset_sprite_layers) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";
	sf::Texture dummy;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	auto& data = fix.render.query(id);
	data.torso[core::SpriteTorsoLayer::Armor].setTexture(dummy);

	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, 0u);
	fix.update();
	
	BOOST_CHECK(data.torso[core::SpriteTorsoLayer::Armor].getTexture() == &dummy);
}

BOOST_AUTO_TEST_CASE(character_is_moved_to_top_layer_on_respawn) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	auto& data = fix.render.query(id);
	data.layer = core::ObjectLayer::Top;

	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, 0u);
	BOOST_CHECK(data.layer == core::ObjectLayer::Top);
}

BOOST_AUTO_TEST_CASE(character_regains_lighting_on_respawn) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.light = std::make_unique<utils::Light>();
	fix.entity.light->radius = 123.f;
	fix.entity.light->color = sf::Color::Magenta;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, 0u);

	BOOST_REQUIRE(fix.render.has(id));
	auto const & render = fix.render.query(id);
	BOOST_REQUIRE(render.light != nullptr);
	BOOST_CHECK_CLOSE(render.light->radius, fix.entity.light->radius, 0.0001f);
	BOOST_CHECK_COLOR_EQUAL(render.light->color, fix.entity.light->color);
}

BOOST_AUTO_TEST_CASE(character_regains_collision_component_on_respawn) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.objects.push_back(id);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, 0u);

	BOOST_CHECK(fix.collision.has(id));
}

BOOST_AUTO_TEST_CASE(character_can_be_focused_after_respawn) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.focus.query(id).has_changed = false;
	fix.objects.push_back(id);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, 0u);

	auto const& data = fix.focus.query(id);
	BOOST_REQUIRE(data.is_active);
	BOOST_CHECK(data.has_changed);
}

BOOST_AUTO_TEST_CASE(character_fov_is_enabled_after_respawn) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, /*fix.ai_script,*/ true);
	fix.focus.query(id).has_changed = false;
	fix.objects.push_back(id);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, 0u);

	auto& data = fix.render.query(id);
	core::render_impl::updateObject(fix.render_context, data);
	BOOST_CHECK_GT(data.fov.getRadius(), 0.f);
}

/*
BOOST_AUTO_TEST_CASE(bot_is_hostile_if_respawned_by_hostile_bot) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 5.f;
	fix.entity.display_name = "obstacle";

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	spawn.pos.x += 1;
	auto causer = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	fix.objects.push_back(id);
	fix.objects.push_back(causer);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, causer);
	fix.update();
	
	BOOST_REQUIRE(fix.script.has(id));
	auto& script = fix.script.query(id);
	BOOST_CHECK(script.api->hostile);
}

BOOST_AUTO_TEST_CASE(hostile_bot_is_hostile_if_respawned_without_actor) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "obstacle";
	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	fix.objects.push_back(id);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, 0u);
	fix.update();
	
	BOOST_REQUIRE(fix.script.has(id));
	auto& script = fix.script.query(id);
	BOOST_CHECK(script.api->hostile);
}

BOOST_AUTO_TEST_CASE(allied_bot_is_allied_if_respawned_without_actor) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "obstacle";
	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, false);
	fix.objects.push_back(id);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, 0u);
	fix.update();
	
	BOOST_REQUIRE(fix.script.has(id));
	auto& script = fix.script.query(id);
	BOOST_CHECK(!script.api->hostile);
}

BOOST_AUTO_TEST_CASE(bot_is_not_hostile_if_respawned_by_player) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "foo";
	fix.player_res.level = 12u;
	fix.player_res.exp = rpg::getNextExp(fix.player_res.level);
	fix.player_res.attributes[rpg::Attribute::Strength] = 25u;
	fix.player_res.attributes[rpg::Attribute::Dexterity] = 55u;
	fix.player_res.attributes[rpg::Attribute::Wisdom] = 10u;

	rpg::ItemTemplate sword;
	rpg::PerkTemplate fireball;
	fix.player_res.inventory.emplace_back("sword", 1u, &sword);
	fix.player_res.perks.emplace_back("fireball", 2u, &fireball);
	fix.player_res.slot_id = 2u;
	std::get<1>(fix.player_res.slots[0u]) = "fireball";
	std::get<3>(fix.player_res.slots[0u]) = &fireball;
	std::get<0>(fix.player_res.slots[2u]) = "sword";
	std::get<2>(fix.player_res.slots[2u]) = &sword;

	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	spawn.pos.x += 1;
	auto causer = fix.factory.createPlayer(fix.player_res, fix.keys, spawn);
	fix.objects.push_back(id);
	fix.objects.push_back(causer);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, causer);
	fix.update();
	
	BOOST_REQUIRE(fix.script.has(id));
	auto& script = fix.script.query(id);
	BOOST_CHECK(!script.api->hostile);
}

BOOST_AUTO_TEST_CASE(bot_is_not_hostile_if_respawned_by_allied_bot) {
	auto& fix = Singleton<FactoryFixture>::get();
	fix.reset();
	fix.entity.collide = true;
	fix.entity.max_sight = 10.f;
	fix.entity.display_name = "obstacle";
	rpg::SpawnMetaData spawn;
	spawn.scene = 1u;
	spawn.pos = {5u, 5u};
	spawn.direction = {1, 0};

	auto id = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, true);
	spawn.pos.x += 1;
	auto allied = fix.factory.createBot(fix.bot, spawn, 10u, fix.ai_script, false);
	fix.objects.push_back(id);
	fix.objects.push_back(allied);
	fix.onCharacterDied(id);
	fix.cleanup();
	fix.onCharacterSpawned(id, allied);
	fix.update();
	
	BOOST_REQUIRE(fix.script.has(id));
	auto& script = fix.script.query(id);
	BOOST_CHECK(!script.api->hostile);
}
*/

BOOST_AUTO_TEST_SUITE_END()
