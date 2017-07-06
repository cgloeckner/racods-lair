#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <core/collision.hpp>
#include <game/mod.hpp>

struct ModFixture {
	core::LogContext log;
	game::ResourceCache cache;
	game::Mod mod;
	sf::Texture texture;
	sf::SoundBuffer sound;
	rpg::SpriteTemplate sprite;
	rpg::EntityTemplate entity;
	rpg::EffectTemplate effect;
	rpg::BulletTemplate bullet;
	rpg::ItemTemplate item;
	rpg::PerkTemplate perk;

	ModFixture()
		: log{}
		, cache{}
		, mod{log, cache, "data"}
		, sprite{}
		, entity{}
		, effect{}
		, bullet{}
		, item{}
		, perk{} {
		//log.debug.add(std::cout);
	}
};

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(mod_test)

BOOST_AUTO_TEST_CASE(mod_get_music_path_delivers_relative_path) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<sf::Music>("ambience");
	BOOST_CHECK_EQUAL(fname, "data/music/ambience.ogg");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_gfx_path_for_textures) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<sf::Texture>("goblin/torso");
	BOOST_CHECK_EQUAL(fname, "data/gfx/goblin/torso.png");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_sfx_path_for_soundbuffer) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<sf::SoundBuffer>("goblin/die");
	BOOST_CHECK_EQUAL(fname, "data/sfx/goblin/die.ogg");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_font_path_for_font) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<sf::Font>("system");
	BOOST_CHECK_EQUAL(fname, "data/font/system.ttf");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_tileset) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<rpg::TilesetTemplate>("dungeon");
	BOOST_CHECK_EQUAL(fname, "data/xml/tileset/dungeon.xml");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_entity) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<rpg::EntityTemplate>("goblin-warrior");
	BOOST_CHECK_EQUAL(fname, "data/xml/entity/goblin-warrior.xml");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_effect) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<rpg::EffectTemplate>("poisoned");
	BOOST_CHECK_EQUAL(fname, "data/xml/effect/poisoned.xml");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_bullet) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<rpg::BulletTemplate>("arrow");
	BOOST_CHECK_EQUAL(fname, "data/xml/bullet/arrow.xml");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_item) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<rpg::ItemTemplate>("enhanced-longbow");
	BOOST_CHECK_EQUAL(fname, "data/xml/item/enhanced-longbow.xml");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_perk) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<rpg::PerkTemplate>("fireball");
	BOOST_CHECK_EQUAL(fname, "data/xml/perk/fireball.xml");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_trap) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<rpg::TrapTemplate>("auto-arrow");
	BOOST_CHECK_EQUAL(fname, "data/xml/trap/auto-arrow.xml");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_bot) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<game::BotTemplate>("goblin-warrior");
	BOOST_CHECK_EQUAL(fname, "data/xml/bot/goblin-warrior.xml");
}

