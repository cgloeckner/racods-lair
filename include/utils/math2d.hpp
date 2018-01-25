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

/// Check whether point is inside Field of View
/// @param center Center position of the FoV
/// @param direction Direction vector of the FoV (no unit vector required)
/// @param fov total angle of the FoV
/// @param max_dist Maximum distance used for the FoV
/// @param pos Position to evaluate
/// @return evaluation result
bool isWithinFov(sf::Vector2f const & center, sf::Vector2f const & direction, float fov,
	float max_dist, sf::Vector2f const & pos);

/// Evaluation position within a Field of View
/// The Field of View is arranged around the center using the direction. The
/// view's size is determined by the given angle and max distance.
/// The position is assumed to be located within the FoV and is evaluated
/// referring its distance and difference angle compared to the center/direction.
/// @param center Center position of the FoV
/// @param direction Direction vector of the FoV (no unit vector required)
/// @param fov total angle of the FoV
/// @param max_dist Maximum distance used for the FoV
/// @param pos Position to evaluate
/// @return evaluation result
float evalPos(sf::Vector2f const & center, sf::Vector2f const & direction, float fov,
	float max_dist, sf::Vector2f const & pos);

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
/// @pre !c2.is_aabb
/// @param p1 Position of the Point
/// @param p2 Center of the Circle
/// @param c2 Collider info of the Circle
/// @return true if Point is inside or at the Circle
bool testPointCirc(sf::Vector2f const & p1, sf::Vector2f const & p2, Collider const & c2);

/// Test whether Point and AABB collide
/// @pre c2.is_aabb
/// @param p1 Position of the Point
/// @param p2 Topleft Position of the AABB
/// @param c2 Collider info of the AABB
/// @return true if Point is inside or at the AABB
bool testPointAABB(sf::Vector2f const & p1, sf::Vector2f const & p2, Collider const & c2);

/// Test whether two Circles collide
/// @pre !c1.is_aabb and !c2.is_aabb
/// @param p1 Center of the first Circle
/// @param c1 Collider info of the first Circle
/// @param p2 Center of the second Circle
/// @param c2 Collider info of the second Circle
/// @return true if Circles intersect in any Point
bool testCircCirc(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2);

/// Test whether two AABBs collide
/// @pre c1.is_aabb and c2.is_aabb
/// @param p1 Topleft Position of the first AABB
/// @param c1 Collider info of the first AABB
/// @param p2 Topleft Position of the second AABB
/// @param c2 Collider info of the second AABB
/// @return true if AABB intersect in any Point
bool testAABBAABB(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2);

/// Test whether a Circle and an AABB collide
/// @pre !c1.is_aabb and c2.is_aabb
/// @param p1 Center of the Circle
/// @param c1 Collider info of the Circle
/// @param p2 Topleft Position of the AABB
/// @param c2 Collider info of the AABB
/// @return true if AABB intersect in any Point
bool testCircAABB(sf::Vector2f const & p1, Collider const & c1, sf::Vector2f const & p2, Collider const & c2);

}  // ::utils

// include implementation details
#include <utils/math2d.inl>
