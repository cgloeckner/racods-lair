#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

class CameraEditorState
	: public State {
  private:
	std::size_t const OK;
	
	ui::Menu menu;
	sf::Text title_label;
	std::vector<sf::Text> cam_labels;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onOkClick();
	
  public:
	CameraEditorState(App& app);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
