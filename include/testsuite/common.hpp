#pragma once
#include <boost/test/unit_test.hpp>

#define BOOST_REQUIRE_BETWEEN(l, v, u)                                       \
	if (v < l || v > u) {                                                    \
		BOOST_FAIL(std::to_string(l) + " <= " + std::to_string(v) + " <= " + \
				   std::to_string(u) + " not satisfied");                    \
	}

#define BOOST_CHECK_BETWEEN(l, v, u)                                          \
	if (v < l || v > u) {                                                     \
		BOOST_ERROR(std::to_string(l) + " <= " + std::to_string(v) + " <= " + \
					std::to_string(u) + " not satisfied");                    \
	}