BOOST_AUTO_TEST_CASE(mod_get_filename_delivers_xml_path_for_rom) {
	auto& fix = Singleton<ModFixture>::get();

	auto fname = fix.mod.get_filename<game::RoomTemplate>("throne-room");
	BOOST_CHECK_EQUAL(fname, "data/xml/room/throne-room.xml");
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_tileset_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TilesetTemplate resource;
	resource.tileset_name = "foo";
	resource.tilesize = {64u, 64u};
	resource.floors.emplace_back(0u, 0u);
	resource.walls.emplace_back(1u, 0u);
	resource.tileset = &fix.texture;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(tileset_without_tileset_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TilesetTemplate resource;
	resource.tileset_name = "";
	resource.tilesize = {64u, 64u};
	resource.floors.emplace_back(0u, 0u);
	resource.walls.emplace_back(1u, 0u);
	resource.tileset = &fix.texture;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(tileset_without_zero_tilewidth_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TilesetTemplate resource;
	resource.tileset_name = "foo";
	resource.tilesize = {0u, 64u};
	resource.floors.emplace_back(0u, 0u);
	resource.walls.emplace_back(1u, 0u);
	resource.tileset = &fix.texture;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(tileset_without_zero_tileheight_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TilesetTemplate resource;
	resource.tileset_name = "foo";
	resource.tilesize = {64u, 0u};
	resource.floors.emplace_back(0u, 0u);
	resource.walls.emplace_back(1u, 0u);
	resource.tileset = &fix.texture;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(tileset_without_rooms_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TilesetTemplate resource;
	resource.tileset_name = "foo";
	resource.tilesize = {64u, 64u};
	resource.walls.emplace_back(1u, 0u);
	resource.tileset = &fix.texture;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(tileset_without_walls_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TilesetTemplate resource;
	resource.tileset_name = "foo";
	resource.tilesize = {64u, 64u};
	resource.floors.emplace_back(0u, 0u);
	resource.tileset = &fix.texture;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(tileset_without_loaded_tileset_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();
	
	rpg::TilesetTemplate resource;
	resource.tileset_name = "foo";
	resource.tilesize = {64u, 64u};
	resource.floors.emplace_back(0u, 0u);
	resource.walls.emplace_back(1u, 0u);
	resource.tileset = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_sprite_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::SpriteTemplate resource;
	resource.legs.append({}, {}, sf::milliseconds(1));
	resource.legs.refresh();
	for (auto& pair : resource.torso) {
		pair.second.append({}, {}, sf::milliseconds(1));
		pair.second.refresh();
	}
	resource.frameset_name = "foo";
	resource.frameset = &fix.texture;
	resource.edges.resize(3u);
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(sprite_without_edges_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::SpriteTemplate resource;
	resource.legs.append({}, {}, sf::milliseconds(1));
	resource.legs.refresh();
	for (auto& pair : resource.torso) {
		pair.second.append({}, {}, sf::milliseconds(1));
		pair.second.refresh();
	}
	resource.frameset_name = "foo";
	resource.frameset = &fix.texture;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(sprite_with_less_then_three_edges_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::SpriteTemplate resource;
	resource.legs.append({}, {}, sf::milliseconds(1));
	resource.legs.refresh();
	for (auto& pair : resource.torso) {
		pair.second.append({}, {}, sf::milliseconds(1));
		pair.second.refresh();
	}
	resource.frameset_name = "foo";
	resource.frameset = &fix.texture;
	resource.edges.resize(2u);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(sprite_without_legs_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::SpriteTemplate resource;
	for (auto& pair : resource.torso) {
		pair.second.append({}, {}, sf::milliseconds(1));
		pair.second.refresh();
	}
	resource.frameset_name = "foo";
	resource.frameset = &fix.texture;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(sprite_with_only_idle_torso_frames_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::SpriteTemplate resource;
	resource.torso[core::AnimationAction::Idle].append(
		{}, {}, sf::milliseconds(1));
	resource.torso[core::AnimationAction::Idle].refresh();
	resource.frameset_name = "foo";
	resource.frameset = &fix.texture;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(sprite_without_torso_frames_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::SpriteTemplate resource;
	resource.frameset_name = "foo";
	resource.frameset = &fix.texture;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(sprite_without_torso_duration_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::SpriteTemplate resource;
	resource.torso[core::AnimationAction::Idle].append({}, {}, sf::Time::Zero);
	resource.torso[core::AnimationAction::Idle].refresh();
	resource.frameset_name = "foo";
	resource.frameset = &fix.texture;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(sprite_without_frameset_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::SpriteTemplate resource;
	resource.torso[core::AnimationAction::Idle].append(
		{}, {}, sf::milliseconds(1));
	resource.torso[core::AnimationAction::Idle].refresh();
	resource.frameset_name = "";
	resource.frameset = &fix.texture;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(sprite_without_loaded_frameset_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();
	
	rpg::SpriteTemplate resource;
	resource.torso[core::AnimationAction::Idle].append(
		{}, {}, sf::milliseconds(1));
	resource.torso[core::AnimationAction::Idle].refresh();
	resource.frameset_name = "foo";
	resource.frameset = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_entity_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 0.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.sounds[core::default_value<core::SoundAction>()].emplace_back("test", &fix.sound);
	resource.light = std::make_unique<utils::Light>();
	resource.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(interactable_entity_without_display_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 0.f;
	resource.max_speed = 10.f;
	resource.display_name = "";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.sounds[core::default_value<core::SoundAction>()].emplace_back("test", &fix.sound);
	resource.light = std::make_unique<utils::Light>();
	resource.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(interactable_entity_with_non_negative_sight_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 3.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.sounds[core::default_value<core::SoundAction>()].emplace_back("test", &fix.sound);
	resource.light = std::make_unique<utils::Light>();
	resource.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_any_sound_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 0.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.light = std::make_unique<utils::Light>();
	resource.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_any_sound_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 0.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.sounds[core::default_value<core::SoundAction>()].emplace_back("", &fix.sound);
	resource.light = std::make_unique<utils::Light>();
	resource.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_any_sound_buffer_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 0.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.sounds[core::default_value<core::SoundAction>()].emplace_back("test", nullptr);
	resource.light = std::make_unique<utils::Light>();
	resource.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_light_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 0.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_interact_type_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.light = std::make_unique<utils::Light>();
	resource.interact = nullptr;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_with_negative_sight_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = -2.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_with_negative_speed_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = -10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_with_too_large_sight_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 100000.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_with_too_large_speed_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = 100000.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_sprite_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "";
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_loaded_sprite_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();
	
	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_soundname_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.sounds[core::default_value<core::SoundAction>()].emplace_back("", &fix.sound);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_without_loaded_sound_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = 10.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.sounds[core::default_value<core::SoundAction>()].emplace_back("test", nullptr);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_barrier_without_speed_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = 0.f;
	resource.display_name = "bar";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	resource.light = std::make_unique<utils::Light>();
	resource.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(entity_with_sight_radius_but_without_display_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EntityTemplate resource;
	resource.is_projectile = false;
	resource.max_sight = 2.f;
	resource.max_speed = 10.f;
	resource.display_name = "";
	resource.sprite_name = "foo";
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_effect_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EffectTemplate resource;
	resource.display_name = "bar";
	resource.inflict_sound = "foo";
	resource.duration = sf::Time::Zero;
	resource.sound = &fix.sound;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(effect_without_inflict_sound_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();
	
	rpg::EffectTemplate resource;
	resource.display_name = "bar";
	resource.inflict_sound = "";
	resource.duration = sf::Time::Zero;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(effect_without_display_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::EffectTemplate resource;
	resource.display_name = "";
	resource.inflict_sound = "foo";
	resource.duration = sf::Time::Zero;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(effect_without_loaded_sound_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();
	
	rpg::EffectTemplate resource;
	resource.display_name = "bar";
	resource.inflict_sound = "foo";
	resource.duration = sf::Time::Zero;
	resource.sound = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_bullet_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::BulletTemplate resource;
	resource.entity_name = "foo";
	resource.radius = 0.25f;
	resource.entity = &fix.entity;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bullet_without_entity_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::BulletTemplate resource;
	resource.entity_name = "";
	resource.radius = 0.25f;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bullet_with_negative_radius_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::BulletTemplate resource;
	resource.entity_name = "foo";
	resource.radius = -0.1f;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bullet_with_too_large_radius_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::BulletTemplate resource;
	resource.entity_name = "foo";
	resource.radius = core::collision_impl::MAX_PROJECTILE_RADIUS + 0.1f;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bullet_without_loaded_entity_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();
	
	rpg::BulletTemplate resource;
	resource.entity_name = "foo";
	resource.radius = 0.5f;
	resource.entity = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_item_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;

	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_use_sound_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_bullet_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "";
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_effect_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = true;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "";
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_display_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_icon_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(equipment_item_without_sprite_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Armor;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "";
	resource.slot = rpg::EquipmentSlot::Body;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_loaded_bullet_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = nullptr;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_loaded_effect_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = nullptr;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_loaded_icon_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = nullptr;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_loaded_sound_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = nullptr;
	resource.sprite = &fix.sprite;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_without_loaded_sprite_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_with_zero_effect_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.0f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	resource.require[rpg::Attribute::Dexterity] = 5;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_with_negative_effect_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = -0.5f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	resource.require[rpg::Attribute::Dexterity] = 5;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(item_with_too_large_effect_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Misc;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 1.1f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	resource.require[rpg::Attribute::Dexterity] = 5;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(range_weapon_item_without_bullet_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Weapon;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::Weapon;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "";
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	resource.require[rpg::Attribute::Dexterity] = 5;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(weapon_item_without_slot_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Weapon;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	resource.require[rpg::Attribute::Dexterity] = 5;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(armor_item_without_slot_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.type = rpg::ItemType::Armor;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.sprite_name = "abc";
	resource.slot = rpg::EquipmentSlot::None;
	resource.melee = false;
	resource.two_handed = false;
	resource.worth = 0;
	resource.bullet.name = "baz";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "kk";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.sprite = &fix.sprite;
	resource.require[rpg::Attribute::Dexterity] = 5;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(revive_item_that_is_a_potion_with_life_recovery_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.revive = true;
	resource.type = rpg::ItemType::Potion;
	resource.recover[rpg::Stat::Life] = 3;

	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(revive_item_that_is_no_potion_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.revive = true;
	resource.type = rpg::ItemType::Misc;
	resource.recover[rpg::Stat::Life] = 3;

	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(
	revive_item_that_is_a_potion_without_life_recovery_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::ItemTemplate resource;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.revive = true;
	resource.type = rpg::ItemType::Potion;
	resource.recover[rpg::Stat::Life] = 0;

	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_perk_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_without_use_sound_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(defensive_perk_with_bullet_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Self;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_with_bullet_name_but_without_bullet_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_without_effect_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "";
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_without_display_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_without_icon_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_with_negative_effect_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = -0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_with_zero_effect_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_without_loaded_bullet_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = nullptr;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_without_loaded_effect_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = nullptr;
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_without_loaded_icon_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = nullptr;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(perk_without_loaded_sound_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.type = rpg::PerkType::Enemy;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.bullet.name = "abc";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "asdf";
	resource.effect.ratio = 0.2f;
	resource.effect.effect = &fix.effect;
	resource.icon = &fix.texture;
	resource.sound = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(
	revive_perk_that_is_supporting_and_has_life_recovery_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.icon = &fix.texture;
	resource.revive = true;
	resource.type = rpg::PerkType::Allied;
	resource.recover[rpg::Stat::Life] = 2;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(revive_perk_that_is_not_supporting_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.revive = true;
	resource.type = rpg::PerkType::Enemy;
	resource.recover[rpg::Stat::Life] = 2;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(revive_perk_without_life_recovery_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::PerkTemplate resource;
	resource.display_name = "foo";
	resource.icon_name = "bar";
	resource.use_sound = "test";
	resource.icon = &fix.texture;
	resource.sound = &fix.sound;
	resource.revive = true;
	resource.type = rpg::PerkType::Allied;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_trap_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.trigger_sound = "test";
	resource.bullet.name = "foo";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "bar";
	resource.effect.ratio = 0.3f;
	resource.effect.effect = &fix.effect;
	resource.sound = &fix.sound;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_without_trigger_sound_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.trigger_sound = "";
	resource.bullet.name = "foo";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "bar";
	resource.effect.ratio = 0.3f;
	resource.effect.effect = &fix.effect;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_without_effect_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.trigger_sound = "test";
	resource.bullet.name = "foo";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "";
	resource.sound = &fix.sound;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_without_bullet_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.bullet.name = "";
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_without_loaded_bullet_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.bullet.name = "foo";
	resource.bullet.bullet = nullptr;
	resource.effect.name = "bar";
	resource.effect.ratio = 0.3f;
	resource.effect.effect = &fix.effect;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_with_negative_effect_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.trigger_sound = "test";
	resource.bullet.name = "foo";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "bar";
	resource.effect.ratio = -0.3f;
	resource.effect.effect = &fix.effect;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_with_zero_effect_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.trigger_sound = "test";
	resource.bullet.name = "foo";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "bar";
	resource.effect.ratio = 0.f;
	resource.effect.effect = &fix.effect;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_with_too_large_effect_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.trigger_sound = "test";
	resource.bullet.name = "foo";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "bar";
	resource.effect.ratio = 1.3f;
	resource.effect.effect = &fix.effect;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_without_loaded_effect_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.trigger_sound = "test";
	resource.bullet.name = "foo";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "bar";
	resource.effect.ratio = 0.3f;
	resource.effect.effect = nullptr;
	resource.sound = &fix.sound;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(trap_without_loaded_sound_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	rpg::TrapTemplate resource;
	resource.trigger_sound = "test";
	resource.bullet.name = "foo";
	resource.bullet.bullet = &fix.bullet;
	resource.effect.name = "bar";
	resource.effect.ratio = 0.3f;
	resource.effect.effect = &fix.effect;
	resource.sound = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_bot_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 1.f;
	resource.defense[rpg::DamageType::Blade] = 3.f;
	resource.properties[rpg::Property::MeleeBase] = 2.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 3.f;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_without_displayname_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 1.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 3.f;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_without_items_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 1.f;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_without_perks_is_still_valid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 1.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 2.f;
	std::get<2>(item) = &fix.item;
	resource.entity = &fix.entity;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_without_entity_name_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.entity_name = "";
	resource.attributes[rpg::Attribute::Dexterity] = 1.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_with_negative_attribute_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = -1.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_with_too_large_attribute_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 1.1f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_with_too_large_attribute_sum_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Strength] = 0.6f;
	resource.attributes[rpg::Attribute::Dexterity] = 0.6f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_with_negative_defense_boni_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.defense[rpg::DamageType::Blade] = -1.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_with_negative_property_boni_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.properties[rpg::Property::MeleeBase] = -1.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_with_zero_item_quantity_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 1.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 0.f;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_without_loaded_item_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 2.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = nullptr;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_with_zero_perk_level_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 2.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_without_loaded_perk_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 2.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = nullptr;
	resource.entity = &fix.entity;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(bot_without_loaded_entity_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::BotTemplate resource;
	resource.display_name = "foo";
	resource.entity_name = "bar";
	resource.attributes[rpg::Attribute::Dexterity] = 2.f;
	resource.items.resize(1u);
	auto& item = resource.items.front();
	std::get<0>(item) = "test";
	std::get<1>(item) = 15u;
	std::get<2>(item) = &fix.item;
	resource.perks.resize(1u);
	auto& perk = resource.perks.front();
	std::get<0>(perk) = "asdf";
	std::get<1>(perk) = 0.5f;
	std::get<2>(perk) = &fix.perk;
	resource.entity = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_encounter_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	game::EncounterTemplate resource;
	game::BotTemplate bot;
	resource.bots.emplace_back("foo", 0.7f, &bot);
	resource.bots.emplace_back("bar", 0.3f, &bot);
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(encounter_without_bots_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::EncounterTemplate resource;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(encounter_with_too_low_total_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::EncounterTemplate resource;
	game::BotTemplate bot;
	resource.bots.emplace_back("foo", 0.6f, &bot);
	resource.bots.emplace_back("bar", 0.3f, &bot);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(encounter_with_too_high_total_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::EncounterTemplate resource;
	game::BotTemplate bot;
	resource.bots.emplace_back("foo", 0.6f, &bot);
	resource.bots.emplace_back("bar", 0.5f, &bot);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(encounter_with_too_low_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::EncounterTemplate resource;
	game::BotTemplate bot;
	resource.bots.emplace_back("foo", 0.0f, &bot);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(encounter_with_too_high_ratio_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::EncounterTemplate resource;
	game::BotTemplate bot;
	resource.bots.emplace_back("foo", 1.1f, &bot);
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(complete_room_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	game::RoomTemplate resource;
	auto& c = resource.create({2u, 3u});
	c.entity.name = "foo";
	c.entity.ptr = &fix.entity;
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(room_without_entity_name_is_valid) {
	auto& fix = Singleton<ModFixture>::get();

	game::RoomTemplate resource;
	auto& c = resource.create({2u, 3u});
	c.entity.name = "";
	BOOST_CHECK(game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_CASE(room_without_entity_ptr_is_invalid) {
	auto& fix = Singleton<ModFixture>::get();

	game::RoomTemplate resource;
	auto& c = resource.create({2u, 3u});
	c.entity.name = "foo";
	c.entity.ptr = nullptr;
	BOOST_CHECK(!game::mod_impl::verify(fix.log.debug, "", resource));
}

BOOST_AUTO_TEST_SUITE_END()
