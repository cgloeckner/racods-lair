#pragma once
#include <SFML/Audio/Sound.hpp>

#include <utils/menu.hpp>
#include <ui/animation.hpp>

namespace ui {

template <typename T>
void centerify(T& widget);

void centerify(sf::Sprite& sprite);

template <typename T>
void setPosition(T& widget, sf::Vector2f const & pos);

/*
template <typename T>
void setup(T& widget, std::string const & caption, sf::Font const & font,
	unsigned int char_size, sf::Color const & default_color,
	sf::Color const & highlight_color);

template <typename T>
void setup(T& widget, sf::Font const & font,
	unsigned int char_size, sf::Color const & default_color,
	sf::Color const & highlight_color);

void setup(sf::Text& text, std::string const & caption, sf::Font const & font,
	unsigned int char_size, sf::Color const & color);
*/

// --------------------------------------------------------------------

class TextWidget {
  protected:
	sf::Text label;
	std::string caption;
	WidgetAnimation ani;
	sf::Color color, highlight;
	sf::SoundBuffer const * navigate_sfx;
	
	sf::Sound* channel;
	
	virtual void onStateChanged() = 0;
	
	void onFocused(bool focused);
	
  public:
	TextWidget();
	
	void setChannel(sf::Sound& channel);
	
	void setString(std::string const & caption);
	void setFont(sf::Font const & font);
	void setCharacterSize(unsigned int size);
	void setDefaultColor(sf::Color const & color);
	void setHighlightColor(sf::Color const & color);
	void setNavigateSfx(sf::SoundBuffer const & sfx);
	
	std::string getString() const;
	sf::Font const * getFont() const;
	unsigned int getCharacterSize() const;
	sf::Color getDefaultColor() const;
	sf::Color getHighlightColor() const;
};

// --------------------------------------------------------------------

class Menu:
	public utils::Menu<std::size_t> {
  public:
	Menu();
	
	void refreshMenuControls();
};

} // ::ui

// include implementation details
#include <ui/common.inl>
