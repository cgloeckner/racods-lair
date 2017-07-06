#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/component_system.hpp>

struct TestComponent {
	std::size_t id;
};

using TestManager = utils::IdManager<std::size_t>;
using TestSystem = utils::ComponentSystem<std::size_t, TestComponent>;

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(component_test)

BOOST_AUTO_TEST_CASE(manager_acquire_unused_keys) {
	TestManager manager{10u};
	BOOST_CHECK_EQUAL(1u, manager.acquire());
	BOOST_CHECK_EQUAL(2u, manager.acquire());
	BOOST_CHECK_EQUAL(3u, manager.acquire());
}

BOOST_AUTO_TEST_CASE(manager_release_keys) {
	TestManager manager{10u};
	manager.acquire();  // 1
	manager.acquire();  // 2
	manager.acquire();  // 3
	manager.release(3u);
	manager.release(1u);
	BOOST_CHECK_EQUAL(4u, manager.acquire());
	BOOST_CHECK_EQUAL(5u, manager.acquire());
	BOOST_CHECK_EQUAL(6u, manager.acquire());
}

BOOST_AUTO_TEST_CASE(manager_reacquire_keys) {
	TestManager manager{10u};
	manager.acquire();  // 1
	manager.acquire();  // 2
	manager.acquire();  // 3
	manager.release(3u);
	manager.release(1u);
	manager.cleanup();
	BOOST_CHECK_EQUAL(1u, manager.acquire());
	BOOST_CHECK_EQUAL(3u, manager.acquire());
	BOOST_CHECK_EQUAL(4u, manager.acquire());
}

BOOST_AUTO_TEST_CASE(manager_too_many_keys) {
	TestManager manager{10u};
	for (auto i = 0u; i < 10u; ++i) {
		manager.acquire();
	}
	BOOST_CHECK_THROW(manager.acquire(), std::bad_alloc);
}

