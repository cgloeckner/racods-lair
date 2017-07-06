#pragma once
#include <SFML/Window/VideoMode.hpp>

#include <rpg/resources.hpp>
#include <game/resources.hpp>

namespace state {

ENUM(Difficulty, Easy, (Easy)(Normal)(Difficult)(Hard))

} // ::state

ENUM_STREAM(state::Difficulty)

SET_ENUM_LIMITS(state::Difficulty::Easy, state::Difficulty::Hard)

namespace state {

struct FontSettings
	: rpg::BaseResource {
	std::string font;
	unsigned int char_size;
	
	FontSettings();
	
	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

// --------------------------------------------------------------------

struct GlobalSettings
	: rpg::BaseResource {
	std::vector<std::string> logo;
	FontSettings widget, title, combat, notification;
	std::string sfx_volume_preview, title_theme, menu_background, levelup_sfx,
		powerup_sfx;
	sf::Time sfx_threshold;
	utils::EnumMap<rpg::FeedbackType, std::string> feedback;
	sf::Color ui_color, ui_highlight, ui_warning;
	std::string ui_menu_sfx_activate, ui_menu_sfx_deactivate,
		ui_menu_sfx_alternate, ui_menu_sfx_navigate, ui_menu_sfx_type,
		ui_menu_sfx_undo;
	std::vector<sf::Color> player_colors;
	std::size_t max_num_objects;
	unsigned int max_num_players, framelimit,
		audio_poolsize, ui_widget_width, max_input_len;
	float horizontal_padding, vertical_padding, hud_padding, hud_margin, zoom;
	rpg::Keybinding default_keyboard, default_gamepad;
	game::GeneratorSettings dungeon_gen;
	sf::Vector2u dungeon_size;
	std::size_t min_num_dungeons, max_num_dungeons;
	utils::EnumMap<Difficulty, float> difficulty;
	
	GlobalSettings();
	
	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
	
	std::string getFilename() const;
};

// --------------------------------------------------------------------

struct Settings
	: rpg::BaseResource {
	sf::VideoMode resolution;
	bool fullscreen, autocam, autosave;
	std::size_t lighting;
	Difficulty difficulty;
	float sound, music;
	
	Settings();
	
	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
	
	std::string getFilename() const;
};

bool operator==(Settings const & lhs, Settings const & rhs);
bool operator!=(Settings const & lhs, Settings const & rhs);

} // ::state
