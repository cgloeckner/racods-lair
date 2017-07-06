#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

namespace ui {

struct FloatingNode {
	sf::Text caption;
	sf::Time age, max_age;
	sf::Vector2f vector;
	float decay, scale, alpha;
	
	FloatingNode();
	
	void update(sf::Time const & elapsed);
};

// --------------------------------------------------------------------

class FloatingTexts
	: public sf::Drawable {
  private:
	std::vector<FloatingNode> nodes;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
  public:
	FloatingTexts();
	
	void add(sf::Font const & font, std::string const & string, unsigned int size,
		sf::Vector2f const & pos, sf::Color const & color, sf::Time const & max_age,
		bool random_dir=true);
	void update(sf::Time const & elapsed);
};

}  // ::ui
