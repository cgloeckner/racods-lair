#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/algorithm.hpp>
#include <game/builder.hpp>

struct BuilderFixture {
	sf::Texture dummy;
	rpg::TilesetTemplate tileset;
	core::Dungeon dungeon;
	game::BuildSettings settings;

	BuilderFixture()
		: dummy{}
		, tileset{}
		, dungeon{1u, dummy, {15u, 15u}, {64.f, 64.f}}
		, settings{} {
		tileset.tilesize = {64u, 64u};
		tileset.floors.resize(1u);
		tileset.walls.resize(1u);
	}

	void reset() {
		for (auto y = 0u; y < 15u; ++y) {
			for (auto x = 0u; x < 15u; ++x) {
				auto& cell = dungeon.getCell({x, y});
				cell.terrain = core::Terrain::Void;
				cell.tile = utils::OrthoTile{};
			}
		}
		settings = game::BuildSettings{};
		settings.cell_size = 15u;
		settings.path_width = 1u;
		settings.random_transform = false;
	}

	std::string print() const {
		std::string s;
		for (auto y = 0u; y < 15u; ++y) {
			for (auto x = 0u; x < 15u; ++x) {
				switch (dungeon.getCell({x, y}).terrain) {
					case core::Terrain::Void:
						s += ".";
						break;
					case core::Terrain::Wall:
						s += "#";
						break;
					case core::Terrain::Floor:
						s += "~";
						break;
				}
			}
			s += "\n";
		}
		return s;
	}
};

BOOST_AUTO_TEST_SUITE(builder_test)

BOOST_AUTO_TEST_CASE(void_with_void_neighbors_will_not_be_wall) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	BOOST_CHECK(!game::dungeon_impl::shouldBeWall(fix.dungeon, {0u, 0u}));
	BOOST_CHECK(!game::dungeon_impl::shouldBeWall(fix.dungeon, {5u, 3u}));
}

BOOST_AUTO_TEST_CASE(floor_tile_will_not_be_wall) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	fix.dungeon.getCell({2u, 3u}).terrain = core::Terrain::Floor;
	BOOST_CHECK(!game::dungeon_impl::shouldBeWall(fix.dungeon, {2u, 3u}));
}

BOOST_AUTO_TEST_CASE(void_tile_with_floor_neighbor_will_be_wall) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	fix.dungeon.getCell({2u, 3u}).terrain = core::Terrain::Floor;
	BOOST_CHECK(game::dungeon_impl::shouldBeWall(fix.dungeon, {2u, 2u}));
}

BOOST_AUTO_TEST_CASE(void_tile_with_floor_and_wall_neighbor_will_be_wall) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	fix.dungeon.getCell({2u, 3u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Wall;
	BOOST_CHECK(game::dungeon_impl::shouldBeWall(fix.dungeon, {2u, 2u}));
}

BOOST_AUTO_TEST_CASE(void_tile_with_only_and_wall_neighbor_will_not_be_wall) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Wall;
	BOOST_CHECK(!game::dungeon_impl::shouldBeWall(fix.dungeon, {2u, 2u}));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(void_tile_will_cause_shading) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	BOOST_CHECK(
		game::dungeon_impl::shouldBeShaded(fix.dungeon, {2u, 2u}, {-1, -1}));
}

BOOST_AUTO_TEST_CASE(floor_tile_will_not_cause_shading) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Floor;
	BOOST_CHECK(
		!game::dungeon_impl::shouldBeShaded(fix.dungeon, {2u, 2u}, {-1, -1}));
}

