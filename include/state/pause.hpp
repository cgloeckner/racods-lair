#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

class PauseState
	: public State {
  private:
	ui::Menu menu;
	sf::Text title_label;
	core::ObjectID const actor;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onContinueClick();
	void onSaveClick();
	void onCameraClick();
	void onControlsClick();
	void onCharacterClick();
	void onSettingsClick();
	void onQuitClick();
	
  public:
	PauseState(App& app, core::ObjectID player);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
	
	void activate() override;
};

} // ::state
