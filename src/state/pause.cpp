#include <ui/button.hpp>

#include <state/camera_editor.hpp>
#include <state/character_viewer.hpp>
#include <state/controls_viewer.hpp>
#include <state/pause.hpp>
#include <state/settings.hpp>

namespace state {

static std::size_t const CONTINUE = 0u;
static std::size_t const SAVE = 1u;
static std::size_t const CAMERA = 2u;
static std::size_t const CONTROLS = 3u;
static std::size_t const CHARACTER = 4u;
static std::size_t const SETTINGS = 5u;
static std::size_t const QUIT = 10u;

// --------------------------------------------------------------------

PauseState::PauseState(App& app, core::ObjectID actor)
	: State{app}
	, menu{}
	, title_label{} 
	, actor{actor} {
	auto& context = app.getContext();
	auto& game = *context.game;
	
	// stop saving thread and ambient music
	if (game.saver.isRunning()) {
		game.saver.stop();
	}
	
	// pause game music and restart title theme
	game.engine.ui.music.pause();
	context.theme.play();
	
	// setup widgets
	ASSERT(context.game != nullptr);
	auto display_name = context.game->engine.physics.focus.query(actor).display_name;
	setupTitle(title_label, "pause.title", context, " (" + display_name + ")");
	auto& continue_btn = menu.acquire<ui::Button>(CONTINUE);
	auto& save_btn = menu.acquire<ui::Button>(SAVE);
	auto& camera_btn = menu.acquire<ui::Button>(CAMERA);
	auto& controls_btn = menu.acquire<ui::Button>(CONTROLS);
	auto& character_btn = menu.acquire<ui::Button>(CHARACTER);
	auto& settings_btn = menu.acquire<ui::Button>(SETTINGS);
	auto& quit_btn = menu.acquire<ui::Button>(QUIT);
	setupButton(continue_btn, "pause.continue", context);
	setupButton(save_btn, "pause.save", context);
	setupButton(camera_btn, "pause.camera", context);
	setupButton(controls_btn, "pause.controls", context);
	setupButton(character_btn, "pause.character", context);
	setupButton(settings_btn, "pause.settings", context);
	setupButton(quit_btn, "pause.quit", context);
	continue_btn.activate = [&]() { onContinueClick(); };
	save_btn.activate = [&]() { onSaveClick(); };
	camera_btn.activate = [&]() { onCameraClick(); };
	controls_btn.activate = [&]() { onControlsClick(); };
	character_btn.activate = [&]() { onCharacterClick(); };
	settings_btn.activate = [&]() { onSettingsClick(); };
	quit_btn.activate = [&]() { onQuitClick(); };
	
	menu.setFocus(continue_btn);
}

void PauseState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
}

void PauseState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	auto pad = context.globals.vertical_padding;
	
	auto& continue_btn = menu.query<ui::Button>(CONTINUE);
	auto& save_btn = menu.query<ui::Button>(SAVE);
	auto& camera_btn = menu.query<ui::Button>(CAMERA);
	auto& controls_btn = menu.query<ui::Button>(CONTROLS);
	auto& character_btn = menu.query<ui::Button>(CHARACTER);
	auto& settings_btn = menu.query<ui::Button>(SETTINGS);
	auto& quit_btn = menu.query<ui::Button>(QUIT);
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	
	controls_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - 3.f * pad});
	character_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - 2.f * pad});
	settings_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - pad});
	quit_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
	
	float offset{4.f};
	if (save_btn.isVisible()) {
		save_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - offset * pad});
		offset += 1.f;
	}
	if (camera_btn.isVisible()) {
		camera_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - offset * pad});
		offset += 1.f;
	}
	continue_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - offset * pad});
}

void PauseState::onContinueClick() {
	quit();
}

void PauseState::onSaveClick() {
	auto& context = getContext();
	context.game->saver.save();
	context.log.debug << "[State/PauseState] Game saved\n";
}

void PauseState::onCameraClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<CameraEditorState>(app);
	app.push(launch);
}

void PauseState::onControlsClick() {
	auto const & session = getContext().game->engine.session;
	auto const & keys = session.input.query(actor).keys;
	auto const & player_name = session.focus.query(actor).display_name;
	
	auto& app = getApplication();
	auto launch = std::make_unique<ControlsViewerState>(app, keys, player_name);
	app.push(launch);
}

void PauseState::onSettingsClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<SettingsState>(app);
	app.push(launch);
}

void PauseState::onCharacterClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<CharacterViewerState>(app, actor);
	app.push(launch);
}

void PauseState::onQuitClick() {
	// mark game state as quit
	auto states = getApplication().queryStates();
	ASSERT(states.size() >= 2u);
	states[states.size() - 2u]->quit();
	// quit pause
	quit();
}

void PauseState::handle(sf::Event const& event) {
	menu.handle(event);
	
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		case sf::Event::Closed:
			onQuitClick();
			break;
			
		case sf::Event::JoystickConnected:
		case sf::Event::JoystickDisconnected:
			menu.refreshMenuControls();
			break;
			
		default:
			break;
	}
}

void PauseState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	menu.update(elapsed);
}

void PauseState::activate() {
	State::activate();
	
	auto& context = getContext();
	ASSERT(context.game != nullptr);
	auto& game = *context.game;
	
	// show save button if autosave is disabled
	auto& save_btn = menu.query<ui::Button>(SAVE);
	save_btn.setVisible(!context.settings.autosave);
	
	// show camera button in multiplayer session if autocam is disabled
	auto& camera_btn = menu.query<ui::Button>(CAMERA);
	camera_btn.setVisible(game.lobby.num_players > 1u && !context.settings.autocam);
	
	onResize(getApplication().getWindow().getSize());
	
	// update menu controls
	// note: gamepad could be disconnected and reconnected while game
	menu.refreshMenuControls();
}

} // ::state
