#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/unionfind.hpp>
#include <game/generator.hpp>

struct GeneratorFixture {
	core::LogContext log;
	
	game::RoomTemplate room;
	game::DungeonGenerator generator;
	
	GeneratorFixture()
		: log{}
		, room{}
		, generator{log} {
		generator.rooms.push_back(&room);
	}
	
	void reset() {
		generator.clear();
		generator.settings = game::GeneratorSettings{};
	}
};

// --------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(generator_test)

BOOST_AUTO_TEST_CASE(layout_contains_some_nodes) {
	auto& fix = Singleton<GeneratorFixture>::get();
	fix.reset();
	
	auto const & data = fix.generator.generate(1u, {256u, 272u});
	auto layout_size = data.graph.getSize();

	auto i = 0u;
	auto num_nodes = static_cast<std::size_t>(
		std::ceil(layout_size.x * layout_size.y * (fix.generator.settings.room_density + fix.generator.settings.deadend_density)));
	sf::Vector2u pos;
	for (pos.y = 0u; pos.y < layout_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < layout_size.x; ++pos.x) {
			if (data.graph.getNode(pos) != nullptr) {
				++i;
			}
		}
	}
	BOOST_CHECK_EQUAL(i, num_nodes);
}

BOOST_AUTO_TEST_CASE(layout_contains_at_least_one_node) {
	auto& fix = Singleton<GeneratorFixture>::get();
	fix.reset();
	
	fix.generator.settings.room_density = 0.001f;
	fix.generator.settings.deadend_density = 0.f;
	auto const & data = fix.generator.generate(1u, {256u, 272u});
	auto layout_size = data.graph.getSize();

	auto i = 0u;
	sf::Vector2u pos;
	for (pos.y = 0u; pos.y < layout_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < layout_size.x; ++pos.x) {
			if (data.graph.getNode(pos) != nullptr) {
				++i;
			}
		}
	}
	BOOST_CHECK_EQUAL(i, 1u);
}

BOOST_AUTO_TEST_CASE(layout_be_fully_meshed_by_rooms) {
	auto& fix = Singleton<GeneratorFixture>::get();
	fix.reset();
	
	fix.generator.settings.room_density = 1.f;
	fix.generator.settings.deadend_density = 0.f;
	auto const & data = fix.generator.generate(1u, {256u, 272u});
	auto layout_size = data.graph.getSize();

	auto i = 0u;
	sf::Vector2u pos;
	for (pos.y = 0u; pos.y < layout_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < layout_size.x; ++pos.x) {
			if (data.graph.getNode(pos) != nullptr) {
				++i;
			}
		}
	}
	BOOST_CHECK_EQUAL(i, layout_size.x * layout_size.y);
}

BOOST_AUTO_TEST_CASE(layout_can_be_fully_meshed_by_rooms_and_deadends) {
	auto& fix = Singleton<GeneratorFixture>::get();
	fix.reset();
	
	fix.generator.settings.room_density = 0.6f;
	fix.generator.settings.deadend_density = 0.4f;
	auto const & data = fix.generator.generate(1u, {256u, 272u});
	auto layout_size = data.graph.getSize();

	auto i = 0u;
	sf::Vector2u pos;
	for (pos.y = 0u; pos.y < layout_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < layout_size.x; ++pos.x) {
			if (data.graph.getNode(pos) != nullptr) {
				++i;
			}
		}
	}
	BOOST_CHECK_EQUAL(i, layout_size.x * layout_size.y);
}
 
BOOST_AUTO_TEST_CASE(all_rooms_and_deadends_are_connected) {
	auto& fix = Singleton<GeneratorFixture>::get();
	fix.reset();
	
	auto const & data = fix.generator.generate(1u, {256u, 272u});
	auto layout_size = data.graph.getSize();

	// prepare cycle test
	utils::Unionfind cycle_test;
	std::vector<utils::Unionfind::Set*> lookup;
	lookup.resize(layout_size.x * layout_size.y, nullptr);
	sf::Vector2u pos;
	for (pos.y = 0u; pos.y < layout_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < layout_size.x; ++pos.x) {
			auto ptr = data.graph.getNode(pos);
			if (ptr == nullptr) {
				continue;
			}
			lookup[pos.x + pos.y * layout_size.x] = &cycle_test.make();
		}
	}
	for (pos.y = 0u; pos.y < layout_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < layout_size.x; ++pos.x) {
			auto ptr = data.graph.getNode(pos);
			if (ptr == nullptr) {
				continue;
			}
			auto& origin =
				cycle_test.find(*lookup[pos.x + pos.y * layout_size.x]);
			for (auto next : ptr->paths) {
				BOOST_REQUIRE_LT(next->offset.x, layout_size.x);
				BOOST_REQUIRE_LT(next->offset.y, layout_size.y);
				auto& target = cycle_test.find(
					*lookup[next->offset.x + next->offset.y * layout_size.x]);
				if (&origin != &target) {
					// connect
					cycle_test.join(origin, target);
				}
			}
		}
	}

	// perform cycle tests
	utils::Unionfind::Set* parent{nullptr};
	for (pos.y = 0u; pos.y < layout_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < layout_size.x; ++pos.x) {
			auto ptr = data.graph.getNode(pos);
			if (ptr == nullptr) {
				continue;
			}
			auto& set = cycle_test.find(*lookup[pos.x + pos.y * layout_size.x]);
			if (parent == nullptr) {
				parent = &set;
			} else if (parent != &set) {
				BOOST_FAIL("<" + std::to_string(pos.x) + "," +
						   std::to_string(pos.y) + "> is isolated");
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(paths_connect_nodes) {
	auto& fix = Singleton<GeneratorFixture>::get();
	fix.reset();
	
	auto const & data = fix.generator.generate(1u, {256u, 272u});

	for (auto const& p : data.builder.paths) {
		sf::Vector2u origin, target;
		origin.x = p.origin.x / fix.generator.settings.cell_size;
		origin.y = p.origin.y / fix.generator.settings.cell_size;
		target.x = p.target.x / fix.generator.settings.cell_size;
		target.y = p.target.y / fix.generator.settings.cell_size;
		auto origin_ptr = data.graph.getNode(origin);
		auto target_ptr = data.graph.getNode(target);

		BOOST_REQUIRE_VECTOR_EQUAL(origin_ptr->offset, origin);
		BOOST_REQUIRE_VECTOR_EQUAL(target_ptr->offset, target);
	}
}

BOOST_AUTO_TEST_CASE(can_generate_dungeons_of_various_sizes) {
	auto& fix = Singleton<GeneratorFixture>::get();
	fix.reset();
	
	utils::SceneID id{1u};
	for (auto y = 20u; y < 1000u; y *= 2) {
		for (auto x = 20u; x < 1000u; x *= 2) {
			BOOST_REQUIRE_NO_ASSERT(fix.generator.generate(id++, {x, y}));
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
