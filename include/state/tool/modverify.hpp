#pragma once
#include <ui/imgui.hpp>
#include <state/common.hpp>

namespace tool {

class ModVerifyState
	: public state::State {
  protected:
	ImGuiTextBuffer modlog;
	std::string modname, result;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onVerifyClick();
	void onBackClick();
	
  public:
	ModVerifyState(state::App& app);
	
	void handle(sf::Event const & event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::tool
