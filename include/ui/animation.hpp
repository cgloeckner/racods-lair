#pragma once
#include <SFML/Graphics.hpp>

#include <utils/animation_utils.hpp>

namespace ui {

class WidgetAnimation {
  private:
	utils::IntervalState scale;
	
  public:
	WidgetAnimation();
	
	void startAnimation();
	void stopAnimation();
	
	void operator()(sf::Text& label, sf::Time const & elapsed);
};

} // ::ui
