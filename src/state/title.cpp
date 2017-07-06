#include <ui/button.hpp>

#include <state/title.hpp>
#include <state/lobby.hpp>
#include <state/settings.hpp>

namespace state {

static std::size_t const LOBBY = 0u;
static std::size_t const SETTINGS = 1u;
static std::size_t const QUIT = 2u;

// --------------------------------------------------------------------

TitleState::TitleState(App& app)
	: State{app}
	, menu{}
	, title_label{} {
	auto& context = app.getContext();
	
	// setup widgets
	setupTitle(title_label, "", context, engine::TITLE);
	auto& lobby_btn = menu.acquire<ui::Button>(LOBBY);
	auto& settings_btn = menu.acquire<ui::Button>(SETTINGS);
	auto& quit_btn = menu.acquire<ui::Button>(QUIT);
	menu.setFocus(lobby_btn); // note: avoid focus sound when title is entered
	setupButton(lobby_btn, "title.lobby", context);
	setupButton(settings_btn, "title.settings", context);
	setupButton(quit_btn, "title.quit", context);
	lobby_btn.activate = [&]() { onLobbyClick(); };
	settings_btn.activate = [&]() { onSettingsClick(); };
	quit_btn.activate = [&]() { onQuitClick(); };
}

void TitleState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
}

void TitleState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	auto pad = context.globals.vertical_padding;
	
	auto& lobby_btn = menu.query<ui::Button>(LOBBY);
	auto& settings_btn = menu.query<ui::Button>(SETTINGS);
	auto& quit_btn = menu.query<ui::Button>(QUIT);
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	lobby_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - 2.f * pad});
	settings_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - pad});
	quit_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
}

void TitleState::onLobbyClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<LobbyState>(app);
	app.push(launch);
}

void TitleState::onSettingsClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<SettingsState>(app);
	app.push(launch);
}

void TitleState::onQuitClick() {
	// mark story state as quit, too
	auto states = getApplication().queryStates();
	ASSERT(states.size() >= 3u);
	states[states.size() - 3u]->quit();
	states[states.size() - 2u]->quit();
	// quit pause
	quit();
}

void TitleState::handle(sf::Event const& event) {
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

void TitleState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	menu.update(elapsed);
}


void TitleState::activate() {
	State::activate();
	
	menu.refreshMenuControls();
}

} // ::state
