#pragma once
#include <ui/systemgraph.hpp>

#include <rpg/event.hpp>
#include <state/common.hpp>

namespace state {

void searchPosition(std::vector<sf::Vector2u> const & tiles, sf::Vector2f& pos, core::Dungeon const & dungeon, unsigned int max_step=20u);

// --------------------------------------------------------------------

class GameState
	: public State
	, public utils::EventListener<rpg::ActionEvent, rpg::ExpEvent /* ALPHA ONLY! */> {
  private:
	std::unique_ptr<SubState> child;
	
	bool freezed;
	sf::Text difficulty, fps;
	sf::View default_view;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
  public:
	mutable ui::SystemGraph time_monitor;
	
	GameState(App& app);
	
	void onPause(core::ObjectID player=0u);
	void onSetFreeze(bool flag);
	
	void handle(sf::Event const& event) override;
	void handle(rpg::ActionEvent const& event);
	void handle(rpg::ExpEvent const& event); /* ALPHA ONLY! */
	void update(sf::Time const& elapsed) override;
	
	void onFramerateUpdate(float framerate) override;
	void activate() override;
};

}  // ::state
