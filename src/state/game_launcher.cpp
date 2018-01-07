#include <utils/algorithm.hpp>
#include <ui/common.hpp>
#include <state/game_launcher.hpp>
#include <state/game.hpp>

namespace state {

GameLauncherState::GameLauncherState(App& app, LobbyContext& lobby)
	: LoadThreadState{app}
	, lobby{lobby}
	, label{}
	, gradient{}
	, progress{0.f} {
	auto& context = app.getContext();
	
	gradient[0.f] = sf::Color::White;
	gradient[0.25f] = sf::Color::Yellow;
	gradient[0.5f] = sf::Color::Red;
	gradient[0.75f] = sf::Color::Yellow;
	gradient[1.f] = sf::Color::White;
	
	setupTitle(label, "general.loading", context);
	
	// start thread
	start();
}

GameLauncherState::~GameLauncherState() {
	// destroy game context
	getContext().game = nullptr;
}

void GameLauncherState::load() {
	// assign gamepad ids
	for (auto& player: lobby.players) {
		if (player.use_gamepad) {
			player.keys.map.setGamepadId(player.gamepad_id);
		}
	}
	
	// create game context
	auto& context = getContext();
	context.game = std::make_unique<GameContext>(getApplication(), lobby);
}

void GameLauncherState::postload() {
	// enter game
	auto& app = getApplication();
	auto launch = std::make_unique<state::GameState>(app);
	app.push(launch);
}

void GameLauncherState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(label);
}

void GameLauncherState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	label.setPosition({screen_size.x / 2.f, screen_size.y / 2.f});
}

void GameLauncherState::handle(sf::Event const& event) {
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		default:
			break;
	}
}

void GameLauncherState::update(sf::Time const& elapsed) {
	LoadThreadState::update(elapsed);
	
	progress += elapsed.asSeconds() * 0.25f;
	while (progress > 1.f) {
		progress -= 1.f;
	}
	
	label.setFillColor(gradient.sampleColor(progress));
	label.setOutlineColor(gradient.sampleColor(progress));
}

} // ::state
