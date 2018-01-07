#include <ui/common.hpp>
#include <state/common.hpp>

namespace state {

unsigned int const MIN_SCREEN_WIDTH = 800u;
unsigned int const MIN_SCREEN_HEIGHT = 600u;

void apply(core::LogContext& log, sf::Window& window,
	state::Settings const & settings, unsigned int framelimit) {
	sf::Uint32 style{sf::Style::Default};
	if (settings.fullscreen) {
		style = sf::Style::Fullscreen;
		// workaround: something is broken with the fullscreen mode
		// style = sf::Style::None;
	}
	auto title = engine::TITLE + " (v" + engine::VERSION + ")";
	window.create(settings.resolution, title, style);
	
	// apply framelimit or vsync
	if (framelimit == 0u) {
		log.debug << "[State/Common] VSync enabled\n";
		window.setVerticalSyncEnabled(true);
	} else {
		log.debug << "[State/Common] Framelimit set to " << framelimit << "\n";
		window.setFramerateLimit(framelimit);
	}
}

// --------------------------------------------------------------------

State::State(App& app)
	: utils::State<Context>{app} {
}

void State::onResize(sf::Vector2u screen_size) {
	auto& context = getContext();
	context.onResize(screen_size);
}

void State::activate() {
	auto screen_size = getApplication().getWindow().getSize();
	// resize ui elements
	sf::Event event;
	event.type = sf::Event::Resized;
	event.size.width = screen_size.x;
	event.size.height = screen_size.y;
	handle(event);
}

// --------------------------------------------------------------------

Context::Context(App& app)
	: app{app}
	, sfx{}
	, theme{}
	, log{}
	, cache{}
	, mod{log, cache, "data"}
	, locale{}
	, globals{}
	, settings{}
	, game{nullptr}
	, background{} {
	theme.setLoop(true);
}

void Context::update(sf::Time const & elapsed) {
	if (game != nullptr) {
		game->update(elapsed);
	}
}

void Context::onResize(sf::Vector2u screen_size) {
	auto& window = app.getWindow();
	
	background.setPosition(sf::Vector2f{screen_size / 2u});
	
	if (screen_size.x < MIN_SCREEN_WIDTH || screen_size.y < MIN_SCREEN_HEIGHT) {
		screen_size.x = std::max(screen_size.x, MIN_SCREEN_WIDTH);
		screen_size.y = std::max(screen_size.y, MIN_SCREEN_HEIGHT);
		log.warning << "[State/Common] Enlarged too small window to "
			<< screen_size << "\n";
		window.setSize(screen_size);
	} else {
		sf::View gui{{0.f, 0.f, static_cast<float>(screen_size.x),
			static_cast<float>(screen_size.y)}};
		window.setView(gui);
	}
}

void Context::drawBackground(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(background, states);
}

// --------------------------------------------------------------------

LobbyContext::Player::Player()
	: filename{}
	, keys{}
	, tpl{}
	, id{0u}
	, player_id{0u}
	, use_gamepad{false}
	, gamepad_id{0u} {
}

std::string LobbyContext::Player::getSavegameName() const {
	ASSERT(!filename.empty());
	return engine::get_preference_dir() + "saves/" + filename + ".sav";
}

std::string LobbyContext::Player::getKeybindingName() const {
	ASSERT(!filename.empty());
	return engine::get_preference_dir() + "saves/" + filename + ".xml";
}

LobbyContext::LobbyContext(std::size_t max_num_players)
	: num_players{1u}
	, num_dungeons{2u}
	, dungeon_size{100u, 100u}
	, players{} {
	players.resize(max_num_players);
}

bool LobbyContext::hasUnsetProfile(std::size_t& profile) const {
	std::size_t i{0u};
	for (auto const & cfg: players) {
		if (i >= num_players) {
			break;
		}
		if (cfg.filename.empty()) {
			profile = i;
			return true;
		}
		++i;
	}
	return false;
}

bool LobbyContext::hasInconsistentProfile(std::size_t& profile) const {
	std::size_t i{0u};
	for (auto const & cfg: players) {
		if (i >= num_players) {
			break;
		}
		for (auto action: utils::EnumRange<rpg::PlayerAction>{}) {
			auto input = cfg.keys.map.get(action);
			if (cfg.use_gamepad) {
				if (input.type == utils::InputAction::Key) {
					// no keyboard input when using gamepads!
					profile = i;
					return true;
				}
			} else {
				if (input.type != utils::InputAction::Key) {
					// only keyboard input when using gamepads!
					profile = i;
					return true;
				}
			}
		}
		++i;
	}
	return false;
}

bool LobbyContext::hasDoubleUsedProfile(std::size_t& profile) const {
	std::set<std::string> set;
	std::size_t i{0u};
	for (auto const & cfg: players) {
		if (i >= num_players) {
			break;
		}
		if (set.find(cfg.filename) != set.end()) {
			profile = i;
			return true;
		}
		set.insert(cfg.filename);
		++i;
	}
	return false;
}

bool LobbyContext::hasAmbiguousInput(utils::InputAction& input, std::size_t& profile) const {
	std::size_t i{0u};
	for (auto const & config: players) {
		if (i >= num_players) {
			break;
		}
		auto tmp = config.keys.map.getAmbiguousActions();
		if (!tmp.empty()) {
			input = tmp.front();
			profile = i;
			return true;
		}
		++i;
	}
	return false;
}

bool LobbyContext::hasSharedInput(utils::InputAction& input, std::size_t& lhs, std::size_t& rhs) const {
	std::size_t i{0u};
	for (auto const & first: players) {
		if (i >= num_players) {
			break;
		}
		std::size_t j{0u};
		for (auto const & second: players) {
			if (j >= num_players) {
				break;
			}
			if (&first != &second) {
				auto tmp = first.keys.map.getCollisions(second.keys.map);
				if (!tmp.empty()) {
					input = tmp.front();
					lhs = i;
					rhs = j;
					return true;
				}
			}
			++j;
		}
		++i;
	}
	return false;
}

bool LobbyContext::hasSharedGamepad(unsigned int& pad_id) const {
	std::set<unsigned int> used;
	std::size_t i{0u};
	for (auto const & cfg: players) {
		if (i >= num_players) {
			break;
		}
		++i;
		if (!cfg.use_gamepad) {
			continue;
		}
		if (used.find(cfg.gamepad_id) != used.end()) {
			pad_id = cfg.gamepad_id;
			return true;
		}
		used.insert(cfg.gamepad_id);
	}
	return false;
}

// --------------------------------------------------------------------

GameContext::GameContext(App& app, LobbyContext lobby)
	: app{app}
	, parent{app.getContext()}
	, lobby{lobby}
	, engine{parent.log, parent.globals.max_num_objects, app.getWindow().getSize(), parent.globals.zoom,
		parent.globals.audio_poolsize, parent.mod, parent.cache,
		parent.locale}
	, mutex{}
	, saver{parent.log, engine.session, mutex} {
	
	engine.ui.sound.setThreshold(parent.globals.sfx_threshold);
	
	// register music
	std::vector<std::string> music;
	parent.mod.getAllFiles<sf::Music>(music);
	for (auto const & str: music) {
		if (str == parent.globals.title_theme) {
			// ignore title theme as ingame music
			continue;
		}
		engine.ui.audio.addMusic(str);
	}
	
	// register feedback sfx
	for (auto const & pair: parent.globals.feedback) {
		if (pair.second.empty()) {
			continue;
		}
		engine.ui.audio.assign(pair.first, parent.mod.get<sf::SoundBuffer>(pair.second));
	}
	
	// register more sfx
	engine.ui.audio.addLevelup(parent.mod.get<sf::SoundBuffer>(parent.globals.levelup_sfx));
	engine.ui.audio.addPowerup(parent.mod.get<sf::SoundBuffer>(parent.globals.powerup_sfx));
	
	// preload all game data from mod
	parent.mod.preload();
}

void GameContext::update(sf::Time const & elapsed) {
	// not used yet
}

void GameContext::applySettings() {
	engine.ui.lighting.setLevelOfDetails(parent.settings.lighting);
	engine.ui.render.setCastShadows(false); // note: maybe used again later
	engine.ui.sound.setVolume(parent.settings.sound);
	engine.ui.music.setVolume(parent.settings.music);
}

// --------------------------------------------------------------------

void setupTitle(sf::Text& label, std::string const & key, Context& context, std::string const & caption_ext) {
	label.setString(context.locale(key) + caption_ext);
	label.setFillColor(context.globals.ui_color);
	label.setOutlineColor(context.globals.ui_color);
	label.setFont(context.mod.get<sf::Font>(context.globals.title.font));
	label.setCharacterSize(context.globals.title.char_size);
	ui::centerify(label);
}

void setupLabel(sf::Text& label, std::string const & key, Context& context, std::string const & caption_ext) {
	std::string s;
	if (!key.empty()) {
		s = context.locale(key);
	}
	label.setString(s + caption_ext);
	label.setFillColor(context.globals.ui_color);
	label.setOutlineColor(context.globals.ui_color);
	label.setFont(context.mod.get<sf::Font>(context.globals.widget.font));
	label.setCharacterSize(context.globals.widget.char_size);
	ui::centerify(label);
}

void setupWarning(sf::Text& label, Context& context) {
	label.setFillColor(context.globals.ui_warning);
	label.setOutlineColor(context.globals.ui_warning);
	label.setFont(context.mod.get<sf::Font>(context.globals.widget.font));
	label.setCharacterSize(context.globals.widget.char_size);
}

void setupButton(ui::Button& button, std::string const & key, Context& context, std::string const & caption_ext) {
	std::string s;
	if (!key.empty()) {
		s = context.locale(key);
	}
	button.setString(s + caption_ext);
	button.setFont(context.mod.get<sf::Font>(context.globals.widget.font));
	button.setCharacterSize(context.globals.widget.char_size);
	button.setDefaultColor(context.globals.ui_color);
	button.setHighlightColor(context.globals.ui_highlight);
	button.setNavigateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_navigate));
	button.setActivateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_activate));
	button.setChannel(context.sfx);
}

