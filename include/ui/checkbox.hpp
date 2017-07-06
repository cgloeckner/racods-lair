#pragma once
#include <SFML/Graphics.hpp>
#include <utils/menu.hpp>
#include <ui/animation.hpp>

namespace ui {

class Checkbox
	: public utils::Button
	, public TextWidget {
  private:
	sf::Sprite box, mark;
	unsigned int width;
	bool checked;
	sf::SoundBuffer const *activate_sfx, *deactivate_sfx;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onStateChanged() override;
	void onActivate() override;
	
  public:
	Checkbox();
	
	void setChecked(bool checked);
	bool isChecked() const;
	
	void setFocus(bool focused) override;
	void setPosition(sf::Vector2f const & pos) override;
	
	void setWidth(unsigned int width);
	void setBoxTexture(sf::Texture const & tex);
	void setMarkTexture(sf::Texture const & tex);
	void setActivateSfx(sf::SoundBuffer const & sfx);
	void setDeactivateSfx(sf::SoundBuffer const & sfx);
	
	void update(sf::Time const & elapsed) override;
};

} // ::ui
