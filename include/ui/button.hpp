#pragma once
#include <SFML/Graphics.hpp>
#include <utils/menu.hpp>
#include <ui/common.hpp>

namespace ui {

class Button
	: public utils::Button
	, public TextWidget {
  private:
	sf::SoundBuffer const * activate_sfx;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onActivate() override;
	void onStateChanged() override;
	
  public:
	Button();
	
	void setFocus(bool focused) override;
	void setPosition(sf::Vector2f const & pos) override;
	void setActivateSfx(sf::SoundBuffer const & sfx);
	
	void update(sf::Time const & elapsed) override;
};

} // ::ui