void setupSelect(ui::Select& select, Context& context) {
	select.setFont(context.mod.get<sf::Font>(context.globals.widget.font));
	select.setCharacterSize(context.globals.widget.char_size);
	select.setDefaultColor(context.globals.ui_color);
	select.setHighlightColor(context.globals.ui_highlight);
	select.setWidth(context.globals.ui_widget_width);
	select.setArrowTexture(context.mod.get<sf::Texture>("ui/arrow_left"));
	select.setNavigateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_navigate));
	select.setActivateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_activate));
	select.setAlternateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_alternate));
	select.setChannel(context.sfx);
}

void setupInput(ui::Input& input, std::string const & key, Context& context) {
	input.setString(context.locale(key));
	input.setFont(context.mod.get<sf::Font>(context.globals.widget.font));
	input.setCharacterSize(context.globals.widget.char_size);
	input.setDefaultColor(context.globals.ui_color);
	input.setHighlightColor(context.globals.ui_highlight);
	input.setBoxTexture(context.mod.get<sf::Texture>("ui/input_box"));
	input.setMaxLength(context.globals.max_input_len);
	input.setNavigateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_navigate));
	input.setTypeSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_type));
	input.setUndoSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_undo));
	input.setChannel(context.sfx);
}

void setupCheckbox(ui::Checkbox& checkbox, std::string const & key, Context& context) {
	checkbox.setString(context.locale(key));
	checkbox.setFont(context.mod.get<sf::Font>(context.globals.widget.font));
	checkbox.setCharacterSize(context.globals.widget.char_size);
	checkbox.setDefaultColor(context.globals.ui_color);
	checkbox.setHighlightColor(context.globals.ui_highlight);
	checkbox.setWidth(context.globals.ui_widget_width);
	checkbox.setBoxTexture(context.mod.get<sf::Texture>("ui/option_braces"));
	checkbox.setMarkTexture(context.mod.get<sf::Texture>("ui/option_mark"));
	checkbox.setNavigateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_navigate));
	checkbox.setActivateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_activate));
	checkbox.setDeactivateSfx(context.mod.get<sf::SoundBuffer>(context.globals.ui_menu_sfx_deactivate));
	checkbox.setChannel(context.sfx);
}

} // ::state
