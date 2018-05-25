#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

#include <boost/property_tree/xml_parser.hpp>

#include <rpg/resources.hpp>

BOOST_AUTO_TEST_SUITE(resources_test)

BOOST_AUTO_TEST_CASE(saving_and_loading_enummap_iterates_all_values) {
	// prepare damagetypes
	utils::EnumMap<rpg::Stat, int> data;
	data[rpg::Stat::Life] = 300;
	data[rpg::Stat::Mana] = 150;
	data[rpg::Stat::Stamina] = 200;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(dump(ptree, data, ""));

	// load (twice!)
	utils::EnumMap<rpg::Stat, int> loaded;
	BOOST_REQUIRE_NO_THROW(parse(ptree, loaded, ""));
	BOOST_REQUIRE_NO_THROW(parse(ptree, loaded, ""));

	BOOST_CHECK_EQUAL(data[rpg::Stat::Life], 300);
	BOOST_CHECK_EQUAL(data[rpg::Stat::Mana], 150);
	BOOST_CHECK_EQUAL(data[rpg::Stat::Stamina], 200);
}

BOOST_AUTO_TEST_CASE(
	saving_and_loading_enummap_with_default_values_is_possible) {
	// prepare damagetypes
	utils::EnumMap<rpg::Stat, int> data;
	data[rpg::Stat::Life] = 300;
	data[rpg::Stat::Mana] = 150;
	data[rpg::Stat::Stamina] = 200;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(dump(ptree, data, "", 0));

	// load (twice!)
	utils::EnumMap<rpg::Stat, int> loaded;
	BOOST_REQUIRE_NO_THROW(parse(ptree, loaded, "", 0));
	BOOST_REQUIRE_NO_THROW(parse(ptree, loaded, "", 0));

	BOOST_CHECK_EQUAL(data[rpg::Stat::Life], 300);
	BOOST_CHECK_EQUAL(data[rpg::Stat::Mana], 150);
	BOOST_CHECK_EQUAL(data[rpg::Stat::Stamina], 200);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_keyboard_keybinding_iterates_all_actions) {
	// prepare keys
	rpg::Keybinding keys;
	keys.map.set(rpg::PlayerAction::Pause, {sf::Keyboard::F1});
	keys.map.set(rpg::PlayerAction::ToggleAutoLook, {sf::Keyboard::F2});
	keys.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::Up});
	keys.map.set(rpg::PlayerAction::MoveE, {sf::Keyboard::Right});
	keys.map.set(rpg::PlayerAction::MoveS, {sf::Keyboard::Down});
	keys.map.set(rpg::PlayerAction::MoveW, {sf::Keyboard::Left});
	keys.map.set(rpg::PlayerAction::LookN, {sf::Keyboard::W});
	keys.map.set(rpg::PlayerAction::LookE, {sf::Keyboard::D});
	keys.map.set(rpg::PlayerAction::LookS, {sf::Keyboard::S});
	keys.map.set(rpg::PlayerAction::LookW, {sf::Keyboard::A});
	keys.map.set(rpg::PlayerAction::Attack, {sf::Keyboard::F7});
	keys.map.set(rpg::PlayerAction::Interact, {sf::Keyboard::F6});
	keys.map.set(rpg::PlayerAction::UseSlot, {sf::Keyboard::F5});
	keys.map.set(rpg::PlayerAction::PrevSlot, {sf::Keyboard::F4});
	keys.map.set(rpg::PlayerAction::NextSlot, {sf::Keyboard::F3});

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(keys.saveToTree(ptree));

	// load (twice!)
	rpg::Keybinding data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// check some data
	BOOST_CHECK(data.map.get(rpg::PlayerAction::Pause) ==
				keys.map.get(rpg::PlayerAction::Pause));
	BOOST_CHECK(data.map.get(rpg::PlayerAction::MoveW) ==
				keys.map.get(rpg::PlayerAction::MoveW));
	BOOST_CHECK(data.map.get(rpg::PlayerAction::LookN) ==
				keys.map.get(rpg::PlayerAction::LookN));
	BOOST_CHECK(data.map.get(rpg::PlayerAction::UseSlot) ==
				keys.map.get(rpg::PlayerAction::UseSlot));
	BOOST_CHECK(!data.is_gamepad);
}

