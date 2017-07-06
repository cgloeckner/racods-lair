#include <boost/test/unit_test.hpp>

#include <utils/priority_queue.hpp>

struct HashHelper {
	HashHelper() {}

	std::size_t range() const { return 10u; }

	std::size_t operator()(unsigned int i) const { return i % 10u; }
};

using TestQueue = utils::PriorityQueue<unsigned int, float, HashHelper>;

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(PriorityQueue_test)

BOOST_AUTO_TEST_CASE(PriorityQueue_constructed_empty) {
	TestQueue queue{HashHelper{}};
	BOOST_CHECK(queue.empty());
}

BOOST_AUTO_TEST_CASE(PriorityQueue_insert_nonempty) {
	TestQueue queue{HashHelper{}};
	queue.insert(17u, 0.f);
	BOOST_CHECK(!queue.empty());
}

BOOST_AUTO_TEST_CASE(PriorityQueue_extract_last_element) {
	TestQueue queue{HashHelper{}};
	queue.insert(17u, 0.f);
	BOOST_CHECK_EQUAL(17u, queue.extract());
}

BOOST_AUTO_TEST_CASE(PriorityQueue_extract_last_then_empty) {
	TestQueue queue{HashHelper{}};
	queue.insert(17u, 0.f);
	queue.extract();
	BOOST_CHECK(queue.empty());
}

BOOST_AUTO_TEST_CASE(PriorityQueue_clear_makes_empty) {
	TestQueue queue{HashHelper{}};

	queue.insert(5u, 2.f);
	queue.insert(3u, 6.f);
	BOOST_CHECK(!queue.empty());
	queue.clear();
	BOOST_CHECK(queue.empty());
}

BOOST_AUTO_TEST_CASE(PriorityQueue_extract_minsorted) {
	TestQueue queue{HashHelper{}};
	queue.insert(5u, 2.f);
	queue.insert(3u, 6.f);
	queue.insert(10u, -3.4f);
	queue.insert(0u, 0.f);

	BOOST_REQUIRE(!queue.empty());
	BOOST_CHECK_EQUAL(10u, queue.extract());
	BOOST_CHECK_EQUAL(0u, queue.extract());
	BOOST_CHECK_EQUAL(5u, queue.extract());
	BOOST_CHECK_EQUAL(3u, queue.extract());
}

BOOST_AUTO_TEST_CASE(PriorityQueue_decrease_affects_extraction_order) {
	TestQueue queue{HashHelper{}};
	queue.insert(19, -2.5f);
	queue.insert(3, 16.f);
	queue.insert(6, 3.14f);
	queue.insert(12, 0.2f);
	queue.decrease(3, 0.0f);
	queue.decrease(12, -0.5f);

	BOOST_REQUIRE(!queue.empty());
	BOOST_CHECK_EQUAL(19u, queue.extract());
	BOOST_CHECK_EQUAL(12u, queue.extract());
	BOOST_CHECK_EQUAL(3u, queue.extract());
	BOOST_CHECK_EQUAL(6u, queue.extract());
}

BOOST_AUTO_TEST_SUITE_END()
