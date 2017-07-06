#include <utils/filesystem.hpp>

#include <ui/button.hpp>
#include <ui/checkbox.hpp>
#include <ui/select.hpp>

#include <state/lobby.hpp>
#include <state/savegame_menu.hpp>
#include <state/controls_editor.hpp>

namespace state {

static std::size_t const CONTROLS = 0u;
static std::size_t const BACK = 10u;

// --------------------------------------------------------------------

SavegameMenuState::SavegameMenuState(App& app, LobbyContext::Player& player, bool just_created)
	: State{app}
	, just_created{just_created}
	, menu{}
	, player{player}
	, title_label{}
	, filename{}
	, charname{}
	, level{}
	, last_game{} {
	auto& context = getContext();
	
	char timestring[80];
	auto rawtime = utils::get_last_changed_date(player.getSavegameName());
	struct tm* timeinfo = localtime(&rawtime);
	strftime(timestring, 80, "%X, %x", timeinfo);
	
	// setup widgets
	setupTitle(title_label, "savegame.title", context);
	setupLabel(filename.first, "general.filename", context);
	setupLabel(filename.second, "", context, player.filename);
	setupLabel(charname.first, "general.charname", context);
	setupLabel(charname.second, "", context, player.tpl.display_name);
	setupLabel(level.first, "general.level", context);
	setupLabel(level.second, "", context, std::to_string(player.tpl.level));
	setupLabel(last_game.first, "savegame.last_game", context);
	setupLabel(last_game.second, "", context, timestring);
	
	auto& controls_btn = menu.acquire<ui::Button>(CONTROLS);
	auto& back_btn = menu.acquire<ui::Button>(BACK);
	controls_btn.activate = [&]() { onControlsClick(); };
	back_btn.activate = [&]() { onBackClick(); };
	setupButton(controls_btn, "savegame.controls", context);
	setupButton(back_btn, "general.back", context);
}

void SavegameMenuState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
	target.draw(filename.first, states);
	target.draw(filename.second, states);
	target.draw(charname.first, states);
	target.draw(charname.second, states);
	target.draw(level.first, states);
	target.draw(level.second, states);
	target.draw(last_game.first, states);
	target.draw(last_game.second, states);
}

void SavegameMenuState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	auto pad = context.globals.vertical_padding;
	auto hpad = context.globals.horizontal_padding / 2u;
	
	auto& controls_btn = menu.query<ui::Button>(CONTROLS);
	auto& back_btn = menu.query<ui::Button>(BACK);
	
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	ui::setPosition(filename.first, {screen_size.x / 2.f - hpad, 100.f + 2 * pad});
	ui::setPosition(filename.second, {screen_size.x / 2.f + hpad, 100.f + 2 * pad});
	ui::setPosition(charname.first, {screen_size.x / 2.f - hpad, 100.f + 3 * pad});
	ui::setPosition(charname.second, {screen_size.x / 2.f + hpad, 100.f + 3 * pad});
	ui::setPosition(level.first, {screen_size.x / 2.f - hpad, 100.f + 4 * pad});
	ui::setPosition(level.second, {screen_size.x / 2.f + hpad, 100.f + 4 * pad});
	ui::setPosition(last_game.first, {screen_size.x / 2.f - hpad, 100.f + 5 * pad});
	ui::setPosition(last_game.second, {screen_size.x / 2.f + hpad, 100.f + 5 * pad});
	
	controls_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - 2 * pad});
	back_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - pad});
}

void SavegameMenuState::onControlsClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<ControlsEditorState>(app, player);
	app.push(launch);
}

void SavegameMenuState::onBackClick() {
	if (just_created) {
		// mark profile creator state as quit too
		auto states = getApplication().queryStates();
		ASSERT(states.size() >= 2u);
		states[states.size() - 2u]->quit();
	}
	// quit controls editor
	quit();
}

void SavegameMenuState::handle(sf::Event const& event) {
	menu.handle(event);
	
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		case sf::Event::JoystickConnected:
		case sf::Event::JoystickDisconnected:
			menu.refreshMenuControls();
			break;
			
		case sf::Event::Closed:
			onBackClick();
			break;
			
		default:
			break;
	}
}

void SavegameMenuState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	menu.update(elapsed);
}

} // ::state
