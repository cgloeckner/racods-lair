#include <SFML/Graphics.hpp>
#include <boost/test/unit_test.hpp>

#include <utils/atlas.hpp>

BOOST_AUTO_TEST_CASE(atlas_typical) {
	// prepare images
	std::vector<sf::Image> images{5u};
	images[0u].create(200u, 150u, sf::Color::Red);
	images[1u].create(250u, 190u, sf::Color::Yellow);
	images[2u].create(130u, 280u, sf::Color::Green);
	images[3u].create(270u, 240u, sf::Color::Blue);
	images[4u].create(100u, 350u, sf::Color::Black);
	
	// create atlas generator
	utils::AtlasGenerator<std::size_t> generator;
	for (auto i = 0u; i < images.size(); ++i) {
		generator.add(i, std::move(images[i]), {});
	}
	
	// generate atlas and assert success
	utils::Atlas<std::size_t> atlas;
	BOOST_REQUIRE(generator.generate({16u, 16u}, 1024u, atlas));
	BOOST_REQUIRE_EQUAL(atlas.frames.size(), 5u);
	
	// assert largest image (#3) to be aligned topleft
	auto const & chunk = atlas.frames[3u];
	BOOST_CHECK_EQUAL(chunk.clipping.left, 0);
	BOOST_CHECK_EQUAL(chunk.clipping.top, 0);
}

// customized hasher
struct MyHasher {
	std::size_t operator()(std::string const & s) const {
		return s.size(); // bad hashing but works for testing
	}
};

BOOST_AUTO_TEST_CASE(atlas_customized) {
	// prepare images
	std::vector<sf::Image> images{5u};
	images[0u].create(200u, 150u, sf::Color::Red);
	images[1u].create(250u, 190u, sf::Color::Yellow);
	images[2u].create(130u, 280u, sf::Color::Green);
	images[3u].create(270u, 240u, sf::Color::Blue);
	images[4u].create(100u, 350u, sf::Color::Black);
	
	// create atlas generator
	utils::AtlasGenerator<std::string> generator;
	for (auto i = 0u; i < images.size(); ++i) {
		generator.add("img_" + std::to_string(i), std::move(images[i]), {});
	}
	
	// generate atlas and assert success
	utils::Atlas<std::string, MyHasher> atlas;
	BOOST_REQUIRE(generator.generate({16u, 16u}, 1024u, atlas));
	BOOST_REQUIRE_EQUAL(atlas.frames.size(), 5u);
	
	// assert largest image ("img_3") to be aligned topleft
	auto const & chunk = atlas.frames["img_3"];
	BOOST_CHECK_EQUAL(chunk.clipping.left, 0);
	BOOST_CHECK_EQUAL(chunk.clipping.top, 0);
}

BOOST_AUTO_TEST_CASE(atlas_empty) {
	// create empty atlas generator
	utils::AtlasGenerator<std::size_t> generator;
	
	// generate atlas and assert success
	utils::Atlas<std::size_t> atlas;
	BOOST_CHECK(generator.generate({16u, 16u}, 1024u, atlas));
}

BOOST_AUTO_TEST_CASE(atlas_too_large_chunk) {
	// create atlas generator
	utils::AtlasGenerator<std::size_t> generator;
	sf::Image img;
	img.create(386u, 512u, sf::Color::Red);
	generator.add(0u, std::move(img), {});
	
	// generate atlas and assert std::length_error
	utils::Atlas<std::size_t> atlas;
	BOOST_CHECK_THROW(generator.generate({16u, 16u}, 256u, atlas), std::length_error);
}

BOOST_AUTO_TEST_CASE(atlas_too_many_chunks) {
	// create atlas generator
	utils::AtlasGenerator<std::size_t> generator;
	for (auto i = 0u; i < 20u; ++i) {
		sf::Image img;
		img.create(256u, 256u, sf::Color::Red);
		generator.add(i, std::move(img), {});
	}
	
	// generate atlas and assert failure
	utils::Atlas<std::size_t> atlas;
	BOOST_CHECK(!generator.generate({16u, 16u}, 1024u, atlas));
}

BOOST_AUTO_TEST_CASE(atlas_shrink_chunk_topleft) {
	// create atlas generator
	utils::AtlasGenerator<std::size_t> generator;
	sf::Image img;
	img.create(200u, 300u, sf::Color::Transparent);
	for (int y = 0; y < 25; ++y) {
		for (int x = 0; x < 12; ++x) {
			img.setPixel(x, y, sf::Color::Green);
		}
	}
	generator.add(0u, std::move(img), {100.f, 150.f});
	
	// generate atlas and assert success
	utils::Atlas<std::size_t> atlas;
	BOOST_REQUIRE(generator.generate({16u, 16u}, 1024u, atlas));
	
	// assert resize, but origin hasn't moved
	auto const & chunk = atlas.frames[0u];
	BOOST_CHECK_EQUAL(chunk.clipping.left, 0);
	BOOST_CHECK_EQUAL(chunk.clipping.top, 0);
	BOOST_CHECK_EQUAL(chunk.clipping.width, 12);
	BOOST_CHECK_EQUAL(chunk.clipping.height, 25);
	BOOST_CHECK_CLOSE(chunk.origin.x, 100.f, 0.0001f);
	BOOST_CHECK_CLOSE(chunk.origin.y, 150.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(atlas_shrink_chunk_middle) {
	// create atlas generator
	utils::AtlasGenerator<std::size_t> generator;
	sf::Image img;
	img.create(200u, 300u, sf::Color::Transparent);
	for (int y = 80; y < 110; ++y) {
		for (int x = 150; x < 170; ++x) {
			img.setPixel(x, y, sf::Color::Green);
		}
	}
	generator.add(0u, std::move(img), {100.f, 150.f});
	
	// generate atlas and assert success
	utils::Atlas<std::size_t> atlas;
	BOOST_REQUIRE(generator.generate({16u, 16u}, 1024u, atlas));
	
	// assert resize, but origin has moved
	auto const & chunk = atlas.frames[0u];
	BOOST_CHECK_EQUAL(chunk.clipping.left, 0);
	BOOST_CHECK_EQUAL(chunk.clipping.top, 0);
	BOOST_CHECK_EQUAL(chunk.clipping.width, 20);
	BOOST_CHECK_EQUAL(chunk.clipping.height, 30);
	BOOST_CHECK_CLOSE(chunk.origin.x, -50.f, 0.0001f);
	BOOST_CHECK_CLOSE(chunk.origin.y, 70.f, 0.0001f);
}

