#pragma once
#include <SFML/Graphics.hpp>
#include <utils/menu.hpp>
#include <ui/common.hpp>

namespace ui {

class Select
	: public utils::Select
	, public TextWidget {
  private:
	sf::Sprite left, right;
	unsigned int width;
	sf::SoundBuffer const *activate_sfx, *alternate_sfx;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onActivate() override;
	void onStateChanged() override;
	void onChanged() override;
	void onUpdate() override;
	
  public:
	Select();
	
	void setFocus(bool focused) override;
	void setPosition(sf::Vector2f const & pos) override;
	
	void setWidth(unsigned int width);
	void setArrowTexture(sf::Texture const & tex);
	void setActivateSfx(sf::SoundBuffer const & sfx);
	void setAlternateSfx(sf::SoundBuffer const & sfx);
	
	void update(sf::Time const & elapsed) override;
};

} // ::ui
