#pragma once
#include <SFML/Graphics/CircleShape.hpp>

namespace utils {

/// Circle Arc Shape using a direction vector and a spanning angle
class ArcShape: public sf::CircleShape {
	private:
		float angle_span;
		sf::Vector2f direction;
		
	public:
		ArcShape(float radius=0.f, std::size_t point_count=30u);
		
		void setAngle(float angle);
		void setDirection(sf::Vector2f const & v);
		
		float getAngle() const;
		sf::Vector2f getDirection() const;
		
		virtual sf::Vector2f getPoint(std::size_t index) const;
};

} // ::utils