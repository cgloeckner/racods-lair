#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

class SavegameMenuState
	: public State {
  private:
	bool const just_created;
	ui::Menu menu;
	LobbyContext::Player& player;
	sf::Text title_label;
	
	std::pair<sf::Text, sf::Text> filename, charname, level, last_game;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onControlsClick();
	void onBackClick();
	
  public:
	SavegameMenuState(App& app, LobbyContext::Player& player, bool just_created=false);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
