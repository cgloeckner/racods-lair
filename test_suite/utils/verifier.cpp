#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/verifier.hpp>

BOOST_AUTO_TEST_SUITE(verifier_test)

BOOST_AUTO_TEST_CASE(verifier_delivers_false_if_verification_failed) {
	utils::Logger log;
	utils::Verifier verify{log};
	verify(false, "test");
	BOOST_CHECK(!verify.result);
}

BOOST_AUTO_TEST_CASE(verifier_delivers_true_if_verification_successful) {
	utils::Logger log;
	utils::Verifier verify{log};
	verify(true, "test");
	BOOST_CHECK(verify.result);
}

BOOST_AUTO_TEST_SUITE_END()
