#pragma once
#include <utils/animation_utils.hpp>
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

struct StoryContext {
	Context& context;
	sf::Text label;
	std::vector<std::string> lines;
	std::size_t index;
	utils::IntervalState alpha;
	sf::Time wait;
	
	StoryContext(Context& context);
	
	void next();
	bool finished() const;
	void update(sf::Time const & elapsed);
};

// --------------------------------------------------------------------

class StoryState
	: public State {
  private:
	StoryContext story;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onContinue();
	void onQuit();
	
  public:
	StoryState(App& app);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
