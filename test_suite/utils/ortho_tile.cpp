#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

#include <utils/assert.hpp>
#include <utils/ortho_tile.hpp>

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(tile_test)

BOOST_AUTO_TEST_CASE(tile_refresh_invalid_scale) {
	utils::OrthoTile tile;
	BOOST_CHECK_ASSERT(
		tile.refresh({12u, 5u}, {0u, 2u}, {1u, 2u}, {48u, 30u}));
	BOOST_CHECK_ASSERT(
		tile.refresh({12u, 5u}, {1u, 0u}, {1u, 2u}, {48u, 30u}));
	BOOST_CHECK_ASSERT(
		tile.refresh({12u, 5u}, {3u, 2u}, {1u, 2u}, {48u, 30u}));
	BOOST_CHECK_ASSERT(
		tile.refresh({12u, 5u}, {2u, 3u}, {1u, 2u}, {48u, 30u}));
}

BOOST_AUTO_TEST_CASE(tile_refresh_invalid_tile_size) {
	utils::OrthoTile tile;
	BOOST_CHECK_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {0u, 30u}));
	BOOST_CHECK_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {2u, 0u}));
}

BOOST_AUTO_TEST_CASE(tile_refresh_position) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(
		tile.refresh({12u, 5u}, {4u, 2u}, {1u, 2u}, {48u, 30u}))

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(46.f, 9.f), tile.vertices[0].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(50.f, 9.f), tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(50.f, 11.f), tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(46.f, 11.f), tile.vertices[3].position, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tile_refresh_texcoord) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(51.f, 65.f), tile.vertices[0].texCoords, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(99.f, 65.f), tile.vertices[1].texCoords, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(99.f, 95.f), tile.vertices[2].texCoords, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(51.f, 95.f), tile.vertices[3].texCoords, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tile_refresh_without_edges) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}));

	BOOST_CHECK(tile.edges.empty());
}

BOOST_AUTO_TEST_CASE(tile_refresh_with_edges) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, 0u, true));

	BOOST_REQUIRE_EQUAL(4u, tile.edges.size());
	BOOST_CHECK_VECTOR_CLOSE(
		tile.edges[0].u, tile.vertices[0].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		tile.edges[0].v, tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		tile.edges[1].u, tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		tile.edges[1].v, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		tile.edges[2].u, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		tile.edges[2].v, tile.vertices[3].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		tile.edges[3].u, tile.vertices[3].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		tile.edges[3].v, tile.vertices[0].position, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tile_refresh_unshaded) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::White);
}

BOOST_AUTO_TEST_CASE(tile_refresh_topshaded) {
	utils::OrthoTile tile;
	auto shading = utils::ShadeTopLeft | utils::ShadeTopRight;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::White);
}

BOOST_AUTO_TEST_CASE(tile_refresh_rightshaded) {
	utils::OrthoTile tile;
	auto shading = utils::ShadeTopRight | utils::ShadeBottomRight;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::White);
}

BOOST_AUTO_TEST_CASE(tile_refresh_bottomshaded) {
	utils::OrthoTile tile;
	auto shading = utils::ShadeBottomLeft | utils::ShadeBottomRight;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::Black);
}

BOOST_AUTO_TEST_CASE(tile_refresh_leftshaded) {
	utils::OrthoTile tile;
	auto shading = utils::ShadeTopLeft | utils::ShadeBottomLeft;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::Black);
}

BOOST_AUTO_TEST_CASE(tile_refresh_topleftshaded) {
	utils::OrthoTile tile;
	auto shading = utils::ShadeTopLeft;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(!tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::White);
}

BOOST_AUTO_TEST_CASE(tile_refresh_toprightshaded) {
	utils::OrthoTile tile;
	auto shading = utils::ShadeTopRight;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::White);
}

BOOST_AUTO_TEST_CASE(tile_refresh_bottomrightshaded) {
	utils::OrthoTile tile;
	auto shading = utils::ShadeBottomRight;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(!tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::White);
}

BOOST_AUTO_TEST_CASE(tile_refresh_bottomleftshaded) {
	utils::OrthoTile tile;
	auto shading = utils::ShadeBottomLeft;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::Black);
}

BOOST_AUTO_TEST_CASE(tile_refresh_topleftunshaded) {
	utils::OrthoTile tile;
	auto shading =
		utils::ShadeTopRight | utils::ShadeBottomRight | utils::ShadeBottomLeft;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(!tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::Black);
}

