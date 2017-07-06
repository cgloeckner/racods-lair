#pragma once
#include <cmath>
#include <boost/test/unit_test.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Transform.hpp>

namespace unittest {

std::string matrix2string(sf::Transform const& matrix);

}  // ::unittest

// ---------------------------------------------------------------------------

#define BOOST_REQUIRE_VECTOR_EQUAL(u, v)                                   \
	if (u.x != v.x || u.y != v.y) {                                        \
		BOOST_FAIL("Vector <" + std::to_string(u.x) + "," +                \
				   std::to_string(u.y) + "> not equal to <" +              \
				   std::to_string(v.x) + "," + std::to_string(v.y) + ">"); \
	}

#define BOOST_REQUIRE_VECTOR_CLOSE(u, v, eps)                              \
	if (std::abs(u.x - v.x) > eps || std::abs(u.y - v.y) > eps) {          \
		BOOST_FAIL("Vector <" + std::to_string(u.x) + "," +                \
				   std::to_string(u.y) + "> not close to <" +              \
				   std::to_string(v.x) + "," + std::to_string(v.y) + ">"); \
	}

#define BOOST_CHECK_VECTOR_EQUAL(u, v)                                      \
	if (u.x != v.x || u.y != v.y) {                                         \
		BOOST_ERROR("Vector <" + std::to_string(u.x) + "," +                \
					std::to_string(u.y) + "> not equal to <" +              \
					std::to_string(v.x) + "," + std::to_string(v.y) + ">"); \
	}

#define BOOST_CHECK_VECTOR_CLOSE(u, v, eps)                                 \
	if (std::abs(u.x - v.x) > eps || std::abs(u.y - v.y) > eps) {           \
		BOOST_ERROR("Vector <" + std::to_string(u.x) + "," +                \
					std::to_string(u.y) + "> not close to <" +              \
					std::to_string(v.x) + "," + std::to_string(v.y) + ">"); \
	}

// ---------------------------------------------------------------------------

#define BOOST_REQUIRE_RECT_EQUAL(u, v)                                     \
	if (u.left != v.left || u.top != v.top || u.width != v.width ||        \
		u.height != v.height) {                                            \
		BOOST_FAIL("Rect <" + std::to_string(u.left) + "," +               \
				   std::to_string(u.top) + ";" + std::to_string(u.width) + \
				   "x" + std::to_string(u.height) + "> not equal to <" +   \
				   std::to_string(v.left) + "," + std::to_string(v.top) +  \
				   ";" + std::to_string(v.width) + "x" +                   \
				   std::to_string(v.height) + ">");                        \
	}

#define BOOST_CHECK_RECT_EQUAL(u, v)                                        \
	if (u.left != v.left || u.top != v.top || u.width != v.width ||         \
		u.height != v.height) {                                             \
		BOOST_ERROR("Rect <" + std::to_string(u.left) + "," +               \
					std::to_string(u.top) + ";" + std::to_string(u.width) + \
					"x" + std::to_string(u.height) + "> not equal to <" +   \
					std::to_string(v.left) + "," + std::to_string(v.top) +  \
					";" + std::to_string(v.width) + "x" +                   \
					std::to_string(v.height) + ">");                        \
	}

// ---------------------------------------------------------------------------

#define BOOST_REQUIRE_4x4_MATRIX_CLOSE(a, b, eps)                      \
	for (auto i = 0u; i < 16u; ++i) {                                  \
		if (std::abs(a.getMatrix()[i] - b.getMatrix()[i]) > eps) {     \
			BOOST_FAIL("Matrix " + unittest::matrix2string(a) +        \
					   " not close to " + unittest::matrix2string(b)); \
		}                                                              \
	}

#define BOOST_CHECK_4x4_MATRIX_CLOSE(a, b, eps)                         \
	for (auto i = 0u; i < 16u; ++i) {                                   \
		if (std::abs(a.getMatrix()[i] - b.getMatrix()[i]) > eps) {      \
			BOOST_ERROR("Matrix " + unittest::matrix2string(a) +        \
						" not close to " + unittest::matrix2string(b)); \
		}                                                               \
	}

// ---------------------------------------------------------------------------

#define BOOST_REQUIRE_COLOR_EQUAL(u, v)                                      \
	if (u != v) {                                                            \
		BOOST_FAIL("Color (" + std::to_string(static_cast<int>(u.r)) + "," + \
				   std::to_string(static_cast<int>(u.g)) + "," +             \
				   std::to_string(static_cast<int>(u.b)) + "," +             \
				   std::to_string(static_cast<int>(u.a)) +                   \
				   ") not equal to (" +                                      \
				   std::to_string(static_cast<int>(v.r)) + "," +             \
				   std::to_string(static_cast<int>(v.g)) + "," +             \
				   std::to_string(static_cast<int>(v.b)) + "," +             \
				   std::to_string(static_cast<int>(v.a)) + ")");             \
	}

#define BOOST_CHECK_COLOR_EQUAL(u, v)                                         \
	if (u != v) {                                                             \
		BOOST_ERROR("Color (" + std::to_string(static_cast<int>(u.r)) + "," + \
					std::to_string(static_cast<int>(u.g)) + "," +             \
					std::to_string(static_cast<int>(u.b)) + "," +             \
					std::to_string(static_cast<int>(u.a)) +                   \
					") not equal to (" +                                      \
					std::to_string(static_cast<int>(v.r)) + "," +             \
					std::to_string(static_cast<int>(v.g)) + "," +             \
					std::to_string(static_cast<int>(v.b)) + "," +             \
					std::to_string(static_cast<int>(v.a)) + ")");             \
	}

// ---------------------------------------------------------------------------

#define BOOST_REQUIRE_TIME_EQUAL(a, b)                                      \
	if (a != b) {                                                           \
		BOOST_FAIL("Time " + std::to_string(a.asMilliseconds()) + "ms " +   \
				   "does not equal " + std::to_string(b.asMilliseconds()) + \
				   "ms");                                                   \
	}

#define BOOST_CHECK_TIME_EQUAL(a, b)                                         \
	if (a != b) {                                                            \
		BOOST_ERROR("Time " + std::to_string(a.asMilliseconds()) + "ms " +   \
					"does not equal " + std::to_string(b.asMilliseconds()) + \
					"ms");                                                   \
	}
