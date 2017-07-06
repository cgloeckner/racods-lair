#pragma once
#include <utils/animation_utils.hpp>
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

struct LogoContext {
	sf::Sprite sprite;
	std::vector<sf::Texture const *> gfx;
	std::size_t index;
	utils::IntervalState alpha;
	sf::Time wait;
	
	LogoContext(Context& context);
	
	void next();
	bool finished() const;
	void update(sf::Time const & elapsed);
};

// --------------------------------------------------------------------

class LogoState
	: public State {
  private:
	LogoContext logo;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onContinue();
	void onQuit();
	
  public:
	LogoState(App& app);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
