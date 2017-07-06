#include <cctype>
#include <boost/algorithm/string/case_conv.hpp>
#include <state/resources.hpp>

namespace state {

FontSettings::FontSettings()
	: font{""}
	, char_size{10u} {
}

void FontSettings::loadFromTree(utils::ptree_type const& ptree) {
	font = ptree.get<std::string>("<xmlattr>.font");
	char_size = ptree.get<unsigned int>("<xmlattr>.char_size");
}

void FontSettings::saveToTree(utils::ptree_type& ptree) const {
	ptree.put("<xmlattr>.font", font);
	ptree.put("<xmlattr>.char_size", char_size);
}

// --------------------------------------------------------------------

GlobalSettings::GlobalSettings()
	: rpg::BaseResource{}
	, widget{}
	, title{}
	, combat{}
	, notification{}
	, sfx_volume_preview{}
	, title_theme{}
	, menu_background{}
	, levelup_sfx{}
	, powerup_sfx{}
	, sfx_threshold{}
	, feedback{}
	, ui_color{sf::Color::White}
	, ui_highlight{sf::Color::Yellow}
	, ui_warning{sf::Color::Red}
	, ui_menu_sfx_activate{}
	, ui_menu_sfx_deactivate{}
	, ui_menu_sfx_alternate{}
	, ui_menu_sfx_navigate{}
	, ui_menu_sfx_type{}
	, ui_menu_sfx_undo{}
	, player_colors{sf::Color::Blue, sf::Color::Red, sf::Color::Green,
		sf::Color::Yellow, sf::Color::Cyan, sf::Color::Magenta}
	, max_num_objects{30000u}
	, max_num_players{6u}
	, framelimit{60u}
	, audio_poolsize{16u}
	, ui_widget_width{200u}
	, max_input_len{20u}
	, horizontal_padding{200.f}
	, vertical_padding{30.f}
	, hud_padding{10.f}
	, hud_margin{100.f}
	, zoom{1.f}
	, default_keyboard{}
	, default_gamepad{}
	, dungeon_gen{}
	, dungeon_size{}
	, min_num_dungeons{3}
	, max_num_dungeons{12}
	, difficulty{1.f} {
}

void GlobalSettings::loadFromTree(utils::ptree_type const& ptree) {
	utils::parse_vector(ptree, "logo", "img", logo,
		[](utils::ptree_type const & child, std::string& s) {
			s = child.get<std::string>("<xmlattr>.src");
	});
	utils::parse_vector(ptree, "players", "color", player_colors,
		[](utils::ptree_type const & child, sf::Color& elem) {
			elem.r = child.get<sf::Uint8>("<xmlattr>.r");
			elem.g = child.get<sf::Uint8>("<xmlattr>.g");
			elem.b = child.get<sf::Uint8>("<xmlattr>.b");
			elem.a = 255u;
	});
	max_num_objects = ptree.get<std::size_t>("engine.<xmlattr>.max_num_objects");
	max_num_players = player_colors.size();
	widget.loadFromTree(ptree.get_child("ui.widget"));
	title.loadFromTree(ptree.get_child("ui.title"));
	combat.loadFromTree(ptree.get_child("ui.combat"));
	notification.loadFromTree(ptree.get_child("ui.notification"));
	sfx_volume_preview = ptree.get<std::string>("audio.<xmlattr>.sfx_preview");
	title_theme = ptree.get<std::string>("audio.<xmlattr>.title_theme");
	menu_background = ptree.get<std::string>("ui.<xmlattr>.menu_background");
	levelup_sfx = ptree.get<std::string>("audio.<xmlattr>.levelup");
	powerup_sfx = ptree.get<std::string>("audio.<xmlattr>.powerup");
	sfx_threshold = sf::milliseconds(ptree.get<unsigned int>("audio.<xmlattr>.sfx_threshold"));
	rpg::parse(ptree, feedback, "audio.feedback", std::string{});
	ui_color.r = ptree.get<sf::Uint8>("ui.color.<xmlattr>.r");
	ui_color.g = ptree.get<sf::Uint8>("ui.color.<xmlattr>.g");
	ui_color.b = ptree.get<sf::Uint8>("ui.color.<xmlattr>.b");
	ui_color.a = 255u;
	ui_highlight.r = ptree.get<sf::Uint8>("ui.highlight.<xmlattr>.r");
	ui_highlight.g = ptree.get<sf::Uint8>("ui.highlight.<xmlattr>.g");
	ui_highlight.b = ptree.get<sf::Uint8>("ui.highlight.<xmlattr>.b");
	ui_highlight.a = 255u;
	ui_warning.r = ptree.get<sf::Uint8>("ui.warning.<xmlattr>.r");
	ui_warning.g = ptree.get<sf::Uint8>("ui.warning.<xmlattr>.g");
	ui_warning.b = ptree.get<sf::Uint8>("ui.warning.<xmlattr>.b");
	ui_menu_sfx_activate = ptree.get<std::string>("ui.menu_sfx.<xmlattr>.activate");
	ui_menu_sfx_deactivate = ptree.get<std::string>("ui.menu_sfx.<xmlattr>.deactivate");
	ui_menu_sfx_alternate = ptree.get<std::string>("ui.menu_sfx.<xmlattr>.alternate");
	ui_menu_sfx_navigate = ptree.get<std::string>("ui.menu_sfx.<xmlattr>.navigate");
	ui_menu_sfx_type = ptree.get<std::string>("ui.menu_sfx.<xmlattr>.type");
	ui_menu_sfx_undo = ptree.get<std::string>("ui.menu_sfx.<xmlattr>.undo");
	ui_warning.a = 255u;
	vertical_padding = ptree.get<float>("ui.<xmlattr>.vertical_padding");
	horizontal_padding = ptree.get<float>("ui.<xmlattr>.horizontal_padding");
	hud_padding = ptree.get<float>("ui.<xmlattr>.hud_padding");
	hud_margin = ptree.get<float>("ui.<xmlattr>.hud_margin");
	ui_widget_width = ptree.get<unsigned int>("ui.<xmlattr>.widget_width");
	max_input_len = ptree.get<unsigned int>("ui.<xmlattr>.max_input_len");
	framelimit = ptree.get<unsigned int>("video.<xmlattr>.framelimit");
	zoom = ptree.get<float>("video.<xmlattr>.zoom");
	audio_poolsize = ptree.get<unsigned int>("audio.<xmlattr>.poolsize");
	default_keyboard.loadFromTree(ptree.get_child("input.keyboard"));
	default_gamepad.loadFromTree(ptree.get_child("input.gamepad"));
	dungeon_gen.loadFromTree(ptree.get_child("dungeon_generator"));
	dungeon_size.x = ptree.get<unsigned int>("dungeon_generator.size.<xmlattr>.width");
	dungeon_size.y = ptree.get<unsigned int>("dungeon_generator.size.<xmlattr>.height");
	min_num_dungeons = ptree.get<unsigned int>("dungeon_generator.<xmlattr>.min_num_dungeons");
	max_num_dungeons = ptree.get<unsigned int>("dungeon_generator.<xmlattr>.max_num_dungeons");
	rpg::parse(ptree, difficulty, "difficulty");
}

void GlobalSettings::saveToTree(utils::ptree_type& ptree) const {
	utils::dump_vector(ptree, "logo", "img", logo,
		[](utils::ptree_type& child, std::string const & s) {
			child.put("<xmlattr>.src", s);
	});
	ASSERT(max_num_players == player_colors.size());
	utils::dump_vector(ptree, "players", "color", player_colors,
		[](utils::ptree_type& child, sf::Color const & elem) {
			child.put("<xmlattr>.r", elem.r);
			child.put("<xmlattr>.g", elem.g);
			child.put("<xmlattr>.b", elem.b);
	});
	ptree.put("engine.<xmlattr>.max_num_objects", max_num_objects);
	utils::ptree_type w, t, c, n;
	widget.saveToTree(w);
	ptree.add_child("ui.widget", w);
	title.saveToTree(t);
	ptree.add_child("ui.title", t);
	combat.saveToTree(c);
	ptree.add_child("ui.combat", c);
	notification.saveToTree(n);
	ptree.add_child("ui.notification", n);
	ptree.put("audio.<xmlattr>.sfx_preview", sfx_volume_preview);
	ptree.put("audio.<xmlattr>.title_theme", title_theme);
	ptree.put("ui.<xmlattr>.menu_background", menu_background);
	ptree.put("audio.<xmlattr>.levelup", levelup_sfx);
	ptree.put("audio.<xmlattr>.powerup", powerup_sfx);
	ptree.put("audio.<xmlattr>.sfx_threshold", sfx_threshold.asMilliseconds());
	rpg::dump(ptree, feedback, "audio.feedback", std::string{});
	ptree.put("ui.color.<xmlattr>.r", ui_color.r);
	ptree.put("ui.color.<xmlattr>.g", ui_color.g);
	ptree.put("ui.color.<xmlattr>.b", ui_color.b);
	ptree.put("ui.highlight.<xmlattr>.r", ui_highlight.r);
	ptree.put("ui.highlight.<xmlattr>.g", ui_highlight.g);
	ptree.put("ui.highlight.<xmlattr>.b", ui_highlight.b);
	ptree.put("ui.color.<xmlattr>.g", ui_color.b);
	ptree.put("ui.warning.<xmlattr>.r", ui_warning.r);
	ptree.put("ui.warning.<xmlattr>.g", ui_warning.g);
	ptree.put("ui.warning.<xmlattr>.b", ui_warning.b);
	ptree.put("ui.<xmlattr>.horizontal_padding", horizontal_padding);
	ptree.put("ui.<xmlattr>.vertical_padding", vertical_padding);
	ptree.put("ui.<xmlattr>.hud_padding", hud_padding);
	ptree.put("ui.<xmlattr>.hud_margin", hud_margin);
	ptree.put("ui.<xmlattr>.widget_width", ui_widget_width);
	ptree.put("ui.<xmlattr>.max_input_len", max_input_len);
	ptree.put("ui.menu_sfx.<xmlattr>.activate", ui_menu_sfx_activate);
	ptree.put("ui.menu_sfx.<xmlattr>.deactivate", ui_menu_sfx_deactivate);
	ptree.put("ui.menu_sfx.<xmlattr>.alternate", ui_menu_sfx_alternate);
	ptree.put("ui.menu_sfx.<xmlattr>.navigate", ui_menu_sfx_navigate);
	ptree.put("ui.menu_sfx.<xmlattr>.type", ui_menu_sfx_type);
	ptree.put("ui.menu_sfx.<xmlattr>.undo", ui_menu_sfx_undo);
	ptree.put("video.<xmlattr>.framelimit", framelimit);
	ptree.put("video.<xmlattr>.zoom", zoom);
	ptree.put("audio.<xmlattr>.poolsize", audio_poolsize);
	utils::ptree_type keyboard, gamepad;
	default_keyboard.saveToTree(keyboard);
	default_gamepad.saveToTree(gamepad);
	ptree.add_child("input.keyboard", keyboard);
	ptree.add_child("input.gamepad", gamepad);
	utils::ptree_type gen;
	dungeon_gen.saveToTree(gen);
	ptree.add_child("dungeon_generator", gen);
	ptree.put("dungeon_generator.size.<xmlattr>.width", dungeon_size.x);
	ptree.put("dungeon_generator.size.<xmlattr>.height", dungeon_size.y);
	ptree.put("dungeon_generator.<xmlattr>.min_num_dungeons", min_num_dungeons);
	ptree.put("dungeon_generator.<xmlattr>.max_num_dungeons", max_num_dungeons);
	
	rpg::dump(ptree, difficulty, "difficulty");
}

std::string GlobalSettings::getFilename() const {
	return "globals.xml";
}

// --------------------------------------------------------------------

Settings::Settings()
	: rpg::BaseResource{}
	, resolution{800u, 600u}
	, fullscreen{false}
	, autocam{false}
	, autosave{true}
	, lighting{2u}
	, difficulty{Difficulty::Easy}
	, sound{100.f}
	, music{50.f} {
}

void Settings::loadFromTree(utils::ptree_type const& ptree) {
	resolution.width = ptree.get<unsigned int>("video.<xmlattr>.width");
	resolution.height = ptree.get<unsigned int>("video.<xmlattr>.height");
	resolution.bitsPerPixel = ptree.get<unsigned int>("video.<xmlattr>.depth");
	fullscreen = ptree.get<bool>("video.<xmlattr>.fullscreen");
	autocam = ptree.get<bool>("game.<xmlattr>.autocam");
	autosave = ptree.get<bool>("game.<xmlattr>.autosave");
	lighting = ptree.get<std::size_t>("game.<xmlattr>.lighting");
	auto diffic = ptree.get<std::string>("game.<xmlattr>.difficulty");
	diffic[0] = std::toupper(diffic[0]);
	difficulty = from_string<Difficulty>(diffic);
	sound = ptree.get<float>("volume.<xmlattr>.sound");
	music = ptree.get<float>("volume.<xmlattr>.music");
	
	// fix bounds to [0.f, 100.f]
	sound = std::min(std::max(sound, 0.f), 100.f);
	music = std::min(std::max(music, 0.f), 100.f);
}

void Settings::saveToTree(utils::ptree_type& ptree) const {
	ptree.put("video.<xmlattr>.width", resolution.width);
	ptree.put("video.<xmlattr>.height", resolution.height);
	ptree.put("video.<xmlattr>.depth", resolution.bitsPerPixel);
	ptree.put("video.<xmlattr>.fullscreen", fullscreen);
	ptree.put("game.<xmlattr>.autocam", autocam);
	ptree.put("game.<xmlattr>.autosave", autosave);
	ptree.put("game.<xmlattr>.lighting", lighting);
	auto diffic = to_string(difficulty);
	boost::algorithm::to_lower(diffic);
	ptree.put("game.<xmlattr>.difficulty", diffic);
	ptree.put("volume.<xmlattr>.sound", sound);
	ptree.put("volume.<xmlattr>.music", music);
}

bool operator==(Settings const & lhs, Settings const & rhs) {
	// note: difficulty is ignored here
	return lhs.resolution == rhs.resolution &&
		lhs.fullscreen == rhs.fullscreen &&
		lhs.lighting == rhs.lighting &&
		lhs.autocam == rhs.autocam &&
		lhs.autosave == rhs.autosave &&
		lhs.sound == rhs.sound &&
		lhs.music == rhs.music;
}

bool operator!=(Settings const & lhs, Settings const & rhs) {
	// note: difficulty is ignored here
	return lhs.resolution != rhs.resolution ||
		lhs.fullscreen != rhs.fullscreen ||
		lhs.lighting != rhs.lighting ||
		lhs.autocam != rhs.autocam ||
		lhs.autosave != rhs.autosave ||
		lhs.sound != rhs.sound ||
		lhs.music != rhs.music;
}

std::string Settings::getFilename() const {
	return "settings.xml";
}


} // ::state
