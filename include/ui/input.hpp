#pragma once
#include <SFML/Graphics.hpp>
#include <utils/menu.hpp>
#include <ui/common.hpp>

namespace ui {

class Input
	: public utils::Input
	, public TextWidget {
  private:
	sf::Sprite box;
	sf::SoundBuffer const *type_sfx, *undo_sfx;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onStateChanged() override;
	void onType() override;
	void onUndo() override;
	
  public:
	Input();
	
	void setFocus(bool focused) override;
	void setPosition(sf::Vector2f const & pos) override;
	
	sf::String getContent() const override;
	void setContent(sf::String const& string) override;
	
	void setBoxTexture(sf::Texture const & tex);
	void setTypeSfx(sf::SoundBuffer const & sfx);
	void setUndoSfx(sf::SoundBuffer const & sfx);
	
	void update(sf::Time const & elapsed) override;
};

class FilenameInput
	: public Input {
  public:
	FilenameInput();
};

} // ::ui