BOOST_AUTO_TEST_CASE(wall_tile_will_not_cause_shading) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	BOOST_CHECK(
		!game::dungeon_impl::shouldBeShaded(fix.dungeon, {2u, 2u}, {-1, -1}));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(tile_can_be_topleft_edge_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	.	#	~
	//	#	X	~
	//	~	~	~
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({0u, 2u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 2u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading, utils::ShadeTopLeft);
}

BOOST_AUTO_TEST_CASE(tile_can_be_topright_edge_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	~	#	.
	//	~	X	#
	//	~	~	~
	fix.dungeon.getCell({0u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 2u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 2u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading, utils::ShadeTopRight);
}

BOOST_AUTO_TEST_CASE(tile_can_be_bottomright_edge_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	~	~	~
	//	~	X	#
	//	~	#	.
	fix.dungeon.getCell({0u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 2u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Wall;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading, utils::ShadeBottomRight);
}

BOOST_AUTO_TEST_CASE(tile_can_be_bottomleft_edge_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	~	~	~
	//	#	X	~
	//	.	#	~
	fix.dungeon.getCell({0u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 2u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading, utils::ShadeBottomLeft);
}

BOOST_AUTO_TEST_CASE(tile_can_be_topleft_corner_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	.	.	.
	//	.	X	#
	//	.	#	~
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 2u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading,
		utils::ShadeTopLeft | utils::ShadeTopRight | utils::ShadeBottomLeft);
}

BOOST_AUTO_TEST_CASE(tile_can_be_topright_corner_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	.	.	.
	//	#	X	.
	//	~	#	.
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 2u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading,
		utils::ShadeTopRight | utils::ShadeTopLeft | utils::ShadeBottomRight);
}

BOOST_AUTO_TEST_CASE(tile_can_be_bottomright_corner_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	~	#	.
	//	#	X	.
	//	.	.	.
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 0u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading, utils::ShadeBottomRight |
								   utils::ShadeBottomLeft |
								   utils::ShadeTopRight);
}

BOOST_AUTO_TEST_CASE(tile_can_be_bottomleft_corner_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	.	#	~
	//	.	X	#
	//	.	.	.
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 0u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading,
		utils::ShadeBottomLeft | utils::ShadeBottomRight | utils::ShadeTopLeft);
}

