#pragma once
#include <ui/imgui.hpp>
#include <state/common.hpp>

namespace tool {

class SpritePackerState
	: public state::State {
  protected:
	ImGuiTextBuffer packlog;
	std::string source, target, result;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onPackClick();
	void onShowClick();
	void onBackClick();
	
  public:
	SpritePackerState(state::App& app);
	
	void handle(sf::Event const & event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::tool