BOOST_AUTO_TEST_CASE(tile_refresh_toprightunshaded) {
	utils::OrthoTile tile;
	auto shading =
		utils::ShadeTopLeft | utils::ShadeBottomRight | utils::ShadeBottomLeft;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::Black);
}

BOOST_AUTO_TEST_CASE(tile_refresh_bottomrightunshaded) {
	utils::OrthoTile tile;
	auto shading =
		utils::ShadeTopLeft | utils::ShadeTopRight | utils::ShadeBottomLeft;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(!tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::White);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::Black);
}

BOOST_AUTO_TEST_CASE(tile_refresh_bottomleftunshaded) {
	utils::OrthoTile tile;
	auto shading =
		utils::ShadeTopLeft | utils::ShadeTopRight | utils::ShadeBottomRight;
	BOOST_REQUIRE_NO_ASSERT(tile.refresh(
		{12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}, shading));

	BOOST_REQUIRE_EQUAL(4u, tile.vertices.size());
	BOOST_CHECK(tile.std_tri);
	BOOST_CHECK(tile.vertices[0].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[1].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[2].color == sf::Color::Black);
	BOOST_CHECK(tile.vertices[3].color == sf::Color::White);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(tile_fetch_fails_if_not_refreshed_before) {
	utils::OrthoTile tile;
	sf::VertexArray vertices;
	BOOST_CHECK_ASSERT(tile.fetchTile(vertices));
}

BOOST_AUTO_TEST_CASE(tile_fetch_entire_tile_with_default_triangulation) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}));
	sf::VertexArray vertices;
	tile.fetchTile(vertices);

	BOOST_REQUIRE_EQUAL(6u, vertices.getVertexCount());
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[0].position, tile.vertices[0].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[1].position, tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[2].position, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[3].position, tile.vertices[0].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[4].position, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[5].position, tile.vertices[3].position, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tile_fetch_entire_tile_with_alternative_triangulation) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}));
	tile.std_tri = false;
	sf::VertexArray vertices;
	tile.fetchTile(vertices);

	BOOST_REQUIRE_EQUAL(6u, vertices.getVertexCount());
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[0].position, tile.vertices[0].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[1].position, tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[2].position, tile.vertices[3].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[3].position, tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[4].position, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[5].position, tile.vertices[3].position, 0.0001f);
}

BOOST_AUTO_TEST_CASE(grid_fetch_fails_if_not_refreshed_before) {
	utils::OrthoTile tile;
	sf::VertexArray vertices;
	BOOST_CHECK_ASSERT(tile.fetchGrid(sf::Color::Red, vertices));
}

BOOST_AUTO_TEST_CASE(tile_fetch_grid_picks_borders_in_given_color) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}));
	sf::VertexArray vertices;
	tile.fetchGrid(sf::Color::Red, vertices);

	BOOST_REQUIRE_EQUAL(8u, vertices.getVertexCount());
	for (auto i = 0u; i < 8u; ++i) {
		BOOST_CHECK(vertices[i].color == sf::Color::Red);
	}
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[0].position, tile.vertices[0].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[1].position, tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[2].position, tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[3].position, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[4].position, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[5].position, tile.vertices[3].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[6].position, tile.vertices[3].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[7].position, tile.vertices[0].position, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tile_fetch_collision_fails_if_not_refreshed_before) {
	utils::OrthoTile tile;
	sf::VertexArray vertices;
	BOOST_CHECK_ASSERT(tile.fetchCollision(sf::Color::Red, vertices));
}

BOOST_AUTO_TEST_CASE(tile_fetch_collision_picks_quad_in_given_color) {
	utils::OrthoTile tile;
	BOOST_REQUIRE_NO_ASSERT(
		tile.refresh({12u, 5u}, {2u, 2u}, {1u, 2u}, {48u, 30u}));
	sf::VertexArray vertices;
	tile.fetchCollision(sf::Color::Red, vertices);

	BOOST_REQUIRE_EQUAL(6u, vertices.getVertexCount());
	for (auto i = 0u; i < 6u; ++i) {
		BOOST_CHECK(vertices[i].color == sf::Color::Red);
	}

	BOOST_CHECK_VECTOR_CLOSE(
		vertices[0].position, tile.vertices[0].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[1].position, tile.vertices[1].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[2].position, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[3].position, tile.vertices[0].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[4].position, tile.vertices[2].position, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		vertices[5].position, tile.vertices[3].position, 0.0001f);
}

BOOST_AUTO_TEST_SUITE_END()