BOOST_AUTO_TEST_CASE(tile_can_be_top_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	.	.	.
	//	#	X	#
	//	~	~	~
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({0u, 2u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 2u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading, utils::ShadeTopLeft | utils::ShadeTopRight);
}

BOOST_AUTO_TEST_CASE(tile_can_be_bottom_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	~	~	~
	//	#	X	#
	//	.	.	.
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({0u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 0u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(
		shading, utils::ShadeBottomLeft | utils::ShadeBottomRight);
}

BOOST_AUTO_TEST_CASE(tile_can_be_right_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	~	#	.
	//	~	X	.
	//	~	#	.
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({0u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({0u, 1u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({0u, 2u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading, utils::ShadeTopRight | utils::ShadeBottomRight);
}

BOOST_AUTO_TEST_CASE(tile_can_be_left_shaded) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	//	.	#	~
	//	.	X	~
	//	.	#	~
	fix.dungeon.getCell({1u, 0u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 1u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({1u, 2u}).terrain = core::Terrain::Wall;
	fix.dungeon.getCell({2u, 0u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 1u}).terrain = core::Terrain::Floor;
	fix.dungeon.getCell({2u, 2u}).terrain = core::Terrain::Floor;
	auto shading = game::dungeon_impl::getShadingCase(fix.dungeon, {1u, 1u});

	BOOST_CHECK_EQUAL(shading, utils::ShadeTopLeft | utils::ShadeBottomLeft);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(
	room_not_valid_if_left_and_width_intersect_with_dungeons_right_border) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({300u, 2u});
	game::RoomBuilder room{5u, 3u, tpl};
	BOOST_CHECK(!room.isValid(fix.dungeon.getSize()));
}

BOOST_AUTO_TEST_CASE(
	room_not_valid_if_top_and_height_intersect_with_dungeons_bottom_border) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({3u, 350u});
	game::RoomBuilder room{2u, 5u, tpl};
	BOOST_CHECK(!room.isValid(fix.dungeon.getSize()));
}

BOOST_AUTO_TEST_CASE(
	create_room_will_fill_inner_with_floor_tiles_and_leave_out_border_and_outside) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({1u, 1u});
	tpl.create({2u, 1u});
	tpl.create({3u, 1u});
	tpl.create({1u, 2u});
	game::RoomBuilder room{0u, 0u, tpl};
	BOOST_CHECK_NO_ASSERT(room(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += ".~~~...........\n";
	out += ".~.............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(room_can_be_rotated_by_90_degree) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({1u, 1u});
	tpl.create({2u, 1u});
	tpl.create({3u, 1u});
	tpl.create({1u, 2u});
	game::RoomBuilder room{0u, 0u, tpl};
	room.angle = 90.f;
	BOOST_CHECK_NO_ASSERT(room(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += "............~~.\n";
	out += ".............~.\n";
	out += ".............~.\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(room_can_be_rotated_by_180_degree) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({1u, 1u});
	tpl.create({2u, 1u});
	tpl.create({3u, 1u});
	tpl.create({1u, 2u});
	game::RoomBuilder room{0u, 0u, tpl};
	room.angle = 180.f;
	BOOST_CHECK_NO_ASSERT(room(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += ".............~.\n";
	out += "...........~~~.\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(room_can_be_rotated_by_270_degree) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({1u, 1u});
	tpl.create({2u, 1u});
	tpl.create({3u, 1u});
	tpl.create({1u, 2u});
	game::RoomBuilder room{0u, 0u, tpl};
	room.angle = 270.f;
	BOOST_CHECK_NO_ASSERT(room(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += ".~.............\n";
	out += ".~.............\n";
	out += ".~~............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}


BOOST_AUTO_TEST_CASE(room_can_be_flipped_vertically) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({1u, 1u});
	tpl.create({2u, 1u});
	tpl.create({3u, 1u});
	tpl.create({1u, 2u});
	game::RoomBuilder room{0u, 0u, tpl};
	room.flip_x = true;
	BOOST_CHECK_NO_ASSERT(room(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += "...........~~~.\n";
	out += ".............~.\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(room_can_be_flipped_horitzontal) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({1u, 1u});
	tpl.create({2u, 1u});
	tpl.create({3u, 1u});
	tpl.create({1u, 2u});
	game::RoomBuilder room{0u, 0u, tpl};
	room.flip_y = true;
	BOOST_CHECK_NO_ASSERT(room(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += ".~.............\n";
	out += ".~~~...........\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(room_can_be_flipped_both) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({1u, 1u});
	tpl.create({2u, 1u});
	tpl.create({3u, 1u});
	tpl.create({1u, 2u});
	game::RoomBuilder room{0u, 0u, tpl};
	room.flip_x = true;
	room.flip_y = true;
	BOOST_CHECK_NO_ASSERT(room(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += ".............~.\n";
	out += "...........~~~.\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(room_can_be_rotated_and_flipped) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate tpl;
	tpl.create({1u, 1u});
	tpl.create({2u, 1u});
	tpl.create({3u, 1u});
	tpl.create({1u, 2u});
	game::RoomBuilder room{0u, 0u, tpl};
	room.angle = 180.f;
	room.flip_x = true;
	room.flip_y = true;
	BOOST_CHECK_NO_ASSERT(room(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += ".~~~...........\n";
	out += ".~.............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(path_not_valid_if_zero_coordinates) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{0u, 1u}, {3u, 7u}};
	BOOST_CHECK(!path.isValid(fix.dungeon.getSize(), 1u));
	path = game::PathBuilder{{1u, 0u}, {3u, 7u}};
	BOOST_CHECK(!path.isValid(fix.dungeon.getSize(), 1u));
	path = game::PathBuilder{{1u, 3u}, {0u, 7u}};
	BOOST_CHECK(!path.isValid(fix.dungeon.getSize(), 1u));
	path = game::PathBuilder{{1u, 3u}, {7u, 0u}};
	BOOST_CHECK(!path.isValid(fix.dungeon.getSize(), 1u));
}

BOOST_AUTO_TEST_CASE(
	path_not_valid_if_coordinates_plus_width_exceeds_dungeon_borders) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{13u, 1u}, {3u, 7u}};
	BOOST_CHECK(!path.isValid(fix.dungeon.getSize(), 2u));
	path = game::PathBuilder{{1u, 17u}, {3u, 7u}};
	BOOST_CHECK(!path.isValid(fix.dungeon.getSize(), 3u));
	path = game::PathBuilder{{1u, 3u}, {14u, 7u}};
	BOOST_CHECK(!path.isValid(fix.dungeon.getSize(), 1u));
	path = game::PathBuilder{{1u, 3u}, {7u, 18u}};
	BOOST_CHECK(!path.isValid(fix.dungeon.getSize(), 2u));
}

BOOST_AUTO_TEST_CASE(
	path_valid_if_both_points_do_not_collide_with_dungeon_borders_after_width_is_applied) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{10u, 1u}, {3u, 7u}};
	BOOST_CHECK(path.isValid(fix.dungeon.getSize(), 4u));
}

BOOST_AUTO_TEST_CASE(create_path_will_go_horizontally_right_first) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{1u, 1u}, {3u, 4u}};
	fix.settings.path_width = 1u;
	BOOST_CHECK_NO_ASSERT(path(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += ".~~~...........\n";
	out += "...~...........\n";
	out += "...~...........\n";
	out += "...~...........\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(create_path_will_go_horizontally_left_first) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{3u, 1u}, {1u, 4u}};
	fix.settings.path_width = 1u;
	BOOST_CHECK_NO_ASSERT(path(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += ".~~~...........\n";
	out += ".~.............\n";
	out += ".~.............\n";
	out += ".~.............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(
	create_path_extends_horizontal_path_of_even_length_using_width) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{2u, 2u}, {4u, 2u}};
	fix.settings.path_width = 3u;
	BOOST_CHECK_NO_ASSERT(path(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += ".~~~~~.........\n";
	out += ".~~~~~.........\n";
	out += ".~~~~~.........\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(
	create_path_extends_horizontal_path_of_odd_length_using_width) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{2u, 2u}, {5u, 2u}};
	fix.settings.path_width = 3u;
	BOOST_CHECK_NO_ASSERT(path(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += ".~~~~~~........\n";
	out += ".~~~~~~........\n";
	out += ".~~~~~~........\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(
	create_path_extends_vertical_path_of_even_length_using_width) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{2u, 2u}, {2u, 4u}};
	fix.settings.path_width = 3u;
	BOOST_CHECK_NO_ASSERT(path(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

BOOST_AUTO_TEST_CASE(
	create_path_extends_vertical_path_of_odd_length_using_width) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::PathBuilder path{{2u, 2u}, {2u, 5u}};
	fix.settings.path_width = 3u;
	BOOST_CHECK_NO_ASSERT(path(fix.dungeon, fix.settings));

	std::string out;
	out += "...............\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += ".~~~...........\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";
	out += "...............\n";

	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected");
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(corridors_do_not_replace_inner_wall_of_rooms) {
	auto& fix = Singleton<BuilderFixture>::get();
	fix.reset();

	game::RoomTemplate room;
	room.create({0u, 1u}).wall = true;
	room.create({1u, 1u});
	room.create({2u, 1u}).wall = true;
	
	game::DungeonBuilder builder({15u, 15u});
	builder.rooms.emplace_back(0u, 0u, room);
	builder.rooms.emplace_back(4u, 0u, room);
	builder.paths.emplace_back(1u, 1u, 5u, 1u);
	fix.settings.path_width = 1u;
	
	builder(fix.tileset, fix.dungeon, fix.settings);
	
	std::string out =
		"#######........\n"
		"#~#~#~#........\n"
		"#######........\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n"
		"...............\n";
	
	auto s = fix.print();
	if (s != out) {
		std::cout << s << "\n\n" << out << "\n";
		BOOST_FAIL("Dungeon was not populated as expected\n");
	}
}

BOOST_AUTO_TEST_SUITE_END()
