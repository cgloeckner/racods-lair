#pragma once
#include <rpg/resources.hpp>
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

using Keys = utils::Keybinding<rpg::PlayerAction>;

class ControlsViewerState
	: public State {
  private:
	ui::Menu menu;
	sf::Text title_label;
	utils::EnumMap<rpg::PlayerAction, std::pair<sf::Text, sf::Text>> nodes;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onBackClick();
	
  public:
	ControlsViewerState(App& app, Keys const & keys, std::string const & player_name);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
