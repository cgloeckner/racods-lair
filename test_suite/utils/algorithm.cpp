#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/algorithm.hpp>

BOOST_AUTO_TEST_SUITE(algorithm_test)

BOOST_AUTO_TEST_CASE(algorithm_int_distance) {
	sf::Vector2u x{12, 15};
	sf::Vector2u y{23, 7};

	BOOST_CHECK_EQUAL(185u, utils::distance(x, y));
}

BOOST_AUTO_TEST_CASE(algorithm_float_distance) {
	sf::Vector2f x{12.5f, 15.25f};
	sf::Vector2f y{23.0f, 7.3f};

	BOOST_CHECK_CLOSE(173.4525f, utils::distance(x, y), 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(can_enlarge_rect_to_topleft) {
	///	####
	///	# *#**
	///	#### *
	///   ****
	sf::FloatRect lhs{0.f, 0.f, 4.f, 3.f}, rhs{2.f, 1.f, 4.f, 3.f};
	auto result = utils::enlarge(rhs, lhs);
	BOOST_CHECK_CLOSE(result.left, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.top, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.width, 6.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.height, 4.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(can_enlarge_rect_to_bottomright) {
	///	####
	///	# *#**
	///	#### *
	///   ****
	sf::FloatRect lhs{0.f, 0.f, 4.f, 3.f}, rhs{2.f, 1.f, 4.f, 3.f};
	auto result = utils::enlarge(lhs, rhs);
	BOOST_CHECK_CLOSE(result.left, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.top, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.width, 6.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.height, 4.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(no_enlarge_if_rhs_is_completly_inside_lhs) {
	///	####
	///	# *#
	///	####
	sf::FloatRect lhs{0.f, 0.f, 4.f, 3.f}, rhs{2.f, 1.f, 1.f, 1.f};
	auto result = utils::enlarge(lhs, rhs);
	BOOST_CHECK_CLOSE(result.left, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.top, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.width, 4.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.height, 3.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(complete_enlarge_if_rhs_is_completly_ouside_lhs) {
	///	####
	///	# *#
	///	####
	sf::FloatRect lhs{0.f, 0.f, 4.f, 3.f}, rhs{2.f, 1.f, 1.f, 1.f};
	auto result = utils::enlarge(rhs, lhs);
	BOOST_CHECK_CLOSE(result.left, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.top, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.width, 4.f, 0.0001f);
	BOOST_CHECK_CLOSE(result.height, 3.f, 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(algorithm_contains) {
	std::vector<int> v{5, 1, -3, 12, 0, 9};

	BOOST_CHECK(utils::contains(v, 5));
	BOOST_CHECK(utils::contains(v, 9));
	BOOST_CHECK(utils::contains(v, -3));
	BOOST_CHECK(!utils::contains(v, 15));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(algorithm_pop_first) {
	std::vector<int> v{5, 1, -3, 12, 0, 9};

	BOOST_REQUIRE(utils::pop(v, 5));
	BOOST_CHECK_EQUAL(5u, v.size());
	BOOST_CHECK_EQUAL(v[0], 9);
	BOOST_CHECK_EQUAL(v[1], 1);
	BOOST_CHECK_EQUAL(v[2], -3);
	BOOST_CHECK_EQUAL(v[3], 12);
	BOOST_CHECK_EQUAL(v[4], 0);
}

BOOST_AUTO_TEST_CASE(algorithm_pop_some) {
	std::vector<int> v{9, 1, -3, 12, 0};
	BOOST_REQUIRE(utils::pop(v, -3));
	BOOST_CHECK_EQUAL(4u, v.size());
	BOOST_CHECK_EQUAL(v[0], 9);
	BOOST_CHECK_EQUAL(v[1], 1);
	BOOST_CHECK_EQUAL(v[2], 0);
	BOOST_CHECK_EQUAL(v[3], 12);
}

BOOST_AUTO_TEST_CASE(algorithm_pop_last) {
	std::vector<int> v{9, 1, -3, 12, 0};
	BOOST_REQUIRE(utils::pop(v, 0));
	BOOST_CHECK_EQUAL(4u, v.size());
	BOOST_CHECK_EQUAL(v[0], 9);
	BOOST_CHECK_EQUAL(v[1], 1);
	BOOST_CHECK_EQUAL(v[2], -3);
	BOOST_CHECK_EQUAL(v[3], 12);
}

BOOST_AUTO_TEST_CASE(algorithm_stably_pop_first) {
	std::vector<int> v{9, 1, -3, 12, 0};
	BOOST_REQUIRE(utils::pop(v, 9, true));
	BOOST_CHECK_EQUAL(4u, v.size());
	BOOST_CHECK_EQUAL(v[0], 1);
	BOOST_CHECK_EQUAL(v[1], -3);
	BOOST_CHECK_EQUAL(v[2], 12);
	BOOST_CHECK_EQUAL(v[3], 0);
}

BOOST_AUTO_TEST_CASE(algorithm_stably_pop_some) {
	std::vector<int> v{9, 1, -3, 12, 0};
	BOOST_REQUIRE(utils::pop(v, -3, true));
	BOOST_CHECK_EQUAL(4u, v.size());
	BOOST_CHECK_EQUAL(v[0], 9);
	BOOST_CHECK_EQUAL(v[1], 1);
	BOOST_CHECK_EQUAL(v[2], 12);
	BOOST_CHECK_EQUAL(v[3], 0);
}

BOOST_AUTO_TEST_CASE(algorithm_stably_pop_last) {
	std::vector<int> v{9, 1, -3, 12, 0};
	BOOST_REQUIRE(utils::pop(v, 0, true));
	BOOST_CHECK_EQUAL(4u, v.size());
	BOOST_CHECK_EQUAL(v[0], 9);
	BOOST_CHECK_EQUAL(v[1], 1);
	BOOST_CHECK_EQUAL(v[2], -3);
	BOOST_CHECK_EQUAL(v[3], 12);
}

BOOST_AUTO_TEST_CASE(algorithm_append_an_empty_array) {
	std::vector<int> u{3, 19, 0, 5};
	std::vector<int> v;
	utils::append(u, v);
	BOOST_CHECK_EQUAL(4u, u.size());
	BOOST_CHECK_EQUAL(3, u[0]);
	BOOST_CHECK_EQUAL(19, u[1]);
}

BOOST_AUTO_TEST_CASE(algorithm_append_to_empty_array) {
	std::vector<int> u{3, 19, 0, 5};
	std::vector<int> v;
	utils::append(v, u);
	BOOST_CHECK_EQUAL(4u, v.size());
	BOOST_CHECK_EQUAL(3, v[0]);
	BOOST_CHECK_EQUAL(19, v[1]);
}

BOOST_AUTO_TEST_CASE(algorithm_append_common_case) {
	std::vector<int> u{3, 19, 0, 5};
	std::vector<int> v{5, 1, -3, 12, 0, 9};

	utils::append(v, u);
	BOOST_REQUIRE_EQUAL(10u, v.size());
	BOOST_CHECK_EQUAL(5, v[0]);
	BOOST_CHECK_EQUAL(9, v[5]);
	BOOST_CHECK_EQUAL(3, v[6]);
	BOOST_CHECK_EQUAL(0, v[8]);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(algorithm_remove_if_existing_elements) {
	std::vector<int> v{5, 1, 3, 12, 0, 9};

	utils::remove_if(v, [](int i) { return i <= 1; });
	BOOST_REQUIRE_EQUAL(4u, v.size());
	BOOST_CHECK_EQUAL(v[0], 5);
	BOOST_CHECK_EQUAL(v[1], 3);
	BOOST_CHECK_EQUAL(v[2], 12);
	BOOST_CHECK_EQUAL(v[3], 9);
}

BOOST_AUTO_TEST_CASE(algorithm_remove_if_missing_elements) {
	std::vector<int> v{5, 1, -3, 12, 0, 9};

	utils::remove_if(v, [](int i) { return i <= -4; });
	BOOST_REQUIRE_EQUAL(6u, v.size());
	BOOST_CHECK_EQUAL(-3, v[2]);
}

BOOST_AUTO_TEST_CASE(algorithm_remove_if_can_drop_all_elements_but_first) {
	std::vector<int> v{5, 1, -3, 12, 0, 9};

	utils::remove_if(v, [](int i) { return i != 5; });
	BOOST_REQUIRE_EQUAL(1u, v.size());
	BOOST_CHECK_EQUAL(5, v[0]);
}

BOOST_AUTO_TEST_CASE(algorithm_remove_if_can_drop_last_element) {
	std::vector<int> v{5, 1};

	utils::remove_if(v, [](int i) { return i == 1; });
	BOOST_REQUIRE_EQUAL(1u, v.size());
	BOOST_CHECK_EQUAL(5, v[0]);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(algorithm_string_split_traverses_all_substrings) {
	std::string str{"hello world,\nthis is an example\nfor splitting"};
	std::vector<std::string> subs;
	utils::split(str, "\n", [&](std::string const & sub) {
		subs.push_back(sub);
	});
	BOOST_REQUIRE_EQUAL(subs.size(), 3u);
	BOOST_CHECK_EQUAL(subs[0], "hello world,");
	BOOST_CHECK_EQUAL(subs[1], "this is an example");
	BOOST_CHECK_EQUAL(subs[2], "for splitting");
}

BOOST_AUTO_TEST_CASE(algorithm_string_works_if_token_not_contained) {
	std::string str{"hello world"};
	std::vector<std::string> subs;
	utils::split(str, "\n", [&](std::string const & sub) {
		subs.push_back(sub);
	});
	BOOST_REQUIRE_EQUAL(subs.size(), 1u);
	BOOST_CHECK_EQUAL(subs[0], "hello world");
}

BOOST_AUTO_TEST_SUITE_END()
