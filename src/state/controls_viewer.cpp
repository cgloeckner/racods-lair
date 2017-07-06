#include <ui/button.hpp>

#include <state/controls_viewer.hpp>

namespace state {

static std::size_t const BACK = 0u;

// --------------------------------------------------------------------

ControlsViewerState::ControlsViewerState(App& app, Keys const & keys, std::string const & player_name)
	: State{app}
	, menu{}
	, title_label{}
	, nodes{} {
	auto& context = app.getContext();
	
	setupTitle(title_label, "controls.title", context, " (" + player_name + ")");
	
	for (auto& pair: nodes) {
		auto& node = pair.second;
		setupLabel(node.first, "controls." + to_string(pair.first), context);
		setupLabel(node.second, "", context, keys.get(pair.first).toString());
	}
	
	auto& back_btn = menu.acquire<ui::Button>(BACK);
	setupButton(back_btn, "general.back", context);
	back_btn.activate = [&]() { onBackClick(); };
}

void ControlsViewerState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
	for (auto const & pair: nodes) {
		auto& node = pair.second;
		target.draw(node.first);
		target.draw(node.second);
	}
}

void ControlsViewerState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	auto pad = context.globals.vertical_padding;
	auto hpad = context.globals.horizontal_padding;
	
	auto& back_btn = menu.query<ui::Button>(BACK);
	
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	back_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
	
	// distribute elements in alternating order
	sf::Vector2f pos{screen_size.x / 2.f, 100.f + 2.f * pad};
	auto n = utils::getEnumCount<rpg::PlayerAction>() / 2;
	std::size_t i{0u};
	bool left{true};
	for (auto& pair: nodes) {
		auto& node = pair.second;
		if (left) {
			ui::setPosition(node.first, {pos.x - 1.5f * hpad, pos.y});
			ui::setPosition(node.second, {pos.x - 0.5f * hpad, pos.y});
		} else {
			ui::setPosition(node.first, {pos.x + 0.5f * hpad, pos.y});
			ui::setPosition(node.second, {pos.x + 1.5f * hpad, pos.y});
		}
		pos.y += pad;
		++i;
		if (i > n) {
			left = false;
			i = 0;
			pos.y = 100.f + 2.f * pad;
		}
	}
}

void ControlsViewerState::onBackClick() {
	quit();
}

void ControlsViewerState::handle(sf::Event const& event) {
	menu.handle(event);
	
	// handle some events
	switch (event.type) {
		case sf::Event::JoystickConnected:
		case sf::Event::JoystickDisconnected:
			menu.refreshMenuControls();
			break;
			
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		case sf::Event::Closed:
			onBackClick();
			break;
			
		default:
			break;
	}
}

void ControlsViewerState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	menu.update(elapsed);
}

} // ::state
