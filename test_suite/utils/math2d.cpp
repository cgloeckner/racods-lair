#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

#include <utils/assert.hpp>
#include <utils/math2d.hpp>

BOOST_AUTO_TEST_SUITE(math2d_test)

BOOST_AUTO_TEST_CASE(int_distance) {
	sf::Vector2u x{12, 15};
	sf::Vector2u y{23, 7};

	BOOST_CHECK_EQUAL(185u, utils::distance(x, y));
}

BOOST_AUTO_TEST_CASE(float_distance) {
	sf::Vector2f x{12.5f, 15.25f};
	sf::Vector2f y{23.0f, 7.3f};

	BOOST_CHECK_CLOSE(173.4525f, utils::distance(x, y), 0.0001f);
}

BOOST_AUTO_TEST_CASE(normalize_regular_vector) {
	sf::Vector2f v{2.f, 4.f};
	
	BOOST_CHECK_VECTOR_CLOSE(utils::normalize(v), thor::unitVector(v), 0.0001f);
}

BOOST_AUTO_TEST_CASE(normalize_zero_vector) {
	sf::Vector2f v;
	
	BOOST_CHECK_VECTOR_CLOSE(utils::normalize(v), v, 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(isWithinFov_regular_case) {
	sf::Vector2f center{5.f, 2.f};
	sf::Vector2f direction{0.f, 1.f};
	float fov{160.f};
	float max_dist{3.f};
	
	BOOST_CHECK(utils::isWithinFov(center, direction, fov, max_dist, {5.f, 5.f}));
}

BOOST_AUTO_TEST_CASE(isWithinFov_fails_if_angle_too_large) {
	sf::Vector2f center{5.f, 2.f};
	sf::Vector2f direction{0.f, 1.f};
	float fov{160.f};
	float max_dist{3.f};
	
	BOOST_CHECK(!utils::isWithinFov(center, direction, fov, max_dist, {4.f, 2.f}));
}

BOOST_AUTO_TEST_CASE(isWithinFov_fails_if_too_far_away) {
	sf::Vector2f center{5.f, 2.f};
	sf::Vector2f direction{0.f, 1.f};
	float fov{160.f};
	float max_dist{3.f};
	
	BOOST_CHECK(!utils::isWithinFov(center, direction, fov, max_dist, {5.f, 12.f}));
}

BOOST_AUTO_TEST_CASE(evalPos_evaluates_dist_wise_close_pos_as_better) {
	sf::Vector2f center{5.f, 2.f};
	sf::Vector2f direction{0.f, 1.f};
	float fov{160.f};
	float max_dist{10.f};
	
	auto eval1 = utils::evalPos(center, direction, fov, max_dist, {5.f, 5.f});
	auto eval2 = utils::evalPos(center, direction, fov, max_dist, {5.2f, 5.2f});
	auto eval3 = utils::evalPos(center, direction, fov, max_dist, {5.2f, 6.f});
	BOOST_CHECK_LT(eval1, eval2);
	BOOST_CHECK_LT(eval2, eval3);
}

BOOST_AUTO_TEST_CASE(evalPos_evaluates_angle_wise_close_pos_as_better) {
	sf::Vector2f center{5.f, 2.f};
	sf::Vector2f direction{0.f, 1.f};
	float fov{160.f};
	float max_dist{10.f};
	
	auto eval1 = utils::evalPos(center, direction, fov, max_dist, {5.f, 5.f});
	auto eval2 = utils::evalPos(center, direction, fov, max_dist, {6.f, 5.f});
	auto eval3 = utils::evalPos(center, direction, fov, max_dist, {7.f, 4.f});
	BOOST_CHECK_LT(eval1, eval2);
	BOOST_CHECK_LT(eval3, eval2);
	BOOST_CHECK_LT(eval1, eval3);
}

BOOST_AUTO_TEST_CASE(evalPos_evaluates_way_distance_close_pos_better_then_angle_wise_close_pos) {
	sf::Vector2f center{5.f, 2.f};
	sf::Vector2f direction{0.f, 1.f};
	float fov{180.f};
	float max_dist{10.f};
	
	auto eval1 = utils::evalPos(center, direction, fov, max_dist, {5.f, 5.f});
	auto eval2 = utils::evalPos(center, direction, fov, max_dist, {2.f, 3.f});
	BOOST_CHECK_LT(eval1, eval2);
}

BOOST_AUTO_TEST_CASE(evalPos_evaluates_out_of_fov_pos_with_negative_value) {
	sf::Vector2f center{5.f, 2.f};
	sf::Vector2f direction{0.f, 1.f};
	float fov{120.f};
	float max_dist{3.f};
	
	auto eval = utils::evalPos(center, direction, fov, max_dist, {5.f, 1.f});
	BOOST_CHECK_CLOSE(eval, -1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(evalPos_evaluates_out_of_range_pos_with_negative_value) {
	sf::Vector2f center{5.f, 2.f};
	sf::Vector2f direction{0.f, 1.f};
	float fov{120.f};
	float max_dist{3.f};
	
	auto eval = utils::evalPos(center, direction, fov, max_dist, {5.f, 13.f});
	BOOST_CHECK_CLOSE(eval, -1.f, 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(AABB_broadphase_radius_update_does_not_work_on_circle) {
	utils::Collider c;
	c.is_aabb = false;
	c.size = {20.f, 20.f};
	BOOST_CHECK_ASSERT(c.updateRadiusAABB());
}

BOOST_AUTO_TEST_CASE(AABB_broadphase_radius_update) {
	utils::Collider c;
	c.is_aabb = true;
	c.size = {20.f, 10.f};
	c.updateRadiusAABB();
	
	BOOST_CHECK_CLOSE(c.radius, 11.18f, 0.1f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(point_inside_circ_does_collide) {
	sf::Vector2f p1{22.f, 23.f}, p2{20.f, 20.f};
	utils::Collider c2;
	c2.radius = 5.f;
	BOOST_REQUIRE(!c2.is_aabb);
	
	BOOST_CHECK(utils::testPointCirc(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_at_circ_arc_does_collide) {
	sf::Vector2f p1{20.f, 25.f}, p2{20.f, 20.f};
	utils::Collider c2;
	c2.radius = 5.f;
	BOOST_REQUIRE(!c2.is_aabb);
	
	BOOST_CHECK(utils::testPointCirc(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_outside_arc_does_not_collide) {
	sf::Vector2f p1{20.f, 26.f}, p2{20.f, 20.f};
	utils::Collider c2;
	c2.radius = 5.f;
	BOOST_REQUIRE(!c2.is_aabb);
	
	BOOST_CHECK(!utils::testPointCirc(p1, p2, c2));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(point_inside_aabb_does_collide) {
	sf::Vector2f p1{30.f, 15.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testPointAABB(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_at_aabb_bottom_border_does_collide) {
	sf::Vector2f p1{30.f, 20.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testPointAABB(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_below_aabb_does_not_collide) {
	sf::Vector2f p1{30.f, 21.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(!utils::testPointAABB(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_at_aabb_top_border_does_collide) {
	sf::Vector2f p1{30.f, 10.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testPointAABB(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_above_aabb_does_not_collide) {
	sf::Vector2f p1{30.f, 9.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(!utils::testPointAABB(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_at_aabb_left_border_does_collide) {
	sf::Vector2f p1{10.f, 15.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testPointAABB(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_left_of_aabb_does_not_collide) {
	sf::Vector2f p1{9.f, 15.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(!utils::testPointAABB(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_at_aabb_right_border_does_collide) {
	sf::Vector2f p1{30.f, 15.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testPointAABB(p1, p2, c2));
}

BOOST_AUTO_TEST_CASE(point_right_of_aabb_does_not_collide) {
	sf::Vector2f p1{31.f, 15.f}, p2{10.f, 10.f};
	utils::Collider c2;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(!utils::testPointAABB(p1, p2, c2));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(circles_collide_if_close_enough) {
	sf::Vector2f p1{10.f, 10.f}, p2{20.f, 10.f};
	utils::Collider c1, c2;
	c1.radius = 4.f;
	c2.radius = 8.f;
	
	BOOST_CHECK(utils::testCircCirc(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(circles_collide_if_just_close_enough) {
	sf::Vector2f p1{10.f, 10.f}, p2{22.f, 10.f};
	utils::Collider c1, c2;
	c1.radius = 4.f;
	c2.radius = 8.f;
	
	BOOST_CHECK(utils::testCircCirc(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(circles_do_not_collide_if_too_far) {
	sf::Vector2f p1{10.f, 10.f}, p2{23.f, 10.f};
	utils::Collider c1, c2;
	c1.radius = 4.f;
	c2.radius = 8.f;
	
	BOOST_CHECK(!utils::testCircCirc(p1, c1, p2, c2));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(AABBs_collide_if_close_enough) {
	sf::Vector2f p1{10.f, 10.f}, p2{30.f, 15.f};
	utils::Collider c1, c2;
	c1.is_aabb = true;
	c1.size = {20.f, 10.f};
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testAABBAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(AABBs_collide_if_just_close_enough_to_the_right) {
	sf::Vector2f p1{10.f, 10.f}, p2{30.f, 10.f};
	utils::Collider c1, c2;
	c1.is_aabb = true;
	c1.size = {20.f, 10.f};
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testAABBAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(AABBs_collide_if_just_close_enough_to_the_left) {
	sf::Vector2f p1{10.f, 10.f}, p2{-10.f, 10.f};
	utils::Collider c1, c2;
	c1.is_aabb = true;
	c1.size = {20.f, 10.f};
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testAABBAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(AABBs_collide_if_just_close_enough_to_the_top) {
	sf::Vector2f p1{10.f, 10.f}, p2{10.f, 0.f};
	utils::Collider c1, c2;
	c1.is_aabb = true;
	c1.size = {20.f, 10.f};
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testAABBAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(AABBs_collide_if_just_close_enough_to_the_bottom) {
	sf::Vector2f p1{10.f, 10.f}, p2{10.f, 20.f};
	utils::Collider c1, c2;
	c1.is_aabb = true;
	c1.size = {20.f, 10.f};
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(utils::testAABBAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(AABBs_do_not_collide_if_too_far) {
	sf::Vector2f p1{10.f, 10.f}, p2{40.f, 22.f};
	utils::Collider c1, c2;
	c1.is_aabb = true;
	c1.size = {20.f, 10.f};
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	
	BOOST_CHECK(!utils::testAABBAABB(p1, c1, p2, c2));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Circ_and_AABB_collide_if_close_enough) {
	sf::Vector2f p1{10.f, 10.f}, p2{0.f, 15.f};
	utils::Collider c1, c2;
	c1.radius = 10.f;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	c2.updateRadiusAABB(); // <-- update broadphase radius using AABB size
	BOOST_REQUIRE(!c1.is_aabb);
	
	BOOST_CHECK(utils::testCircAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(Circ_and_AABB_collide_if_just_close_enough) {
	sf::Vector2f p1{10.f, 10.f}, p2{0.f, 20.f};
	utils::Collider c1, c2;
	c1.radius = 10.f;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	c2.updateRadiusAABB(); // <-- update broadphase radius using AABB size
	BOOST_REQUIRE(!c1.is_aabb);
	
	BOOST_CHECK(utils::testCircAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(Circ_and_AABB_do_not_collide_if_too_far_away) {
	sf::Vector2f p1{10.f, 10.f}, p2{0.f, 21.f};
	utils::Collider c1, c2;
	c1.radius = 10.f;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	c2.updateRadiusAABB(); // <-- update broadphase radius using AABB size
	BOOST_REQUIRE(!c1.is_aabb);
	
	BOOST_CHECK(!utils::testCircAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_CASE(Circ_and_AABB_do_not_collide_if_way_too_far_away_so_the_broadphase_kicks_in) {
	sf::Vector2f p1{10.f, 10.f}, p2{0.f, 100.f};
	utils::Collider c1, c2;
	c1.radius = 10.f;
	c2.is_aabb = true;
	c2.size = {20.f, 10.f};
	c2.updateRadiusAABB(); // <-- update broadphase radius using AABB size
	BOOST_REQUIRE(!c1.is_aabb);
	
	BOOST_CHECK(!utils::testCircAABB(p1, c1, p2, c2));
}

BOOST_AUTO_TEST_SUITE_END()