BOOST_AUTO_TEST_CASE(saving_and_loading_gamepad_keybinding_iterates_all_actions) {
	// prepare keys
	rpg::Keybinding keys;
	keys.map.set(rpg::PlayerAction::Pause, {0, 5u});
	keys.map.set(rpg::PlayerAction::ToggleAutoLook, {0u, 6u});
	keys.map.set(rpg::PlayerAction::MoveN, {0u, sf::Joystick::Axis::U, -25.f});
	keys.map.set(rpg::PlayerAction::MoveE, {0u, sf::Joystick::Axis::V, -25.f});
	keys.map.set(rpg::PlayerAction::MoveS, {0u, sf::Joystick::Axis::U, 25.f});
	keys.map.set(rpg::PlayerAction::MoveW, {0u, sf::Joystick::Axis::V, 25.f});
	keys.map.set(rpg::PlayerAction::LookN, {0u, sf::Joystick::Axis::Y, -25.f});
	keys.map.set(rpg::PlayerAction::LookE, {0u, sf::Joystick::Axis::X, 25.f});
	keys.map.set(rpg::PlayerAction::LookS, {0u, sf::Joystick::Axis::Y, 25.f});
	keys.map.set(rpg::PlayerAction::LookW, {0u, sf::Joystick::Axis::X, -25.f});
	keys.map.set(rpg::PlayerAction::Attack, {0u, 0u});
	keys.map.set(rpg::PlayerAction::Interact, {0u, 1u});
	keys.map.set(rpg::PlayerAction::UseSlot, {0u, 2u});
	keys.map.set(rpg::PlayerAction::PrevSlot, {0u, 3u});
	keys.map.set(rpg::PlayerAction::NextSlot, {0u, 4u});

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(keys.saveToTree(ptree));

	// load (twice!)
	rpg::Keybinding data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// check some data
	BOOST_CHECK(data.map.get(rpg::PlayerAction::Pause) ==
				keys.map.get(rpg::PlayerAction::Pause));
	BOOST_CHECK(data.map.get(rpg::PlayerAction::MoveW) ==
				keys.map.get(rpg::PlayerAction::MoveW));
	BOOST_CHECK(data.map.get(rpg::PlayerAction::LookN) ==
				keys.map.get(rpg::PlayerAction::LookN));
	BOOST_CHECK(data.map.get(rpg::PlayerAction::UseSlot) ==
				keys.map.get(rpg::PlayerAction::UseSlot));
	BOOST_CHECK(data.is_gamepad);
}

/*
BOOST_AUTO_TEST_CASE(inconsistent_keybinding_cannot_be_load) {
	// prepare keys
	rpg::Keybinding keys;
	keys.map.set(rpg::PlayerAction::Pause, {sf::Keyboard::F1});
	keys.map.set(rpg::PlayerAction::ToggleAutoLook, {sf::Keyboard::F2});
	keys.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::Up});
	keys.map.set(rpg::PlayerAction::MoveE, {sf::Keyboard::Right});
	keys.map.set(rpg::PlayerAction::MoveS, {sf::Keyboard::Down});
	keys.map.set(rpg::PlayerAction::MoveW, {sf::Keyboard::Left});
	keys.map.set(rpg::PlayerAction::LookN, {0u, sf::Joystick::Axis::Y, -25.f});
	keys.map.set(rpg::PlayerAction::LookE, {0u, sf::Joystick::Axis::X, 25.f});
	keys.map.set(rpg::PlayerAction::LookS, {0u, sf::Joystick::Axis::Y, 25.f});
	keys.map.set(rpg::PlayerAction::LookW, {0u, sf::Joystick::Axis::X, -25.f});
	keys.map.set(rpg::PlayerAction::Attack, {0u, 0u});
	keys.map.set(rpg::PlayerAction::Interact, {0u, 1u});
	keys.map.set(rpg::PlayerAction::UseSlot, {0u, 2u});
	keys.map.set(rpg::PlayerAction::PrevSlot, {0u, 3u});
	keys.map.set(rpg::PlayerAction::NextSlot, {0u, 4u});

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(keys.saveToTree(ptree));

	// load (twice!)
	rpg::Keybinding data;
	BOOST_REQUIRE_THROW(data.loadFromTree(ptree), utils::ptree_error);
	BOOST_REQUIRE_THROW(data.loadFromTree(ptree), utils::ptree_error);
}
*/

