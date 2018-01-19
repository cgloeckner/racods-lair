#include <boost/test/unit_test.hpp>

#include <utils/math2d.hpp>
#include <utils/pathfinder.hpp>

using FakeEntity = unsigned short;

struct FakeScene {
	sf::Vector2u block_pos;

	FakeScene() : block_pos{0u, 0u} {}

	float getDistance(sf::Vector2u const& u, sf::Vector2u const& v) const {
		auto dx = utils::distance(u.x, v.x);
		auto dy = utils::distance(u.y, v.y);
		auto max = std::max(dx, dy);
		auto min = std::min(dx, dy);
		// max-min		: straight distance
		// min * 1.414f	: diagonale distance
		return (max - min) + min * 1.414f;
	}
	sf::Vector2u getSize() const { return {10u, 10u}; }
	std::vector<sf::Vector2u> getNeighbors(
		FakeEntity, sf::Vector2u const& pos,
		std::vector<FakeEntity> const& ignore) const {
		std::vector<sf::Vector2u> neighbors;
		sf::Vector2i delta;
		for (delta.y = -1; delta.y <= 1; ++delta.y) {
			for (delta.x = -1; delta.x <= 1; ++delta.x) {
				if (delta.x == 0 && delta.y == 0) {
					continue;
				}
				auto next = sf::Vector2u{sf::Vector2i{pos} + delta};

				// check for grid violation and collision
				if (next.x >= 10u || next.y >= 10u) {
					continue;
				}

				/*
				##########
				#........#
				#....#...#
				#....#...#
				#....#...#
				#....#...#
				#....#...#
				###..#...#
				#.#......#
				##########
				*/
				bool tile_collision =
					(next.x == 0u || next.x == 9u || next.y == 0u ||
						next.y == 9u ||
						(next.x == 5u && next.y >= 2u && next.y <= 7u) ||
						(next.x == 1u && next.y == 7u) ||
						(next.x == 2u && next.y == 7u) ||
						(next.x == 2u && next.y == 8u));
				bool obj_collision = next == block_pos;
				if (!tile_collision && !obj_collision) {
					neighbors.push_back(next);
				}
			}
		}
		return neighbors;
	}
};

void printPath(utils::Path const &path) {
	std::cout << "----\n";
	for (auto const & p: path) {
		std::cout << p.x << "," << p.y << "\t";
	}
	std::cout << "\n----\n";
}

using Testfinder = utils::Pathfinder<FakeScene, FakeEntity>;

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Pathfinder_test)

