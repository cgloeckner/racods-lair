#include <algorithm>
#include <SFML/Graphics.hpp>
#include <boost/test/unit_test.hpp>

#include <utils/tiling.hpp>

BOOST_AUTO_TEST_CASE(tiling_iterator_ctor) {
	utils::TilingIterator<utils::GridMode::Orthogonal> i{{2, 1}, {2, 3}};
	utils::TilingIterator<utils::GridMode::Orthogonal> end{{4, 1}, {2, 3}};
	
	auto pos = *i;
	BOOST_CHECK(i != end);
	BOOST_CHECK_EQUAL(pos.x, 2u);
	BOOST_CHECK_EQUAL(pos.y, 1u);
	BOOST_CHECK_EQUAL(i.getRange().x, 2);
	BOOST_CHECK_EQUAL(i.getRange().y, 3);
	BOOST_CHECK_EQUAL(i.getRange().x, end.getRange().x);
	BOOST_CHECK_EQUAL(i.getRange().y, end.getRange().y);
}

BOOST_AUTO_TEST_CASE(tiling_ortho_iterator_step) {
	utils::TilingIterator<utils::GridMode::Orthogonal> i{{2, 1}, {2, 3}};
	sf::Vector2i range;
	range = i.getRange();
	
	// step to the right
	++i;
	auto pos = *i;
	BOOST_CHECK_EQUAL(pos.x, 3u);
	BOOST_CHECK_EQUAL(pos.y, 1u);
	BOOST_CHECK_EQUAL(i.getRange().x, range.x);
	BOOST_CHECK_EQUAL(i.getRange().y, range.y);
	
	// step to the top of the next line
	++i;
	pos = *i;
	BOOST_CHECK_EQUAL(pos.x, 2u);
	BOOST_CHECK_EQUAL(pos.y, 2u);
}

