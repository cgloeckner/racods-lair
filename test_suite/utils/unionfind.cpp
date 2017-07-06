#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/unionfind.hpp>

BOOST_AUTO_TEST_SUITE(unionfind_test)

BOOST_AUTO_TEST_CASE(unionfind_parent_of_self) {
	utils::Unionfind dsf;
	auto& a = dsf.make();
	BOOST_CHECK_EQUAL(&a, &dsf.find(a));
}

BOOST_AUTO_TEST_CASE(unionfind_disjoint_sets) {
	utils::Unionfind dsf;
	auto& a = dsf.make();
	auto& b = dsf.make();

	BOOST_REQUIRE_NE(&a, &b);
	BOOST_CHECK_EQUAL(&dsf.find(a), &a);
	BOOST_CHECK_EQUAL(&dsf.find(b), &b);
}

BOOST_AUTO_TEST_CASE(unionfind_link_once_to_shared_parent) {
	utils::Unionfind dsf;
	auto& a = dsf.make();
	auto& b = dsf.make();
	dsf.join(b, a);

	BOOST_CHECK_EQUAL(&a, &dsf.find(b));
	BOOST_CHECK_EQUAL(&a, &dsf.find(a));
}

BOOST_AUTO_TEST_CASE(unionfind_link_once_inverse_to_shared_parent) {
	utils::Unionfind dsf;
	auto& a = dsf.make();
	auto& b = dsf.make();
	dsf.join(a, b);

	BOOST_CHECK_EQUAL(&b, &dsf.find(a));
	BOOST_CHECK_EQUAL(&b, &dsf.find(b));
}

BOOST_AUTO_TEST_CASE(unionfind_link_cycle_causes_one_to_be_parent) {
	utils::Unionfind dsf;
	auto& a = dsf.make();
	auto& b = dsf.make();
	dsf.join(b, a);
	dsf.join(a, b);

	BOOST_CHECK_EQUAL(&a, &dsf.find(b));
	BOOST_CHECK_EQUAL(&a, &dsf.find(a));
}

BOOST_AUTO_TEST_CASE(unionfind_link_star_to_shared_parent) {
	utils::Unionfind dsf;
	auto& parent = dsf.make();
	std::vector<utils::Unionfind::Set*> sets;
	for (int i = 0; i < 7; ++i) {
		sets.push_back(&dsf.make());
		dsf.join(*sets.back(), parent);
	}

	for (int i = 0; i < 7; ++i) {
		BOOST_CHECK_EQUAL(&parent, &dsf.find(*sets[i]));
	}
}

BOOST_AUTO_TEST_CASE(unionfind_link_chain_to_shared_parent) {
	utils::Unionfind dsf;
	auto& parent = dsf.make();
	std::vector<utils::Unionfind::Set*> sets;
	for (int i = 0; i < 7; ++i) {
		sets.push_back(&dsf.make());
		if (i > 0) {
			dsf.join(*sets.back(), *sets[i - 1]);
		} else {
			dsf.join(*sets.back(), parent);
		}
	}

	for (int i = 0; i < 7; ++i) {
		BOOST_CHECK_EQUAL(&parent, &dsf.find(*sets[i]));
	}
}

BOOST_AUTO_TEST_CASE(unionfind_link_groups_to_shared_parent) {
	utils::Unionfind dsf;
	auto& a1 = dsf.make();
	auto& a2 = dsf.make();
	auto& a3 = dsf.make();
	auto& a4 = dsf.make();
	auto& b1 = dsf.make();
	auto& b2 = dsf.make();
	auto& b3 = dsf.make();
	dsf.join(a2, a1);
	dsf.join(a3, a1);
	dsf.join(a4, a1);
	dsf.join(b2, b1);
	dsf.join(b3, b1);
	BOOST_REQUIRE_NE(&dsf.find(a4), &dsf.find(b3));

	dsf.join(a2, b3);
	BOOST_REQUIRE_EQUAL(&dsf.find(a4), &dsf.find(b3));
}

BOOST_AUTO_TEST_SUITE_END()
