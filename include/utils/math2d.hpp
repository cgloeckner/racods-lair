#pragma once
#include <SFML/System/Vector2.hpp>

namespace utils {

/// Calculate distance between two integer values
unsigned int distance(unsigned int u, unsigned int v);

/// Squared distance between two 2d vectors (the actual root is not calculated)
/// @param u first 2d vector
/// @param v second 2d vector
/// @result squared distance
template <typename T>
T distance(sf::Vector2<T> const& u, sf::Vector2<T> const& v);

// ---------------------------------------------------------------------------

/// Collision information
struct Collider {
	bool is_aabb;
	float radius;
	sf::Vector2f size; // used if is_aabb
	
	Collider();
	
	/// Update radius for broadphase collision (AABB only)
	void updateRadiusAABB();
};

/// Test whether Point and Circle collide
/// @param p1 Position of the Point
/// @param p2 Center of the Circle
/// @param c2 Collider info of the Circle
/// @return true if Point is inside or at the Circle
bool testPointCirc(sf::Vector2f const & p1, sf::Vector2f const & p2, Collider const & c2);

/// Test whether Point and AABB collide
/// @param p1 Position of the Point
/// @param p2 Topleft Position of the AABB
/// @param c2 Collider info of the AABB
/// @return true if Point is inside or at the AABB
bool testPointAABB(sf::Vector2f const & p1, sf::Vector2f const & p2, Collider const & c2);

/// Test whether two Circles collide
/// @param p1 Center of the first Circle
/// @param c1 Collider info of the first Circle
/// @param p2 Center of the second Circle
/// @param c2 Collider info of the second Circle
/// @return true if Circles intersect in any Point
bool testCircCirc(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2);

/// Test whether two AABBs collide
/// @param p1 Topleft Position of the first AABB
/// @param c1 Collider info of the first AABB
/// @param p2 Topleft Position of the second AABB
/// @param c2 Collider info of the second AABB
/// @return true if AABB intersect in any Point
bool testAABBAABB(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2);

/// Test whether a Circle and an AABB collide
/// @param p1 Center of the Circle
/// @param c1 Collider info of the Circle
/// @param p2 Topleft Position of the AABB
/// @param c2 Collider info of the AABB
/// @return true if AABB intersect in any Point
bool testCircAABB(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2);

}  // ::utils

// include implementation details
#include <utils/math2d.inl>
