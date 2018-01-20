#include <utils/assert.hpp>
#include <utils/math2d.hpp>

namespace utils {

unsigned int distance(unsigned int u, unsigned int v) {
	if (u >= v) {
		return u - v;
	} else {
		return v - u;
	}
}

// --------------------------------------------------------------------------------

float evalPos(sf::Vector2f const & center, sf::Vector2f const & direction, float fov,
	float max_dist, sf::Vector2f const & pos) {
	auto delta = pos - center;
	auto dist  = thor::squaredLength(delta);
	auto angle = thor::signedAngle(direction, delta);
	ASSERT(angle <= fov / 2.f);
	
	auto normalized_dist  = dist  / max_dist;
	auto normalized_angle = angle / (fov / 2.f); // center equals angle of 0°
	
	return normalized_dist + normalized_angle * normalized_angle;
}

// --------------------------------------------------------------------------------

Collider::Collider()
	: is_aabb{false}
	, radius{0.f}
	, size{} {
}

void Collider::updateRadiusAABB() {
	ASSERT(is_aabb);
	// based on pythagoras d^2 = a^2 + b^2 with r = 1/2 d
	radius = std::sqrt(0.25 * (size.x * size.x + size.y * size.y));
}

bool testPointCirc(sf::Vector2f const & p1, sf::Vector2f const & p2, Collider const & c2) {
	auto d = distance(p1, p2);
	return d <= c2.radius * c2.radius; // 'cause d is squared
}

bool testPointAABB(sf::Vector2f const & p1, sf::Vector2f const & p2, Collider const & c2) {
	return (p2.x <= p1.x) and (p2.y <= p1.y) and
		(p1.x <= p2.x + c2.size.x) and (p1.y <= p2.y + c2.size.y);
}

bool testCircCirc(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2) {
	auto d = distance(p1, p2);
	float r = c1.radius + c2.radius;
	return d <= r * r; // 'cause d is squared
}

bool testAABBAABB(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2) {
	return (p1.x <= p2.x + c2.size.x) && (p1.y <= p2.y + c2.size.y) and
		(p2.x <= p1.x + c1.size.x) && (p2.y <= p1.y + c1.size.y);
}

bool testCircAABB(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2) {
	// broadphase test using AABBs bounding circle
	if (!testCircCirc(p1, c1, p2, c2)) {
		// Circle and AABB are way too far away from each other
		return false;
	}
	
	// narrowphase:
	// a) pick a vector directing to the rectangle's center
	auto center = p2 + c2.size / 2.f;
	auto vector = center - p1;
	// b) create a point on the Circle's arc using the vector
	vector = thor::unitVector(vector);
	auto p = p1 + vector * c1.radius;
	// c) if that Point collides with the AABB, the Circle collides with it
	//    if not, they does not collide
	return testPointAABB(p, p2, c2);
}

}  // ::utils
