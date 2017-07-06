#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

namespace ui {

struct NotificationNode {
	sf::Text caption;
	sf::Time age, max_age;
	float decay, alpha;
	
	NotificationNode();
	
	void update(sf::Time const & elapsed);
};

// --------------------------------------------------------------------

class NotificationTexts
	: public sf::Drawable
	, public sf::Transformable {
	
  private:
	sf::Font const * font;
	unsigned int char_size;
	
	std::vector<NotificationNode> nodes;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
  public:
	NotificationTexts();
	
	void setup(sf::Font const & font, unsigned int char_size);
	
	void add(std::string const & string, sf::Color const & color, sf::Time const & max_age);
	
	void update(sf::Time const & elapsed);
};

}  // ::ui
