#pragma once
#include <iostream>
#include <cstdlib>

// ----------------------------------------------------------------------------
// in case of unittesting
#if defined(UNIT_TEST)

#include <stdexcept>
#include <string>
#include <boost/preprocessor/stringize.hpp>
#include <boost/test/unit_test.hpp>

namespace boost {
namespace test {

struct assertion_failed : std::runtime_error {
	assertion_failed(std::string const &msg) : std::runtime_error{msg} {}
};
}
}

#define ASSERT(condition)                                                 \
	if (!(condition)) {                                                   \
		throw boost::test::assertion_failed("Assertion failed at " +      \
											std::string{__FILE__} + ":" + \
											std::to_string(__LINE__));    \
	}

#define TEST_ASSERT_IMPL(function)                        \
	{                                                     \
		bool caught = false;                              \
		try {                                             \
			function;                                     \
		} catch (boost::test::assertion_failed const &) { \
			caught = true;                                \
		}

#define BOOST_CHECK_ASSERT(function)                        \
	TEST_ASSERT_IMPL(function);                             \
	if (!caught) {                                          \
		BOOST_ERROR("No Assertion failed but expected it"); \
	}                                                       \
	}

#define BOOST_REQUIRE_ASSERT(function)                     \
	TEST_ASSERT_IMPL(function);                            \
	if (!caught) {                                         \
		BOOST_FAIL("No Assertion failed but expected it"); \
	}                                                      \
	}

#define BOOST_CHECK_NO_ASSERT(function)                   \
	TEST_ASSERT_IMPL(function);                           \
	if (caught) {                                         \
		BOOST_ERROR("Assertion failed but not expected"); \
	}                                                     \
	}

#define BOOST_REQUIRE_NO_ASSERT(function)                \
	TEST_ASSERT_IMPL(function);                          \
	if (caught) {                                        \
		BOOST_FAIL("Assertion failed but not expected"); \
	}                                                    \
	}

// ----------------------------------------------------------------------------
// in case of regular execution
#else

#include <sstream>
#include <string>
#include <cassert>

namespace assert_impl {

extern std::string fname;

void dump_crash(std::string const & msg);

} // ::assert_impl

#define ASSERT(condition)									\
	if (!(condition)) {										\
		std::stringstream s;								\
		s << "Fatal program error at " << __FILE__ << ":"	\
			<< __LINE__ << " inside "						\
			<< __PRETTY_FUNCTION__<< "\n" << "Condition: "	\
			<< #condition << "\n";							\
		auto str = s.str();									\
		std::cerr << str << std::endl;						\
		assert_impl::dump_crash(str);						\
		std::abort();										\
	}
#endif