BOOST_AUTO_TEST_CASE(Pathfinder_trivial_path) {
	FakeScene scene;
	Testfinder pathfinder{scene};

	/*
	##########
	#........#
	#.S..#...#
	#.T..#...#
	#....#...#
	#....#...#
	#....#...#
	###..#...#
	#.#......#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {2u, 3u}, 20u);
	std::vector<sf::Vector2u> predicted{{2u, 3u}, {2u, 2u}};
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_CASE(Pathfinder_simple_path) {
	FakeScene scene;
	Testfinder pathfinder{scene};

	/*
	##########
	#........#
	#.S..#...#
	#.x..#...#
	#.x..#...#
	#..x.#...#
	#..T.#...#
	###..#...#
	#.#......#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {3u, 6u}, 20u);
	std::vector<sf::Vector2u> predicted{
		{3u, 6u}, {3u, 5u}, {2u, 4u}, {2u, 3u}, {2u, 2u}};
	
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_CASE(Pathfinder_invalid_start_pos) {
	FakeScene scene;
	Testfinder pathfinder{scene};

	auto path = pathfinder(1, {12u, 2u}, {3u, 6u}, 20u);
	// source is out of map
	std::vector<sf::Vector2u> predicted{{12u, 2u}};
	
	BOOST_CHECK(path == predicted);
}

BOOST_AUTO_TEST_CASE(Pathfinder_invalid_target_pos) {
	FakeScene scene;
	Testfinder pathfinder{scene};

	/*
	##########
	#....x...#
	#.Sxx#x..#
	#....#.x.#
	#....#..x#
	#....#..x#
	#....#..x#
	###..#...#
	#.#......#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {11u, 6u}, 20u);
	// target is out of map
	std::vector<sf::Vector2u> predicted{{8u, 6u}, {8u, 5u}, {8u, 4u}, {7u, 3u},
		{6u, 2u}, {5u, 1u}, {4u, 2u}, {3u, 2u}, {2u, 2u}};
	
	BOOST_CHECK(path == predicted);
}

BOOST_AUTO_TEST_CASE(Pathfinder_avoid_walls) {
	FakeScene scene;
	Testfinder pathfinder{scene};

	/*
	##########
	#........#
	#.S..#...#
	#..x.#...#
	#..x.#...#
	#...x#...#
	#...x#...#
	###.x#xT.#
	#.#..x...#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {7u, 7u}, 20u);
	std::vector<sf::Vector2u> predicted{{7u, 7u}, {6u, 7u}, {5u, 8u}, {4u, 7u},
		{4u, 6u}, {4u, 5u}, {3u, 4u}, {3u, 3u}, {2u, 2u}};
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_CASE(Pathfinder_target_is_wall) {
	FakeScene scene;
	Testfinder pathfinder{scene};

	/*
	##########
	#........#
	#.S..#...#
	#.x..#...#
	#.x..#...#
	#..x.#...#
	#...x#...#
	###.@T...#
	#.#......#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {5u, 7u}, 20u);
	std::vector<sf::Vector2u> predicted{{4u, 7u}, {4u, 6u}, {3u, 5u},
		{2u, 4u}, {2u, 3u}, {2u, 2u}};
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_CASE(Pathfinder_target_is_object) {
	FakeScene scene;
	Testfinder pathfinder{scene};
	scene.block_pos = {3u, 5u};

	/*
	##########
	#........#
	#.S..#...#
	#.x..#...#
	#..x.#...#
	#..@.#...#
	#....#...#
	###..#...#
	#.#......#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {3u, 5u}, 20u);
	std::vector<sf::Vector2u> predicted{{3u, 4u}, {2u, 3u}, {2u, 2u}};
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_CASE(Pathfinder_target_finds_alternative_to_unreachable_target) {
	FakeScene scene;
	Testfinder pathfinder{scene};

	/*
	##########
	#........#
	#.S..#...#
	#x...#...#
	#x...#...#
	#x...#...#
	#@...#...#
	###..#...#
	#T#......#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {1u, 8u}, 20u);
	std::vector<sf::Vector2u> predicted{
		{1u, 6u}, {1u, 5u}, {1u, 4u}, {1u, 3u}, {2u, 2u}};
	
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_CASE(Pathfinder_avoid_objects) {
	FakeScene scene;
	Testfinder pathfinder{scene};
	scene.block_pos = {2u, 4u};

	/*
	##########
	#........#
	#.S..#...#
	#.x..#...#
	#xB..#...#
	#x...#...#
	#.T..#...#
	###..#...#
	#.#......#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {2u, 6u}, 20u);
	std::vector<sf::Vector2u> predicted{
		{2u, 6u}, {1u, 5u}, {1u, 4u}, {2u, 3u}, {2u, 2u}};
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_CASE(Pathfinder_avoid_walls_and_objects) {
	FakeScene scene;
	Testfinder pathfinder{scene};
	scene.block_pos = {5u, 8u};

	/*
	##########
	#...xx...#
	#.Sx.#x..#
	#....#.x.#
	#....#.x.#
	#....#.x.#
	#....#.x.#
	###..#.T.#
	#.#..B...#
	##########
	*/
	auto path = pathfinder(1, {2u, 2u}, {7u, 7u}, 20u);
	std::vector<sf::Vector2u> predicted{{7u, 7u}, {7u, 6u}, {7u, 5u}, {7u, 4u},
		{7u, 3u}, {6u, 2u}, {5u, 1u}, {4u, 1u}, {3u, 2u}, {2u, 2u}};
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_CASE(Pathfinder_avoid_walls_and_objects_with_maxlength) {
	FakeScene scene;
	Testfinder pathfinder{scene};
	scene.block_pos = {5u, 8u};

	/*
	##########
	#........#
	#....#...#
	#....#...#
	#....#...#
	#....#...#
	#.S..#...#
	###xx#.T.#
	#.#..B...#
	##########
	*/
	auto path = pathfinder(1, {2u, 6u}, {7u, 7u}, 10u);
	std::vector<sf::Vector2u> predicted{{4u, 7u}, {3u, 7u}, {2u, 6u}};
	BOOST_CHECK(predicted == path);
}

BOOST_AUTO_TEST_SUITE_END()