BOOST_AUTO_TEST_CASE(tiling_ortho_view) {
	// note: this behavior is gridmode-independent
	utils::Tiling<utils::GridMode::Orthogonal> tiling{{48.f, 32.f}};
	sf::View view{{200, 200, 300, 200}};
	tiling.setView(view);
	
	BOOST_CHECK_CLOSE(48.f, tiling.getTileSize().x, 0.0001f);
	BOOST_CHECK_CLOSE(32.f, tiling.getTileSize().y, 0.0001f);
	
	tiling.setTileSize({32.f, 64.f});
	BOOST_CHECK_CLOSE(32.f, tiling.getTileSize().x, 0.0001f);
	BOOST_CHECK_CLOSE(64.f, tiling.getTileSize().y, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tiling_ortho_boundary) {
	utils::Tiling<utils::GridMode::Orthogonal> tiling{{32.f, 28.f}};
	sf::View view{{200.f, 200.f, 300.f, 200.f}};
	tiling.setView(view);

	auto range = tiling.getRange();
	// note: default padding necessary to fill entire view
	BOOST_CHECK_EQUAL(range.x, 12);	// ceil(300/32) + 2 (default padding)
	BOOST_CHECK_EQUAL(range.y, 10);	// ceil(200/28) + 2 (default padding)
	
	auto topleft = tiling.getTopleft();
	auto bottomleft = tiling.getBottomleft();
	BOOST_CHECK_EQUAL(bottomleft.x, topleft.x);
	BOOST_CHECK_EQUAL(bottomleft.y, topleft.y + range.y);
}

BOOST_AUTO_TEST_CASE(tiling_ortho_padding) {
	utils::Tiling<utils::GridMode::Orthogonal> tiling{{32.f, 28.f}};
	sf::View view{{200.f, 200.f, 300.f, 200.f}};
	tiling.setView(view);
	auto old_range = tiling.getRange();
	auto old_topleft = tiling.getTopleft();
	tiling.setPadding({12u, 7u});
	auto new_range = tiling.getRange();
	auto new_topleft = tiling.getTopleft();
	auto new_bottomleft = tiling.getBottomleft();
	
	BOOST_CHECK_EQUAL(new_range.x, old_range.x + 24u);
	BOOST_CHECK_EQUAL(new_range.y, old_range.y + 14u);
	BOOST_CHECK_EQUAL(new_topleft.x, old_topleft.x - 12u);
	BOOST_CHECK_EQUAL(new_topleft.y, old_topleft.y - 7u);
	BOOST_CHECK_EQUAL(new_bottomleft.x, new_topleft.x);
	BOOST_CHECK_EQUAL(new_bottomleft.y, new_topleft.y + new_range.y);
}

BOOST_AUTO_TEST_CASE(tiling_ortho_iteration) {
	utils::Tiling<utils::GridMode::Orthogonal> tiling{{32.f, 28.f}};
	sf::View view{{200.f, 200.f, 300.f, 200.f}};
	tiling.setView(view);

	auto range = tiling.getRange();
	auto n = 0u;
	std::for_each(begin(tiling), end(tiling), [&n](sf::Vector2u const & pos) {
		++n;
	});
	BOOST_CHECK_EQUAL(n, range.x * range.y);
}

BOOST_AUTO_TEST_CASE(tiling_orthogonal_toscreen) {
	utils::Tiling<utils::GridMode::Orthogonal> tiling{{32.f, 28.f}};
	
	auto spos = tiling.toScreen({12.f, 9.5f});
	BOOST_CHECK_CLOSE(spos.x, 384.f, 0.0001f);
	BOOST_CHECK_CLOSE(spos.y, 266.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tiling_orthogonal_fromscreen) {
	utils::Tiling<utils::GridMode::Orthogonal> tiling{{32.f, 28.f}};
	
	auto wpos = tiling.fromScreen({300.f, 140.f});
	BOOST_CHECK_CLOSE(wpos.x, 9.375f, 0.0001f);
	BOOST_CHECK_CLOSE(wpos.y, 5.f, 0.0001f);
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(tiling_isodiamond_iterator_step) {
	utils::TilingIterator<utils::GridMode::IsoDiamond> i{{2, 1}, {2, 3}};
	sf::Vector2i range;
	range = i.getRange();
	
	// step to the right (view-related)
	++i;
	auto pos = *i;
	BOOST_CHECK_EQUAL(pos.x, 3u);
	BOOST_CHECK_EQUAL(pos.y, 0u);
	BOOST_CHECK_EQUAL(i.getRange().x, range.x);
	BOOST_CHECK_EQUAL(i.getRange().y, range.y);
	
	// "zig"-step to the top of the next line (view-related)
	++i;
	pos = *i;
	BOOST_CHECK_EQUAL(pos.x, 3u);
	BOOST_CHECK_EQUAL(pos.y, 1u);
	
	// step to the right (view-related)
	++i;
	pos = *i;
	BOOST_CHECK_EQUAL(pos.x, 4u);
	BOOST_CHECK_EQUAL(pos.y, 0u);
	
	// "zag"-step to the top of the next line (view-related)
	++i;
	pos = *i;
	BOOST_CHECK_EQUAL(pos.x, 3u);
	BOOST_CHECK_EQUAL(pos.y, 2u);
}

BOOST_AUTO_TEST_CASE(tiling_isodiamond_boundary) {
	utils::Tiling<utils::GridMode::IsoDiamond> tiling{{32.f, 28.f}};
	sf::View view{{200.f, 200.f, 300.f, 200.f}};
	tiling.setView(view);

	auto range = tiling.getRange();
	// note: default padding necessary to fill entire view
	BOOST_CHECK_EQUAL(range.x, 14);	// ceil(300/32) + 4 (default padding)
	BOOST_CHECK_EQUAL(range.y, 24);	// (ceil(200/28) + 4 (default padding)) * 2 (iso height)
	
	auto topleft = tiling.getTopleft();
	auto bottomleft = tiling.getBottomleft();
	// note: going down in zig-zag will increase x- and y-coordinates by half height
	BOOST_CHECK_EQUAL(bottomleft.x, topleft.x + range.y / 2);
	BOOST_CHECK_EQUAL(bottomleft.y, topleft.y + range.y / 2);
}

BOOST_AUTO_TEST_CASE(tiling_isodiamond_toscreen) {
	utils::Tiling<utils::GridMode::IsoDiamond> tiling{{32.f, 14.f}};
	
	auto spos = tiling.toScreen({12.f, 9.5f});
	BOOST_CHECK_CLOSE(spos.x, 40.f, 0.0001f);
	BOOST_CHECK_CLOSE(spos.y, 150.5f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tiling_isodiamond_fromscreen) {
	utils::Tiling<utils::GridMode::IsoDiamond> tiling{{32.f, 14.f}};
	
	auto wpos = tiling.fromScreen({300.f, 140.f});
	BOOST_CHECK_CLOSE(wpos.x, 19.375f, 0.0001f);
	BOOST_CHECK_CLOSE(wpos.y, 0.625f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(tiling_isodiamond_padding) {
	utils::Tiling<utils::GridMode::IsoDiamond> tiling{{32.f, 28.f}};
	sf::View view{{200.f, 200.f, 300.f, 200.f}};
	tiling.setView(view);
	auto old_range = tiling.getRange();
	auto old_topleft = tiling.getTopleft();
	tiling.setPadding({12u, 7u});
	auto new_range = tiling.getRange();
	auto new_topleft = tiling.getTopleft();
	auto new_bottomleft = tiling.getBottomleft();
	
	BOOST_CHECK_EQUAL(new_range.x, old_range.x + 24u);
	BOOST_CHECK_EQUAL(new_range.y, old_range.y + 14u);
	BOOST_CHECK_EQUAL(new_topleft.x, old_topleft.x - 12u);
	BOOST_CHECK_EQUAL(new_topleft.y, old_topleft.y - 7u);
	// note: going down in zig-zag will increase x- and y-coordinates by half height
	BOOST_CHECK_EQUAL(new_bottomleft.x, new_topleft.x + new_range.y / 2);
	BOOST_CHECK_EQUAL(new_bottomleft.y, new_topleft.y + new_range.y / 2);
}

