#pragma once
#include <unordered_map>
#include <list>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

namespace ui {

using LineList = std::vector<std::pair<sf::Vector2f, sf::Vector2f>>;

class SystemGraph
	: public sf::Drawable
	, public sf::Transformable {
  private:
	struct Node {
		sf::Color color;
		std::size_t pending;
		std::list<std::size_t> values;
	};
	sf::Time interval;
	std::size_t num_records, max_value;
	sf::Vector2u size;
	sf::Time passed;
	sf::RectangleShape background;
	
	using container = std::unordered_map<std::string, Node>;
	using const_iterator = container::const_iterator;
	
	container systems;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
	SystemGraph();
	
	void setInterval(sf::Time const & interval);
	void setNumRecords(std::size_t num_records);
	void setMaxValue(std::size_t max_value);
	void setSize(sf::Vector2u const & size);
	void setFillColor(sf::Color const & color);

	void init(std::string const& name, sf::Color const& color);
	std::size_t& operator[](std::string const& name);
	void update(sf::Time const& elapsed);
	
	sf::Vector2u getSize() const;
	
	LineList getLines(std::string const & system) const;
	
	const_iterator begin() const;
	const_iterator end() const;
};

};  // ::ui
