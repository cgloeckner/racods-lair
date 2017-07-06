#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

#include <boost/property_tree/xml_parser.hpp>

#include <game/resources.hpp>

BOOST_AUTO_TEST_SUITE(resources_test)

BOOST_AUTO_TEST_CASE(saving_and_loading_generator_settings_iterates_all_data) {
	// prepare settings
	game::GeneratorSettings settings;
	settings.cell_size = 31u;
	settings.room_density = 0.7f;
	settings.deadend_density = 0.1f;
	settings.ambience_density = 0.6f;
	settings.redundant_paths_ratio = 0.3f;
	
	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(settings.saveToTree(ptree));

	// load (twice!)
	game::GeneratorSettings loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(settings.cell_size, loaded.cell_size);
	BOOST_CHECK_CLOSE(settings.room_density, loaded.room_density, 0.0001f);
	BOOST_CHECK_CLOSE(settings.deadend_density, loaded.deadend_density, 0.0001f);
	BOOST_CHECK_CLOSE(settings.ambience_density, loaded.ambience_density, 0.0001f);
	BOOST_CHECK_CLOSE(settings.redundant_paths_ratio, loaded.redundant_paths_ratio, 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_bot_iterates_all_data) {
	// prepare bot
	game::BotTemplate bot;
	bot.display_name = "Goblin Warrior";
	bot.entity_name = "goblin-warrior";
	bot.color = sf::Color::Red;
	bot.attributes[rpg::Attribute::Strength] = 45.f;
	bot.defense[rpg::DamageType::Blade] = 0.2f;
	bot.properties[rpg::Property::MeleeBase] = 0.3f;

	bot.items.emplace_back("potion", 5.f, nullptr);
	bot.items.emplace_back("longbow", 1.f, nullptr);
	bot.perks.emplace_back("fireball", 3.f, nullptr);

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(bot.saveToTree(ptree));

	// load (twice!)
	game::BotTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(loaded.display_name, bot.display_name);
	BOOST_CHECK_EQUAL(loaded.entity_name, bot.entity_name);
	BOOST_CHECK_COLOR_EQUAL(loaded.color, bot.color);
	BOOST_CHECK_CLOSE(loaded.attributes[rpg::Attribute::Strength], 45.f, 0.0001f);
	BOOST_CHECK(loaded.defense == bot.defense);
	BOOST_CHECK(loaded.properties == bot.properties);
	BOOST_REQUIRE_EQUAL(loaded.items.size(), 2u);
	BOOST_CHECK_EQUAL(std::get<0>(loaded.items[1]), "longbow");
	BOOST_CHECK_CLOSE(std::get<1>(loaded.items[1]), 1.f, 0.0001f);
	BOOST_REQUIRE_EQUAL(loaded.perks.size(), 1u);
	BOOST_CHECK_EQUAL(std::get<0>(loaded.perks[0]), "fireball");
	BOOST_CHECK_CLOSE(std::get<1>(loaded.perks[0]), 3.f, 0.0001f);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_encounter_iterates_all_data) {
	// prepare bot
	game::EncounterTemplate encounter;
	encounter.bots.emplace_back("goblin-warrior", 0.7f);
	encounter.bots.emplace_back("goblin-archer", 0.2f);
	encounter.bots.emplace_back("goblin-warlock", 0.1f);

	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(encounter.saveToTree(ptree));

	// load (twice!)
	game::EncounterTemplate loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_REQUIRE_EQUAL(loaded.bots.size(), encounter.bots.size());
	auto const & lhs = loaded.bots[1];
	auto const & rhs = encounter.bots[1];
	BOOST_CHECK_EQUAL(lhs.filename, rhs.filename);
	BOOST_CHECK_CLOSE(lhs.ratio, rhs.ratio, 0.0001f);
	BOOST_CHECK(lhs.ptr == nullptr);
	BOOST_CHECK(rhs.ptr == nullptr);
}

BOOST_AUTO_TEST_CASE(pick_bot_from_encounter_list_is_deterministic) {
	// prepare bot
	game::EncounterTemplate encounter;
	game::BotTemplate a, b, c;
	encounter.bots.emplace_back("goblin-warlock", 0.1f, &a);
	encounter.bots.emplace_back("goblin-archer", 0.2f, &b);
	encounter.bots.emplace_back("goblin-warrior", 0.7f, &c);
	
	BOOST_CHECK_EQUAL(&encounter.pick(0.05f), &a);
	BOOST_CHECK_EQUAL(&encounter.pick(0.15f), &b);
	BOOST_CHECK_EQUAL(&encounter.pick(0.25f), &b);
	BOOST_CHECK_EQUAL(&encounter.pick(0.35f), &c);
	BOOST_CHECK_EQUAL(&encounter.pick(0.5f), &c);
	BOOST_CHECK_EQUAL(&encounter.pick(0.75f), &c);
	BOOST_CHECK_EQUAL(&encounter.pick(0.99f), &c);
	
	BOOST_CHECK_ASSERT(encounter.pick(-1.f));
	BOOST_CHECK_ASSERT(encounter.pick(1.1f));
}

// ---------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_room_iterates_all_data) {
	game::RoomTemplate room;
	game::RoomTemplate::RoomCell a, b, c;
	a.entity.name = "foo";
	a.entity.direction.x = -1;
	b.entity.name = "bar";
	b.entity.direction.y = 0;
	c.wall = true;
	room.cells[{2u, 3u}] = a;
	room.cells[{3u, 1u}] = a;
	room.cells[{1u, 4u}] = a;
	
	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(room.saveToTree(ptree));

	// load
	game::RoomTemplate loaded;
	loaded.cells[{0u, 0u}] = decltype(a){}; // will be dropped while loading
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check all data
	BOOST_CHECK(loaded == room);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(empty_is_valid) {
	game::RoomTemplate room;
	utils::Logger log;
	BOOST_CHECK(room.isValid(log, 5u));
}

BOOST_AUTO_TEST_CASE(room_with_outside_cell_is_invalid) {
	game::RoomTemplate room;
	room.create({6u, 7u});
	utils::Logger log;
	BOOST_CHECK(!room.isValid(log, 5u));
}

BOOST_AUTO_TEST_CASE(room_with_top_border_floor_is_invalid) {
	game::RoomTemplate room;
	room.create({2u, 0u});
	utils::Logger log;
	BOOST_CHECK(!room.isValid(log, 5u));
}

BOOST_AUTO_TEST_CASE(room_with_bottom_border_floor_is_invalid) {
	game::RoomTemplate room;
	room.create({2u, 4u});
	utils::Logger log;
	BOOST_CHECK(!room.isValid(log, 5u));
}

BOOST_AUTO_TEST_CASE(room_with_left_border_floor_is_invalid) {
	game::RoomTemplate room;
	room.create({0u, 3u});
	utils::Logger log;
	BOOST_CHECK(!room.isValid(log, 5u));
}

BOOST_AUTO_TEST_CASE(room_with_right_border_floor_is_invalid) {
	game::RoomTemplate room;
	room.create({4u, 3u});
	utils::Logger log;
	BOOST_CHECK(!room.isValid(log, 5u));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_player_iterates_all_data) {
	game::PlayerTemplate player;
	player.display_name = "Foo bar";
	player.entity_name = "human-base";
	player.inventory.resize(2u);
	std::get<0>(player.inventory[0]) = "sword";
	std::get<1>(player.inventory[0]) = 1;
	std::get<0>(player.inventory[1]) = "potion";
	std::get<1>(player.inventory[1]) = 5;
	player.equipment[rpg::EquipmentSlot::Weapon] = "sword";
	player.perks.resize(1u);
	std::get<0>(player.perks[0]) = "fireball";
	std::get<1>(player.perks[0]) = 3u;
	player.level = 5u;
	player.attributes[rpg::Attribute::Strength] = 25u;
	player.attributes[rpg::Attribute::Dexterity] = 10u;
	player.attributes[rpg::Attribute::Wisdom] = 15u;
	player.slot_id = 1u;
	std::get<1>(player.slots[0u]) = "fireball";
	std::get<0>(player.slots[1u]) = "potion";
	player.exp = 7382ul;
	player.attrib_points = 5u;
	player.perk_points = 2u;

	// save and load
	auto stream = player.saveToPacket();
	sf::Packet packet;
	packet.append(stream.getData(), stream.getDataSize());
	game::PlayerTemplate loaded;
	loaded.loadFromPacket(packet);

	// check some data
	BOOST_CHECK_EQUAL(player.display_name, loaded.display_name);
	BOOST_CHECK_EQUAL(player.entity_name, loaded.entity_name);
	BOOST_REQUIRE_EQUAL(player.inventory.size(), loaded.inventory.size());
	BOOST_CHECK_EQUAL(
		std::get<0>(player.inventory[0]), std::get<0>(loaded.inventory[0]));
	BOOST_CHECK_EQUAL(
		std::get<1>(player.inventory[1]), std::get<1>(loaded.inventory[1]));
	BOOST_CHECK_EQUAL(player.equipment[rpg::EquipmentSlot::Weapon],
		loaded.equipment[rpg::EquipmentSlot::Weapon]);
	BOOST_CHECK_EQUAL(player.equipment[rpg::EquipmentSlot::Body],
		loaded.equipment[rpg::EquipmentSlot::Body]);
	BOOST_REQUIRE_EQUAL(player.perks.size(), loaded.perks.size());
	BOOST_CHECK_EQUAL(
		std::get<0>(player.perks[0]), std::get<0>(loaded.perks[0]));
	BOOST_CHECK_EQUAL(
		std::get<1>(player.perks[0]), std::get<1>(loaded.perks[0]));
	BOOST_CHECK_EQUAL(player.level, loaded.level);
	BOOST_CHECK_EQUAL(player.attributes[rpg::Attribute::Strength],
		loaded.attributes[rpg::Attribute::Strength]);
	BOOST_CHECK_EQUAL(player.slot_id, loaded.slot_id);
	BOOST_CHECK_EQUAL(std::get<0>(player.slots[0]), "");
	BOOST_CHECK_EQUAL(std::get<1>(player.slots[0]), "fireball");
	BOOST_CHECK_EQUAL(std::get<0>(player.slots[1]), "potion");
	BOOST_CHECK_EQUAL(std::get<1>(player.slots[1]), "");
	BOOST_CHECK_EQUAL(player.exp, loaded.exp);
	BOOST_CHECK_EQUAL(player.attrib_points, loaded.attrib_points);
	BOOST_CHECK_EQUAL(player.perk_points, loaded.perk_points);
}

BOOST_AUTO_TEST_CASE(playertemplate_can_be_fetched_from_components) {
	rpg::ItemTemplate sword, armor, potion;
	sword.internal_name = "sword";
	armor.internal_name = "armor";
	potion.internal_name = "potion";
	rpg::PerkTemplate fireball;
	fireball.internal_name = "fireball";
	rpg::ItemData items;
	items.inventory[rpg::ItemType::Weapon].emplace_back(sword, 1u);
	items.inventory[rpg::ItemType::Armor].emplace_back(armor, 1u);
	items.inventory[rpg::ItemType::Misc].emplace_back(potion, 3u);
	items.equipment[rpg::EquipmentSlot::Weapon] = &sword;
	items.equipment[rpg::EquipmentSlot::Body] = &armor;
	rpg::PerkData perks;
	perks.perks.emplace_back(fireball, 4u);
	rpg::StatsData stats;
	stats.level = 12u;
	stats.attributes[rpg::Attribute::Strength] = 30u;
	stats.attributes[rpg::Attribute::Dexterity] = 22u;
	stats.attributes[rpg::Attribute::Wisdom] = 10;
	rpg::QuickslotData qslots;
	qslots.slot_id = 3u;
	qslots.slots[0] = sword;
	qslots.slots[1] = fireball;
	qslots.slots[3] = potion;
	rpg::PlayerData p;
	p.exp = 13337ul;
	p.attrib_points = 7u;
	p.perk_points = 1u;
	
	game::PlayerTemplate player;
	player.fetch(items, perks, stats, qslots, p);
	
	BOOST_REQUIRE_EQUAL(player.inventory.size(), 3u);
	BOOST_CHECK_EQUAL(std::get<0>(player.inventory[0]), "sword");
	BOOST_CHECK_EQUAL(std::get<1>(player.inventory[0]), 1u);
	BOOST_CHECK_EQUAL(std::get<0>(player.inventory[1]), "armor");
	BOOST_CHECK_EQUAL(std::get<1>(player.inventory[1]), 1u);
	BOOST_CHECK_EQUAL(std::get<0>(player.inventory[2]), "potion");
	BOOST_CHECK_EQUAL(std::get<1>(player.inventory[2]), 3u);
	
	BOOST_CHECK_EQUAL(player.equipment[rpg::EquipmentSlot::Weapon], "sword");
	BOOST_CHECK_EQUAL(player.equipment[rpg::EquipmentSlot::Extension], "");
	BOOST_CHECK_EQUAL(player.equipment[rpg::EquipmentSlot::Body], "armor");
	BOOST_CHECK_EQUAL(player.equipment[rpg::EquipmentSlot::Head], "");
	
	BOOST_REQUIRE_EQUAL(player.perks.size(), 1u);
	BOOST_CHECK_EQUAL(std::get<0>(player.perks[0]), "fireball");
	BOOST_CHECK_EQUAL(std::get<1>(player.perks[0]), 4u);
	
	BOOST_CHECK_EQUAL(player.level, 12u);
	BOOST_CHECK_EQUAL(player.attributes[rpg::Attribute::Strength], 30u);
	BOOST_CHECK_EQUAL(player.attributes[rpg::Attribute::Dexterity], 22u);
	BOOST_CHECK_EQUAL(player.attributes[rpg::Attribute::Wisdom], 10u);
	
	BOOST_CHECK_EQUAL(player.slot_id, 3u);
	BOOST_CHECK_EQUAL(std::get<0>(player.slots[0u]), "sword");
	BOOST_CHECK_EQUAL(std::get<1>(player.slots[0u]), "");
	BOOST_CHECK_EQUAL(std::get<0>(player.slots[1u]), "");
	BOOST_CHECK_EQUAL(std::get<1>(player.slots[1u]), "fireball");
	BOOST_CHECK_EQUAL(std::get<0>(player.slots[2u]), "");
	BOOST_CHECK_EQUAL(std::get<1>(player.slots[2u]), "");
	BOOST_CHECK_EQUAL(std::get<0>(player.slots[3u]), "potion");
	BOOST_CHECK_EQUAL(std::get<1>(player.slots[3u]), "");
	
	BOOST_CHECK_EQUAL(player.exp, 13337ul);
	BOOST_CHECK_EQUAL(player.attrib_points, 7u);
	BOOST_CHECK_EQUAL(player.perk_points, 1u);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(localization_uses_fallback_if_key_not_found) {
	game::Localization locale;
	BOOST_CHECK_EQUAL(locale("foo.bar", "fall"), "fall");
	BOOST_CHECK_EQUAL(locale("foobar", "fall"), "fall");
}

BOOST_AUTO_TEST_CASE(localization_returns_string_if_key_was_found) {
	game::Localization locale;
	utils::ptree_type ptree;
	ptree.put("foo.<xmlattr>.bar", "test");
	locale.loadFromTree(ptree);
	
	BOOST_CHECK_EQUAL(locale("foo.bar", "fall"), "test");
}

BOOST_AUTO_TEST_SUITE_END()
