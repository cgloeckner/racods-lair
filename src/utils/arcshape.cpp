#include <Thor/Math.hpp>
#include <Thor/Vectors.hpp>

#include <utils/arcshape.hpp>

namespace utils {

ArcShape::ArcShape(float radius, std::size_t point_count)
	: sf::CircleShape{radius, point_count}
	, angle_span{}
	, direction{0.f, 1.f} {
	setAngle(360.f);
}

void ArcShape::setAngle(float angle) {
	angle_span = thor::toRadian(angle);
	update();
}

void ArcShape::setDirection(sf::Vector2f const & v) {
	direction = v;
	update();
}

float ArcShape::getAngle() const {
	return thor::toDegree(angle_span);
}

sf::Vector2f ArcShape::getDirection() const {
	return direction;
}

sf::Vector2f ArcShape::getPoint(std::size_t index) const {
	auto const radius = getRadius();
	auto angle = index * 2.f * thor::Pi / getPointCount();
	float x{0.f}, y{0.f};
	
	if (0.f <= angle && angle <= angle_span) {
		angle += thor::toRadian(thor::polarAngle(direction));
		angle -= angle_span / 2.f;
		
		x = std::cos(angle) * radius;
		y = std::sin(angle) * radius;
	}
	return {radius + x, radius + y};
}

} // ::utils