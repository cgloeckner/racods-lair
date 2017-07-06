#pragma once
#include <SFML/Graphics.hpp>

namespace ui {

class BaseBar
	: public sf::Drawable
	, public sf::Transformable {
  private:
	sf::Drawable &box, &fill;
	
  protected:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
  public:
	BaseBar(sf::Drawable& box, sf::Drawable& fill);
	
	virtual void update(std::uint64_t current, std::uint64_t maximum) = 0;
};

// --------------------------------------------------------------------

class ExperienceBar
	: public BaseBar {
  private:
	sf::RectangleShape box, fill;
	float thickness, value;
	
	void updateValue();
	
  public:
	ExperienceBar();
	
	sf::Vector2u getSize() const;
	
	void setSize(sf::Vector2u const & size);
	void setOutlineColor(sf::Color const & color);
	void setFillColor(sf::Color const & color);
	void setOutlineThickness(float border);

	void update(std::uint64_t current, std::uint64_t maximum) override;
};

// --------------------------------------------------------------------

class StatsBar
	: public BaseBar {
  private:
	sf::Sprite box, fill;
	float value;
	bool valid, vertical;
	
	void updateValue();
	
  public:
	StatsBar(bool vertical=false);
	
	sf::Vector2u getSize() const;
	
	void setBoxTexture(sf::Texture const & tex);
	void setFillTexture(sf::Texture const & tex);
	
	void setFillColor(sf::Color const & color);
	
	bool isValid() const;

	void update(std::uint64_t current, std::uint64_t maximum) override;
};

};  // ::ui
