#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

class TitleState
	: public State {
  private:
	ui::Menu menu;
	sf::Text title_label;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onLobbyClick();
	void onSettingsClick();
	void onQuitClick();
	
  public:
	TitleState(App& app);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
	
	void activate() override;
};

} // ::state
