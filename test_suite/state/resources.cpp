#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

#include <state/resources.hpp>

BOOST_AUTO_TEST_SUITE(state_resources_test)

BOOST_AUTO_TEST_CASE(saving_and_loading_font_settings_iterates_all_data) {
	state::FontSettings settings;
	settings.font = "foo";
	settings.char_size = 12u;
	
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(settings.saveToTree(ptree));
	
	// load (twice!)
	state::FontSettings loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(settings.font, loaded.font);
	BOOST_CHECK_EQUAL(settings.char_size, loaded.char_size);
}

BOOST_AUTO_TEST_CASE(saving_and_loading_global_settings_iterates_all_data) {
	// prepare settings
	state::GlobalSettings settings;
	settings.logo.emplace_back("sfml-logo");
	settings.logo.emplace_back("boost-logo");
	settings.widget.font = "common";
	settings.title.font = "common2";
	settings.combat.font = "common3";
	settings.notification.font = "common4";
	settings.sfx_volume_preview = "sfx";
	settings.title_theme = "music";
	settings.menu_background = "test";
	settings.levelup_sfx = "levelup";
	settings.powerup_sfx = "powerup";
	settings.sfx_threshold = sf::milliseconds(400);
	settings.feedback[rpg::FeedbackType::NotEnoughMana] = "no-mana";
	settings.ui_color = sf::Color::Cyan;
	settings.ui_highlight = sf::Color::Magenta;
	settings.ui_warning = sf::Color::White;
	settings.ui_menu_sfx_activate = "click";
	settings.ui_menu_sfx_deactivate = "click2";
	settings.ui_menu_sfx_alternate = "alter";
	settings.ui_menu_sfx_navigate = "switch";
	settings.ui_menu_sfx_type = "tyyype";
	settings.ui_menu_sfx_undo = "undooo";
	settings.player_colors = {sf::Color::Yellow, sf::Color::Black};
	settings.max_num_objects = 100u;
	settings.max_num_players = settings.player_colors.size();
	settings.framelimit = 200u;
	settings.audio_poolsize = 64u;
	settings.ui_widget_width = 3u;
	settings.max_input_len = 100u;
	settings.horizontal_padding = 20.f;
	settings.vertical_padding = 20.f;
	settings.hud_padding = 15.f;
	settings.hud_margin = 150.f;
	settings.zoom = 1.5f;
	settings.default_keyboard.map.set(rpg::PlayerAction::MoveN, {sf::Keyboard::Up});
	settings.default_gamepad.map.set(rpg::PlayerAction::MoveN, {0u, 2u});
	settings.dungeon_gen.cell_size = 31u;
	settings.dungeon_size = {200u, 300u};
	settings.min_num_dungeons = 5u;
	settings.max_num_dungeons = 15u;
	settings.difficulty[state::Difficulty::Easy] = 0.2f;
	settings.difficulty[state::Difficulty::Normal] = 0.4f;
	settings.difficulty[state::Difficulty::Difficult] = 0.6f;
	settings.difficulty[state::Difficulty::Hard] = 0.8f;
	
	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(settings.saveToTree(ptree));

	// load (twice!)
	state::GlobalSettings loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK(settings.logo == loaded.logo);
	BOOST_CHECK_EQUAL(settings.widget.font, loaded.widget.font);
	BOOST_CHECK_EQUAL(settings.title.font, loaded.title.font);
	BOOST_CHECK_EQUAL(settings.combat.font, loaded.combat.font);
	BOOST_CHECK_EQUAL(settings.notification.font, loaded.notification.font);
	BOOST_CHECK_EQUAL(settings.sfx_volume_preview, loaded.sfx_volume_preview);
	BOOST_CHECK_EQUAL(settings.title_theme, loaded.title_theme);
	BOOST_CHECK_EQUAL(settings.menu_background, loaded.menu_background);
	BOOST_CHECK_EQUAL(settings.levelup_sfx, loaded.levelup_sfx);
	BOOST_CHECK_EQUAL(settings.powerup_sfx, loaded.powerup_sfx);
	BOOST_CHECK_TIME_EQUAL(settings.sfx_threshold, loaded.sfx_threshold);
	BOOST_CHECK(settings.feedback == loaded.feedback);
	BOOST_CHECK_COLOR_EQUAL(settings.ui_color, loaded.ui_color);
	BOOST_CHECK_COLOR_EQUAL(settings.ui_highlight, loaded.ui_highlight);
	BOOST_CHECK_COLOR_EQUAL(settings.ui_warning, loaded.ui_warning);
	BOOST_CHECK_EQUAL(settings.ui_menu_sfx_activate, loaded.ui_menu_sfx_activate);
	BOOST_CHECK_EQUAL(settings.ui_menu_sfx_deactivate, loaded.ui_menu_sfx_deactivate);
	BOOST_CHECK_EQUAL(settings.ui_menu_sfx_alternate, loaded.ui_menu_sfx_alternate);
	BOOST_CHECK_EQUAL(settings.ui_menu_sfx_navigate, loaded.ui_menu_sfx_navigate);
	BOOST_CHECK_EQUAL(settings.ui_menu_sfx_type, loaded.ui_menu_sfx_type);
	BOOST_CHECK_EQUAL(settings.ui_menu_sfx_undo, loaded.ui_menu_sfx_undo);
	BOOST_CHECK(settings.player_colors == loaded.player_colors);
	BOOST_CHECK_EQUAL(settings.max_num_objects, loaded.max_num_objects);
	BOOST_CHECK_EQUAL(settings.max_num_players, loaded.max_num_players);
	BOOST_CHECK_EQUAL(settings.framelimit, loaded.framelimit);
	BOOST_CHECK_EQUAL(settings.audio_poolsize, loaded.audio_poolsize);
	BOOST_CHECK_EQUAL(settings.ui_widget_width, loaded.ui_widget_width);
	BOOST_CHECK_EQUAL(settings.max_input_len, loaded.max_input_len);
	BOOST_CHECK_CLOSE(settings.horizontal_padding, loaded.horizontal_padding, 0.0001f);
	BOOST_CHECK_CLOSE(settings.vertical_padding, loaded.vertical_padding, 0.0001f);
	BOOST_CHECK_CLOSE(settings.hud_padding, loaded.hud_padding, 0.0001f);
	BOOST_CHECK_CLOSE(settings.hud_margin, loaded.hud_margin, 0.0001f);
	BOOST_CHECK_CLOSE(settings.zoom, loaded.zoom, 0.0001f);
	BOOST_CHECK(settings.default_keyboard.map.get(rpg::PlayerAction::MoveN)
		== loaded.default_keyboard.map.get(rpg::PlayerAction::MoveN));
	BOOST_CHECK(settings.default_gamepad.map.get(rpg::PlayerAction::MoveN)
		== loaded.default_gamepad.map.get(rpg::PlayerAction::MoveN));
	BOOST_CHECK_EQUAL(settings.dungeon_gen.cell_size,
		loaded.dungeon_gen.cell_size);
	BOOST_CHECK_VECTOR_EQUAL(settings.dungeon_size, loaded.dungeon_size);
	BOOST_CHECK_EQUAL(settings.min_num_dungeons, loaded.min_num_dungeons);
	BOOST_CHECK_EQUAL(settings.max_num_dungeons, loaded.max_num_dungeons);
	BOOST_CHECK(settings.difficulty == loaded.difficulty);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(saving_and_loading_settings_iterates_all_data) {
	// prepare settings
	state::Settings settings;
	settings.resolution.width = 1440u;
	settings.resolution.height = 900u;
	settings.resolution.bitsPerPixel = 64u;
	settings.fullscreen = true;
	settings.autocam = false;
	settings.autosave = false;
	settings.lighting = 100u;
	settings.difficulty = state::Difficulty::Hard;
	settings.sound = 30.f;
	settings.music = 70.f;
	
	// save
	utils::ptree_type ptree;
	BOOST_REQUIRE_NO_THROW(settings.saveToTree(ptree));

	// load (twice!)
	state::Settings loaded;
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));
	BOOST_REQUIRE_NO_THROW(loaded.loadFromTree(ptree));

	// check some data
	BOOST_CHECK_EQUAL(settings.resolution.width, loaded.resolution.width);
	BOOST_CHECK_EQUAL(settings.resolution.height, loaded.resolution.height);
	BOOST_CHECK_EQUAL(settings.resolution.bitsPerPixel, loaded.resolution.bitsPerPixel);
	BOOST_CHECK_EQUAL(settings.fullscreen, loaded.fullscreen);
	BOOST_CHECK_EQUAL(settings.autocam, loaded.autocam);
	BOOST_CHECK_EQUAL(settings.autosave, loaded.autosave);
	BOOST_CHECK_EQUAL(settings.lighting, loaded.lighting);
	BOOST_CHECK(settings.difficulty == loaded.difficulty);
	BOOST_CHECK_CLOSE(settings.sound, loaded.sound, 0.0001f);
	BOOST_CHECK_CLOSE(settings.music, loaded.music, 0.0001f);
}

BOOST_AUTO_TEST_SUITE_END()