BOOST_AUTO_TEST_CASE(manager_too_many_keys_release_require) {
	TestManager manager{10u};
	for (auto i = 0u; i < 10u; ++i) {
		manager.acquire();
	}
	BOOST_CHECK_THROW(manager.acquire(), std::bad_alloc);
	manager.release(5u);
	BOOST_CHECK_THROW(manager.acquire(), std::bad_alloc);
	manager.cleanup();
	BOOST_CHECK_NO_THROW(manager.acquire());
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(component_acquire_valid_key) {
	TestSystem system{10u};
	auto& data = system.acquire(5u);
	BOOST_CHECK_EQUAL(5u, data.id);
}

BOOST_AUTO_TEST_CASE(component_acquire_invalid_key) {
	TestSystem system{10u};
	BOOST_CHECK_ASSERT(system.acquire(0u));
	BOOST_CHECK_ASSERT(system.acquire(11u));
}

BOOST_AUTO_TEST_CASE(component_acquire_valid_key_twice) {
	TestSystem system{10u};
	system.acquire(5u);
	BOOST_CHECK_ASSERT(system.acquire(5u));
}

BOOST_AUTO_TEST_CASE(component_acquire_too_many_keys) {
	TestSystem system{10u};
	for (auto i = 1u; i <= 10u; ++i) {
		system.acquire(i);
	}
	BOOST_CHECK_THROW(system.acquire(5u), std::bad_alloc);
}

BOOST_AUTO_TEST_CASE(component_release_without_cleanup_does_nothing) {
	TestSystem system{10u};
	auto const& const_sys = system;
	system.acquire(7u);
	system.release(7u);
	BOOST_CHECK(system.has(7u));
	BOOST_CHECK(const_sys.has(7u));
	BOOST_CHECK_NO_ASSERT(system.query(7u));
	BOOST_CHECK_NO_ASSERT(const_sys.query(7u));
	BOOST_CHECK_EQUAL(1u, system.size());
	BOOST_CHECK_ASSERT(system.acquire(7u));
	system.cleanup();
	BOOST_CHECK(!system.has(7u));
	BOOST_CHECK(!const_sys.has(7u));
	BOOST_CHECK_ASSERT(system.query(7u));
	BOOST_CHECK_ASSERT(const_sys.query(7u));
	BOOST_CHECK_EQUAL(0u, system.size());
	BOOST_CHECK_NO_ASSERT(system.acquire(7u));
}

BOOST_AUTO_TEST_CASE(component_release_invalid_key) {
	TestSystem system{10u};
	BOOST_CHECK_ASSERT(system.release(0u));
	BOOST_CHECK_ASSERT(system.release(5u));
	BOOST_CHECK_ASSERT(system.release(11u));
}

BOOST_AUTO_TEST_CASE(component_release_valid_key) {
	TestSystem system{10u};
	system.acquire(5u);
	BOOST_CHECK_NO_ASSERT(system.release(5u));
}

BOOST_AUTO_TEST_CASE(component_release_and_cleanup_valid_key_twice) {
	TestSystem system{10u};
	system.acquire(5u);
	BOOST_CHECK_NO_ASSERT(system.release(5u));
	system.cleanup();
	BOOST_CHECK_ASSERT(system.release(5u));
}

BOOST_AUTO_TEST_CASE(component_release_and_cleanup_drops_key) {
	TestSystem system{10u};
	system.acquire(5u);
	BOOST_CHECK_NO_ASSERT(system.release(5u));
	system.cleanup();
	BOOST_CHECK(!system.has(5u));
}

BOOST_AUTO_TEST_CASE(component_reacquire_valid_key) {
	TestSystem system{10u};
	system.acquire(5u);
	BOOST_CHECK_NO_ASSERT(system.release(5u));
	system.cleanup();
	auto& data = system.acquire(5u);
	BOOST_CHECK_EQUAL(5u, data.id);
}

BOOST_AUTO_TEST_CASE(component_hasnt_invalid_key) {
	TestSystem system{10u};
	BOOST_CHECK_ASSERT(system.has(0u));
	BOOST_CHECK(!system.has(5u));
	BOOST_CHECK_ASSERT(system.has(11u));
}

BOOST_AUTO_TEST_CASE(component_has_valid_key) {
	TestSystem system{10u};
	system.acquire(5u);
	BOOST_CHECK(system.has(5u));
}

BOOST_AUTO_TEST_CASE(component_hasnt_key_after_release_and_cleanup) {
	TestSystem system{10u};
	system.acquire(5u);
	system.release(5u);
	system.cleanup();
	BOOST_CHECK(!system.has(5u));
}

BOOST_AUTO_TEST_CASE(component_query_existing_key) {
	TestSystem system{10u};
	auto const& const_sys = system;
	system.acquire(7u);
	BOOST_CHECK_NO_THROW(system.query(7u));
	BOOST_CHECK_NO_THROW(const_sys.query(7u));
	auto& data = system.query(7u);
	auto& const_data = const_sys.query(7u);
	BOOST_CHECK_EQUAL(7u, data.id);
	BOOST_CHECK_EQUAL(7u, const_data.id);
}

BOOST_AUTO_TEST_CASE(component_query_invalid_key) {
	TestSystem system{10u};
	auto const& const_sys = system;
	BOOST_CHECK_ASSERT(system.query(0u));
	BOOST_CHECK_ASSERT(const_sys.query(0u));
	BOOST_CHECK_ASSERT(system.query(7u));
	BOOST_CHECK_ASSERT(const_sys.query(7u));
	BOOST_CHECK_ASSERT(system.query(11u));
	BOOST_CHECK_ASSERT(const_sys.query(11u));
}

BOOST_AUTO_TEST_CASE(component_query_key_after_release_and_cleanup) {
	TestSystem system{10u};
	system.acquire(7u);
	system.release(7u);
	system.cleanup();
	auto const& const_sys = system;
	BOOST_CHECK_ASSERT(system.query(7u));
	BOOST_CHECK_ASSERT(const_sys.query(7u));
}

BOOST_AUTO_TEST_CASE(component_acquire_increases_size) {
	TestSystem system{10u};
	BOOST_CHECK_EQUAL(0u, system.size());
	system.acquire(2u);
	BOOST_CHECK_EQUAL(1u, system.size());
}

BOOST_AUTO_TEST_CASE(component_release_and_cleanup_decreases_size) {
	TestSystem system{10u};
	system.acquire(2u);
	system.release(2u);
	system.cleanup();
	BOOST_CHECK_EQUAL(0u, system.size());
}

BOOST_AUTO_TEST_CASE(component_capacity_always_constant) {
	TestSystem system{10u};
	BOOST_CHECK_EQUAL(10u, system.capacity());
	system.acquire(2u);
	BOOST_CHECK_EQUAL(10u, system.capacity());
	system.release(2u);
	system.cleanup();
	BOOST_CHECK_EQUAL(10u, system.capacity());
}

BOOST_AUTO_TEST_CASE(component_iterate_over_empty_objectset) {
	TestSystem system{10u};
	auto const& const_sys = system;
	BOOST_CHECK(system.begin() == system.end());
	BOOST_CHECK(const_sys.begin() == const_sys.end());
}

BOOST_AUTO_TEST_CASE(component_iterate_over_existing_objectset) {
	TestSystem system{10u};
	auto const& const_sys = system;
	system.acquire(2u);
	system.acquire(1u);
	auto i = system.begin();
	BOOST_CHECK_EQUAL(2u, i->id);
	++i;
	BOOST_CHECK_EQUAL(1u, i->id);
	++i;
	BOOST_CHECK(i == system.end());
	auto ci = const_sys.begin();
	BOOST_CHECK_EQUAL(2u, ci->id);
	++ci;
	BOOST_CHECK_EQUAL(1u, ci->id);
	++ci;
	BOOST_CHECK(i == const_sys.end());
}

BOOST_AUTO_TEST_SUITE_END()