// ---------------------------------------------------------------------------
/*
BOOST_AUTO_TEST_CASE(saving_and_loading_avatar_progress_iterates_all_data) {
	// prepare keys
	rpg::AvatarProgress ava;
	ava.level = 5u;
	ava.exp = 25348532u;
	ava.attributes[rpg::Attribute::Strength] = 45;
	ava.properties[rpg::Property::MaxMana] = 250;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(ava.saveToTree(ptree));

	// load (twice!)
	rpg::AvatarProgress loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(loaded.level, 5u);
	BOOST_CHECK_EQUAL(loaded.exp, 25348532u);
	BOOST_CHECK_EQUAL(loaded.attributes[rpg::Attribute::Strength], 45);
	BOOST_CHECK_EQUAL(loaded.properties[rpg::Property::MaxMana], 250);
}
*/
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(loading_and_saving_tileset_iterates_all_existing_data) {
	// prepare data
	rpg::TilesetTemplate tileset;
	tileset.tileset_name = "dungeon";
	tileset.tilesize.x = 64;
	tileset.tilesize.y = 96;
	tileset.floors.emplace_back(0, 0);
	tileset.floors.emplace_back(64, 0);
	tileset.walls.emplace_back(128, 0);

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(tileset.saveToTree(ptree));

	// load (twice!)
	rpg::TilesetTemplate data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// test some data
	BOOST_CHECK_EQUAL(data.tileset_name, "dungeon");
	BOOST_CHECK_VECTOR_EQUAL(data.tilesize, sf::Vector2u(64, 96));
	BOOST_REQUIRE_EQUAL(data.floors.size(), 2u);
	BOOST_REQUIRE_EQUAL(data.walls.size(), 1u);
	BOOST_CHECK_VECTOR_EQUAL(data.walls[0], sf::Vector2u(128, 0));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(loading_and_save_sprite_iterates_all_existing_data) {
	// prepare entity
	rpg::SpriteTemplate sprite;
	sprite.frameset_name = "goblin/warrior";
	sprite.legs.append({0, 0, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	sprite.legs.append({32, 0, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	sprite.legs.append({64, 0, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	sprite.torso[core::AnimationAction::Idle].append(
		{0, 32, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	sprite.torso[core::AnimationAction::Use].append(
		{32, 32, 32, 32}, {0.f, 0.f}, sf::milliseconds(100));
	sprite.torso[core::AnimationAction::Use].append(
		{64, 32, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	sprite.edges.resize(3u);
	sprite.edges[0].u = {0.f, 0.f};
	sprite.edges[0].v = {1.f, 0.f};
	sprite.edges[1].u = {1.f, 0.f};
	sprite.edges[1].v = {0.5f, 0.5f};
	sprite.edges[2].u = {0.5f, 0.5f};
	sprite.edges[2].v = {0.f, 0.f};

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(sprite.saveToTree(ptree));

	// load (twice!)
	rpg::SpriteTemplate data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// test some data
	BOOST_CHECK_EQUAL(data.frameset_name, "goblin/warrior");
	BOOST_REQUIRE_EQUAL(3u, data.legs.frames.size());
	BOOST_CHECK_RECT_EQUAL(
		sf::IntRect(32, 0, 32, 32), data.legs.frames[1].clip);
	BOOST_REQUIRE_EQUAL(
		2u, data.torso[core::AnimationAction::Use].frames.size());
	BOOST_CHECK_EQUAL(100u, data.torso[core::AnimationAction::Use]
								.frames[0]
								.duration.asMilliseconds());
	BOOST_CHECK(data.torso[core::AnimationAction::Die].frames.empty());
	BOOST_REQUIRE_EQUAL(data.edges.size(), 3u);
	BOOST_CHECK_VECTOR_CLOSE(data.edges[1].v, data.edges[1].v, 0.001f);
}

BOOST_AUTO_TEST_CASE(sprite_with_legs_is_animated) {
	rpg::SpriteTemplate sprite;
	sprite.legs.append({0, 0, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	BOOST_CHECK(sprite.isAnimated());
}

BOOST_AUTO_TEST_CASE(sprite_with_multiple_idle_frames_is_animated) {
	rpg::SpriteTemplate sprite;
	sprite.torso[core::AnimationAction::Idle].append(
		{0, 32, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	sprite.torso[core::AnimationAction::Idle].append(
		{64, 32, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	BOOST_CHECK(sprite.isAnimated());
}

BOOST_AUTO_TEST_CASE(sprite_with_other_frames_is_animated) {
	rpg::SpriteTemplate sprite;
	sprite.torso[core::AnimationAction::Idle].append(
		{0, 32, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	sprite.torso[core::AnimationAction::Use].append(
		{32, 32, 32, 32}, {0.f, 0.f}, sf::milliseconds(100));
	BOOST_CHECK(sprite.isAnimated());
}

BOOST_AUTO_TEST_CASE(
	sprite_without_legs_with_one_idle_frame_and_without_other_frames_is_not_animated) {
	rpg::SpriteTemplate sprite;
	sprite.torso[core::AnimationAction::Idle].append(
		{0, 32, 32, 32}, {0.f, 0.f}, sf::milliseconds(150));
	BOOST_CHECK(!sprite.isAnimated());
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(loading_and_save_entity_with_circle_collision_iterates_all_existing_data) {
	// prepare entity
	rpg::EntityTemplate entity;
	entity.is_projectile = true;
	entity.collide = true;
	entity.flying = true;
	entity.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier); // doesn't fit neither
	entity.max_sight = 7.5f;
	entity.max_speed = 12.667f;
	entity.fov = 120.f;
	entity.display_name = "Goblin";
	entity.sprite_name = "goblin";
	entity.shape.is_aabb = false;
	entity.shape.radius = 2.f;
	entity.sounds[core::default_value<core::SoundAction>()].emplace_back("goblin-sfx", nullptr);
	entity.light = std::make_unique<utils::Light>();
	entity.light->color = sf::Color::Yellow;
	entity.light->intensity = 200u;
	entity.light->cast_shadow = true;
	entity.light->lod = 3u;
	entity.blood_color = sf::Color::Cyan;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(entity.saveToTree(ptree));

	// load (twice!)
	rpg::EntityTemplate data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// test some data
	BOOST_CHECK_EQUAL(data.is_projectile, true);
	BOOST_CHECK_EQUAL(data.collide, false);
	BOOST_CHECK(data.flying);
	BOOST_REQUIRE(data.interact != nullptr);
	BOOST_CHECK(*data.interact == rpg::InteractType::Barrier);
	BOOST_CHECK_CLOSE(data.max_speed, 12.667f, 0.0001f);
	BOOST_CHECK_CLOSE(data.fov, 120.f, 0.0001f);
	BOOST_CHECK_EQUAL(data.display_name, "Goblin");
	BOOST_CHECK_EQUAL(data.sprite_name, "goblin");
	BOOST_CHECK(!data.shape.is_aabb);
	BOOST_CHECK_CLOSE(data.shape.radius, 2.f, 0.0001f);
	BOOST_CHECK_EQUAL(entity.sounds[core::default_value<core::SoundAction>()].front().first, "goblin-sfx");
	BOOST_REQUIRE(data.light != nullptr);
	BOOST_CHECK_COLOR_EQUAL(data.light->color, sf::Color::Yellow);
	BOOST_CHECK_EQUAL(data.light->intensity, 200u);
	BOOST_CHECK(data.light->cast_shadow);
	BOOST_CHECK_EQUAL(data.light->lod, 3u);
	BOOST_CHECK_COLOR_EQUAL(data.blood_color, sf::Color::Cyan);
}

BOOST_AUTO_TEST_CASE(loading_and_save_entity_with_aabb_collision_iterates_all_existing_data) {
	// prepare entity
	rpg::EntityTemplate entity;
	entity.is_projectile = true;
	entity.collide = true;
	entity.flying = true;
	entity.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier); // doesn't fit neither
	entity.max_sight = 7.5f;
	entity.max_speed = 12.667f;
	entity.fov = 120.f;
	entity.display_name = "Goblin";
	entity.sprite_name = "goblin";
	entity.shape.is_aabb = true;
	entity.shape.size = {1.5f, 2.3f};
	entity.sounds[core::default_value<core::SoundAction>()].emplace_back("goblin-sfx", nullptr);
	entity.light = std::make_unique<utils::Light>();
	entity.light->color = sf::Color::Yellow;
	entity.light->intensity = 200u;
	entity.light->cast_shadow = true;
	entity.light->lod = 3u;
	entity.blood_color = sf::Color::Cyan;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(entity.saveToTree(ptree));

	// load (twice!)
	rpg::EntityTemplate data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// test some data
	BOOST_CHECK_EQUAL(data.is_projectile, true);
	BOOST_CHECK_EQUAL(data.collide, false);
	BOOST_CHECK(data.flying);
	BOOST_REQUIRE(data.interact != nullptr);
	BOOST_CHECK(*data.interact == rpg::InteractType::Barrier);
	BOOST_CHECK_CLOSE(data.max_speed, 12.667f, 0.0001f);
	BOOST_CHECK_CLOSE(data.fov, 120.f, 0.0001f);
	BOOST_CHECK_EQUAL(data.display_name, "Goblin");
	BOOST_CHECK_EQUAL(data.sprite_name, "goblin");
	BOOST_CHECK(data.shape.is_aabb);
	BOOST_CHECK_VECTOR_CLOSE(data.shape.size, sf::Vector2f(1.5f, 2.3f), 0.0001f);
	BOOST_CHECK_EQUAL(entity.sounds[core::default_value<core::SoundAction>()].front().first, "goblin-sfx");
	BOOST_REQUIRE(data.light != nullptr);
	BOOST_CHECK_COLOR_EQUAL(data.light->color, sf::Color::Yellow);
	BOOST_CHECK_EQUAL(data.light->intensity, 200u);
	BOOST_CHECK(data.light->cast_shadow);
	BOOST_CHECK_EQUAL(data.light->lod, 3u);
	BOOST_CHECK_COLOR_EQUAL(data.blood_color, sf::Color::Cyan);
}

BOOST_AUTO_TEST_CASE(loading_and_save_entity_without_collider_iterates_all_existing_data) {
	// prepare entity
	rpg::EntityTemplate entity;
	entity.is_projectile = true;
	entity.collide = false;
	entity.flying = true;
	entity.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier); // doesn't fit neither
	entity.max_sight = 7.5f;
	entity.max_speed = 12.667f;
	entity.fov = 120.f;
	entity.display_name = "Goblin";
	entity.sprite_name = "goblin";
	entity.sounds[core::default_value<core::SoundAction>()].emplace_back("goblin-sfx", nullptr);
	entity.light = std::make_unique<utils::Light>();
	entity.light->color = sf::Color::Yellow;
	entity.light->intensity = 200u;
	entity.light->cast_shadow = true;
	entity.light->lod = 3u;
	entity.blood_color = sf::Color::Cyan;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(entity.saveToTree(ptree));

	// load (twice!)
	rpg::EntityTemplate data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// test some data
	BOOST_CHECK_EQUAL(data.is_projectile, true);
	BOOST_CHECK_EQUAL(data.collide, false);
	BOOST_CHECK(data.flying);
	BOOST_REQUIRE(data.interact != nullptr);
	BOOST_CHECK(*data.interact == rpg::InteractType::Barrier);
	BOOST_CHECK_CLOSE(data.max_speed, 12.667f, 0.0001f);
	BOOST_CHECK_CLOSE(data.fov, 120.f, 0.0001f);
	BOOST_CHECK_EQUAL(data.display_name, "Goblin");
	BOOST_CHECK_EQUAL(data.sprite_name, "goblin");
	BOOST_CHECK_EQUAL(entity.sounds[core::default_value<core::SoundAction>()].front().first, "goblin-sfx");
	BOOST_REQUIRE(data.light != nullptr);
	BOOST_CHECK_COLOR_EQUAL(data.light->color, sf::Color::Yellow);
	BOOST_CHECK_EQUAL(data.light->intensity, 200u);
	BOOST_CHECK(data.light->cast_shadow);
	BOOST_CHECK_EQUAL(data.light->lod, 3u);
	BOOST_CHECK_COLOR_EQUAL(data.blood_color, sf::Color::Cyan);
}

BOOST_AUTO_TEST_CASE(entity_without_blood_color_is_posible) {
	// prepare entity
	rpg::EntityTemplate entity;
	entity.is_projectile = true;
	entity.collide = false;  // doesn't fit to `is_projectile` but wayne
	entity.interact = std::make_unique<rpg::InteractType>(rpg::InteractType::Barrier); // doesn't fit neither
	entity.max_sight = 7.5f;
	entity.max_speed = 12.667f;
	entity.display_name = "Goblin";
	entity.sprite_name = "goblin";
	entity.sounds[core::default_value<core::SoundAction>()].emplace_back("goblin-sfx", nullptr);
	entity.light = std::make_unique<utils::Light>();
	entity.light->color = sf::Color::Yellow;
	entity.light->intensity = 200u;
	entity.light->cast_shadow = true;
	entity.light->lod = 3u;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(entity.saveToTree(ptree));

	// load (twice!)
	rpg::EntityTemplate data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// test some data
	BOOST_CHECK_EQUAL(data.is_projectile, true);
	BOOST_CHECK_EQUAL(data.collide, false);
	BOOST_REQUIRE(data.interact != nullptr);
	BOOST_CHECK(*data.interact == rpg::InteractType::Barrier);
	BOOST_CHECK_CLOSE(data.max_speed, 12.667f, 0.0001f);
	BOOST_CHECK_EQUAL(data.display_name, "Goblin");
	BOOST_CHECK_EQUAL(data.sprite_name, "goblin");
	BOOST_CHECK_EQUAL(entity.sounds[core::default_value<core::SoundAction>()].front().first, "goblin-sfx");
	BOOST_REQUIRE(data.light != nullptr);
	BOOST_CHECK_COLOR_EQUAL(data.light->color, sf::Color::Yellow);
	BOOST_CHECK_EQUAL(data.light->intensity, 200u);
	BOOST_CHECK(data.light->cast_shadow);
	BOOST_CHECK_EQUAL(data.light->lod, 3u);
	BOOST_CHECK_COLOR_EQUAL(data.blood_color, sf::Color::Transparent);
}

BOOST_AUTO_TEST_CASE(entity_without_interact_type_is_possible) {
	// prepare entity
	rpg::EntityTemplate entity;
	entity.is_projectile = true;
	entity.collide = false;  // doesn't fit to `is_projectile` but wayne
	entity.max_sight = 7.5f;
	entity.max_speed = 12.667f;
	entity.display_name = "Goblin";
	entity.sprite_name = "goblin";
	entity.sounds[core::default_value<core::SoundAction>()].emplace_back("goblin-sfx", nullptr);
	entity.light = std::make_unique<utils::Light>();
	entity.light->color = sf::Color::Yellow;
	entity.light->intensity = 200u;
	entity.light->cast_shadow = true;
	entity.light->lod = 3u;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(entity.saveToTree(ptree));

	// load (twice!)
	rpg::EntityTemplate data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// test some data
	BOOST_CHECK_EQUAL(data.is_projectile, true);
	BOOST_CHECK_EQUAL(data.collide, false);
	BOOST_CHECK(data.interact == nullptr);
	BOOST_CHECK_CLOSE(data.max_speed, 12.667f, 0.0001f);
	BOOST_CHECK_EQUAL(data.display_name, "Goblin");
	BOOST_CHECK_EQUAL(data.sprite_name, "goblin");
	BOOST_CHECK_EQUAL(entity.sounds[core::default_value<core::SoundAction>()].front().first, "goblin-sfx");
	BOOST_REQUIRE(data.light != nullptr);
	BOOST_CHECK_COLOR_EQUAL(data.light->color, sf::Color::Yellow);
	BOOST_CHECK_EQUAL(data.light->intensity, 200u);
	BOOST_CHECK(data.light->cast_shadow);
	BOOST_CHECK_EQUAL(data.light->lod, 3u);
}

BOOST_AUTO_TEST_CASE(loading_and_save_entity_without_light_is_possible) {
	// prepare entity
	rpg::EntityTemplate entity;
	entity.is_projectile = true;
	entity.collide = false;  // doesn't fit to `is_projectile` but wayne
	entity.max_sight = 7.5f;
	entity.max_speed = 12.667f;
	entity.display_name = "Goblin";
	entity.sprite_name = "goblin";
	entity.sounds[core::default_value<core::SoundAction>()].emplace_back("goblin-sfx", nullptr);

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(entity.saveToTree(ptree));

	// load (twice!)
	rpg::EntityTemplate data;
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(data.loadFromTree(ptree));

	// test some data
	BOOST_CHECK_EQUAL(data.is_projectile, true);
	BOOST_CHECK_EQUAL(data.collide, false);
	BOOST_CHECK_CLOSE(data.max_speed, 12.667f, 0.0001f);
	BOOST_CHECK_EQUAL(data.display_name, "Goblin");
	BOOST_CHECK_EQUAL(data.sprite_name, "goblin");
	BOOST_CHECK_EQUAL(entity.sounds[core::default_value<core::SoundAction>()].front().first, "goblin-sfx");
	BOOST_REQUIRE(data.light == nullptr);
}

BOOST_AUTO_TEST_CASE(entity_with_any_sound_name_implies_sounds) {
	rpg::EntityTemplate entity;
	entity.sounds[core::default_value<core::SoundAction>()].emplace_back("goblin-sfx", nullptr);
	BOOST_CHECK(entity.hasSounds());
}

BOOST_AUTO_TEST_CASE(entity_with_any_sound_ptr_implies_sounds) {
	rpg::EntityTemplate entity;
	sf::SoundBuffer tmp;
	entity.sounds[core::default_value<core::SoundAction>()].emplace_back("", &tmp);
	BOOST_CHECK(entity.hasSounds());
}

BOOST_AUTO_TEST_CASE(entity_without_sound_names_implies_no_sounds) {
	rpg::EntityTemplate entity;
	BOOST_CHECK(!entity.hasSounds());
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_effect_iterates_all_data) {
	// prepare effect
	rpg::EffectTemplate effect;
	effect.display_name = "Poisoned";
	effect.duration = sf::milliseconds(2500);
	effect.inflict_sound = "poison";
	effect.boni.properties[rpg::Property::MaxStamina] = -20;
	effect.recover[rpg::Stat::Mana] = 0.05f;
	effect.boni.defense[rpg::DamageType::Blunt] = -0.5f;
	effect.damage[rpg::DamageType::Poison] = 1.f;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(effect.saveToTree(ptree));

	// load (twice!)
	rpg::EffectTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(loaded.display_name, "Poisoned");
	BOOST_CHECK_TIME_EQUAL(loaded.duration, sf::milliseconds(2500));
	BOOST_CHECK_EQUAL(loaded.inflict_sound, "poison");
	BOOST_CHECK_EQUAL(loaded.boni.properties[rpg::Property::MaxStamina], -20);
	BOOST_CHECK_CLOSE(loaded.recover[rpg::Stat::Mana], 0.05f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.boni.defense[rpg::DamageType::Blunt], -0.5f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.damage[rpg::DamageType::Poison], 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	saving_and_loading_effect_without_duration_and_sound_is_possible) {
	// prepare effect
	rpg::EffectTemplate effect;
	effect.display_name = "Poisoned";
	effect.boni.properties[rpg::Property::MaxStamina] = -20;
	effect.recover[rpg::Stat::Mana] = 0.05f;
	effect.boni.defense[rpg::DamageType::Blunt] = -0.5f;
	effect.damage[rpg::DamageType::Poison] = 1.f;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(effect.saveToTree(ptree));

	// load (twice!)
	rpg::EffectTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(loaded.display_name, "Poisoned");
	BOOST_CHECK_TIME_EQUAL(loaded.duration, sf::Time::Zero);
	BOOST_CHECK(loaded.inflict_sound.empty());
	BOOST_CHECK_EQUAL(loaded.boni.properties[rpg::Property::MaxStamina], -20);
	BOOST_CHECK_CLOSE(loaded.recover[rpg::Stat::Mana], 0.05f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.boni.defense[rpg::DamageType::Blunt], -0.5f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.damage[rpg::DamageType::Poison], 1.f, 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_bullet_iterates_all_data) {
	// prepare bullet
	rpg::BulletTemplate bullet;
	bullet.entity_name = "Fireball";
	bullet.radius = 1.3f;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(bullet.saveToTree(ptree));

	// load (twice!)
	rpg::BulletTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(loaded.entity_name, bullet.entity_name);
	BOOST_CHECK_CLOSE(loaded.radius, bullet.radius, 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_item_iterates_all_data) {
	// prepare item
	rpg::ItemTemplate item;
	item.type = rpg::ItemType::Weapon;
	item.display_name = "Longbow of Pure Magic";
	item.icon_name = "bow";
	item.slot = rpg::EquipmentSlot::Weapon;
	item.melee = false;
	item.two_handed = true;
	item.worth = 123u;
	item.use_sound = "equip";
	item.bullet.name = "arrow";
	item.effect.name = "burn";
	item.effect.ratio = 0.5f;
	item.damage[rpg::DamageType::Bullet] = 1.2f;
	item.boni.defense[rpg::DamageType::Fire] = 1.f;
	item.require[rpg::Attribute::Dexterity] = 2.f;
	item.boni.properties[rpg::Property::RangeBase] = 12;
	item.recover[rpg::Stat::Stamina] = 5u;
	item.revive = true;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(item.saveToTree(ptree));

	// load (twice!)
	rpg::ItemTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK(loaded.type == rpg::ItemType::Weapon);
	BOOST_CHECK_EQUAL(loaded.display_name, "Longbow of Pure Magic");
	BOOST_CHECK_EQUAL(loaded.icon_name, "bow");
	BOOST_CHECK(loaded.slot == rpg::EquipmentSlot::Weapon);
	BOOST_CHECK(!loaded.melee);
	BOOST_CHECK(loaded.two_handed);
	BOOST_CHECK_EQUAL(loaded.worth, 123u);
	BOOST_CHECK_EQUAL(loaded.use_sound, "equip");
	BOOST_CHECK_EQUAL(loaded.bullet.name, "arrow");
	BOOST_CHECK_EQUAL(loaded.effect.name, "burn");
	BOOST_CHECK_CLOSE(loaded.effect.ratio, 0.5f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.damage[rpg::DamageType::Bullet], 1.2f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.boni.defense[rpg::DamageType::Fire], 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.require[rpg::Attribute::Dexterity], 2.f, 0.0001f);
	BOOST_CHECK_EQUAL(loaded.boni.properties[rpg::Property::RangeBase], 12);
	BOOST_CHECK_EQUAL(loaded.recover[rpg::Stat::Stamina], 5u);
	BOOST_CHECK(loaded.revive);
}

BOOST_AUTO_TEST_CASE(saving_and_loading_item_without_sound_etc_is_possible) {
	// prepare item
	rpg::ItemTemplate item;
	item.type = rpg::ItemType::Weapon;
	item.display_name = "Longbow of Pure Magic";
	item.icon_name = "bow";
	item.slot = rpg::EquipmentSlot::Weapon;
	item.two_handed = true;
	item.worth = 123u;
	item.damage[rpg::DamageType::Bullet] = 1.2f;
	item.boni.defense[rpg::DamageType::Fire] = 1.f;
	item.require[rpg::Attribute::Dexterity] = 2.f;
	item.boni.properties[rpg::Property::RangeBase] = 12u;
	item.recover[rpg::Stat::Stamina] = 5u;
	item.revive = true;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(item.saveToTree(ptree));

	// load (twice!)
	rpg::ItemTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK(loaded.type == rpg::ItemType::Weapon);
	BOOST_CHECK_EQUAL(loaded.display_name, "Longbow of Pure Magic");
	BOOST_CHECK_EQUAL(loaded.icon_name, "bow");
	BOOST_CHECK(loaded.slot == rpg::EquipmentSlot::Weapon);
	BOOST_CHECK(loaded.two_handed);
	BOOST_CHECK(loaded.use_sound.empty());
	BOOST_CHECK_EQUAL(loaded.bullet.name, "");
	BOOST_CHECK_EQUAL(loaded.effect.name, "");
	BOOST_CHECK_CLOSE(loaded.effect.ratio, 0.f, 0.0001f);
	BOOST_CHECK_EQUAL(loaded.worth, 123u);
	BOOST_CHECK_CLOSE(loaded.damage[rpg::DamageType::Bullet], 1.2f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.boni.defense[rpg::DamageType::Fire], 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.require[rpg::Attribute::Dexterity], 2., 0.0001f);
	BOOST_CHECK_EQUAL(loaded.boni.properties[rpg::Property::RangeBase], 12u);
	BOOST_CHECK_EQUAL(loaded.recover[rpg::Stat::Stamina], 5u);
	BOOST_CHECK(loaded.revive);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_perk_iterates_all_data) {
	// prepare perk
	rpg::PerkTemplate perk;
	perk.type = rpg::PerkType::Enemy;
	perk.display_name = "Fireball";
	perk.icon_name = "blast";
	perk.use_sound = "fireball";
	perk.revive = true;
	perk.bullet.name = "fireball";
	perk.effect.name = "burn";
	perk.effect.ratio = 0.25f;
	perk.damage[rpg::DamageType::Fire] = 2.5f;
	perk.recover[rpg::Stat::Mana] = -0.5f;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(perk.saveToTree(ptree));

	// load (twice!)
	rpg::PerkTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK(loaded.type == rpg::PerkType::Enemy);
	BOOST_CHECK_EQUAL(loaded.display_name, "Fireball");
	BOOST_CHECK_EQUAL(loaded.icon_name, "blast");
	BOOST_CHECK_EQUAL(loaded.use_sound, "fireball");
	BOOST_CHECK(loaded.revive);
	BOOST_CHECK_EQUAL(loaded.bullet.name, "fireball");
	BOOST_CHECK_EQUAL(loaded.effect.name, "burn");
	BOOST_CHECK_CLOSE(loaded.effect.ratio, 0.25f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.damage[rpg::DamageType::Fire], 2.5f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.recover[rpg::Stat::Mana], -0.5f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	saving_and_loading_perk_without_effect_projectile_and_sound_is_possible) {
	// prepare perk
	rpg::PerkTemplate perk;
	perk.type = rpg::PerkType::Enemy;
	perk.display_name = "Fireball";
	perk.icon_name = "blast";
	perk.revive = true;
	perk.damage[rpg::DamageType::Fire] = 2.5f;
	perk.recover[rpg::Stat::Mana] = -0.5f;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(perk.saveToTree(ptree));

	// load (twice!)
	rpg::PerkTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK(loaded.type == rpg::PerkType::Enemy);
	BOOST_CHECK_EQUAL(loaded.display_name, "Fireball");
	BOOST_CHECK_EQUAL(loaded.icon_name, "blast");
	BOOST_CHECK(loaded.use_sound.empty());
	BOOST_CHECK(loaded.revive);
	BOOST_CHECK(loaded.bullet.name.empty());
	BOOST_CHECK(loaded.effect.name.empty());
	BOOST_CHECK_CLOSE(loaded.effect.ratio, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.damage[rpg::DamageType::Fire], 2.5f, 0.0001f);
	BOOST_CHECK_CLOSE(loaded.recover[rpg::Stat::Mana], -0.5f, 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_trap_iterates_all_data) {
	// prepare perk
	rpg::TrapTemplate trap;
	trap.trigger_sound = "explode";
	trap.bullet.name = "fireball";
	trap.effect.name = "burn";
	trap.effect.ratio = 0.66f;
	trap.damage[rpg::DamageType::Fire] = 25u;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(trap.saveToTree(ptree));

	// load (twice!)
	rpg::TrapTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(loaded.trigger_sound, "explode");
	BOOST_CHECK_EQUAL(loaded.bullet.name, "fireball");
	BOOST_CHECK_EQUAL(loaded.effect.name, "burn");
	BOOST_CHECK_CLOSE(loaded.effect.ratio, 0.66f, 0.0001f);
	BOOST_CHECK_EQUAL(loaded.damage[rpg::DamageType::Fire], 25u);
}

BOOST_AUTO_TEST_CASE(saving_and_loading_trap_without_effect_is_possible) {
	// prepare perk
	rpg::TrapTemplate trap;
	trap.bullet.name = "fireball";
	trap.damage[rpg::DamageType::Fire] = 25u;

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(trap.saveToTree(ptree));

	// load (twice!)
	rpg::TrapTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK(loaded.trigger_sound.empty());
	BOOST_CHECK_EQUAL(loaded.bullet.name, "fireball");
	BOOST_CHECK(loaded.effect.name.empty());
	BOOST_CHECK_EQUAL(loaded.damage[rpg::DamageType::Fire], 25u);
}

BOOST_AUTO_TEST_SUITE_END()
