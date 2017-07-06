#pragma once
#include <ui/imgui.hpp>
#include <state/common.hpp>

namespace tool {

class ToolMenuState
	: public state::State {
  protected:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onVerifyClick();
	void onPackerClick();
	void onViewerClick();
	void onRoomClick();
	void onSavegameClick();
	void onQuitClick();
	
  public:
	ToolMenuState(state::App& app);
	
	void handle(sf::Event const & event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::tool
