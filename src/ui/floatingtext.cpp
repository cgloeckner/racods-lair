#include <iostream>

#include <Thor/Graphics.hpp>
#include <Thor/Math.hpp>

#include <utils/algorithm.hpp>
#include <utils/assert.hpp>
#include <ui/floatingtext.hpp>

namespace ui {

extern float const MIN_FLOAT_VEC = -0.33f;
extern float const MAX_FLOAT_VEC = 0.33f;
extern float const FLOAT_SPEED = 0.1f;
extern float const SCALE_SPEED = 0.00075f;

FloatingNode::FloatingNode()
	: caption{}
	, age{sf::Time::Zero}
	, max_age{sf::seconds(1.f)}
	, vector{}
	, scale{0.2f}
	, alpha{1.f} {
}

void FloatingNode::update(sf::Time const & elapsed) {
	// calculate changes
	age += elapsed;
	auto delta = vector * (FLOAT_SPEED * elapsed.asMilliseconds());
	scale += SCALE_SPEED * elapsed.asMilliseconds();
	alpha -= decay * elapsed.asMilliseconds();
	alpha = std::max(alpha, 0.f); 
	// update data
	caption.move(delta);
	caption.setScale({scale, scale});
	thor::setAlpha(caption, static_cast<sf::Uint8>(255 * alpha));
}

// --------------------------------------------------------------------

FloatingTexts::FloatingTexts()
	: sf::Drawable{}
	, nodes{} {
	nodes.reserve(200u);
}

void FloatingTexts::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	for (auto const & node: nodes) {
		target.draw(node.caption, states);
	}
}

void FloatingTexts::add(sf::Font const & font, std::string const & string,
	unsigned int size, sf::Vector2f const & pos, sf::Color const & color, sf::Time const & max_age, bool random_dir) {
	nodes.emplace_back();
	auto& node = nodes.back();
	node.caption.setPosition(pos);
	node.caption.setFont(font);
	node.caption.setString(string);
	node.caption.setCharacterSize(size);
	node.caption.setFillColor(color);
	node.caption.setOutlineColor(color);
	node.max_age = max_age;
	node.decay = 1.f / max_age.asMilliseconds();
	if (random_dir) {
		node.vector.x = thor::random(MIN_FLOAT_VEC, MAX_FLOAT_VEC);
		node.vector.y = thor::random(MIN_FLOAT_VEC, MAX_FLOAT_VEC);
	}
	node.alpha = color.a / 255.f;
	
	auto rect = node.caption.getLocalBounds();
	node.caption.setOrigin({rect.left + rect.width / 2.f,
		rect.top + rect.height / 2.f});
}

void FloatingTexts::update(sf::Time const & elapsed) {
	for (auto & node: nodes) {
		node.update(elapsed);
	}
	utils::remove_if(nodes, [&](FloatingNode& node) {
		return node.age >= node.max_age;
	});
}

}  // ::ui
