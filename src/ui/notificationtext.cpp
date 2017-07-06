#include <iostream>

#include <Thor/Graphics.hpp>
#include <Thor/Math.hpp>

#include <utils/algorithm.hpp>
#include <utils/assert.hpp>
#include <ui/notificationtext.hpp>

namespace ui {

NotificationNode::NotificationNode()
	: caption{}
	, age{sf::Time::Zero}
	, max_age{sf::seconds(1.f)}
	, alpha{1.f} {
}

void NotificationNode::update(sf::Time const & elapsed) {
	if (alpha > 0.f) {
		// calculate changes
		age += elapsed;
		alpha -= decay * elapsed.asMilliseconds();
		alpha = std::max(alpha, 0.f); 
		thor::setAlpha(caption, static_cast<sf::Uint8>(255 * alpha));
	}
}

// --------------------------------------------------------------------

NotificationTexts::NotificationTexts()
	: sf::Drawable{}
	, sf::Transformable{}
	, font{nullptr}
	, char_size{10u}
	, nodes{} {
	nodes.reserve(5u);
}

void NotificationTexts::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	
	float offset{0.f};
	if (!nodes.empty()) {
		auto rect = nodes.front().caption.getLocalBounds();
		offset = rect.top + rect.height;
	}
	states.transform.translate(0, -(nodes.size() * offset));
	for (auto const & node: nodes) {
		states.transform.translate(0, offset);
		target.draw(node.caption, states);
	}
}

void NotificationTexts::setup(sf::Font const & font, unsigned int char_size) {
	this->font = &font;
	this->char_size = char_size;
}

void NotificationTexts::add(std::string const & string, sf::Color const & color,
	sf::Time const & max_age) {
	ASSERT(font != nullptr);
	
	// create node
	nodes.emplace_back();
	auto& node = nodes.back();
	node.caption.setFont(*font);
	node.caption.setString(string);
	node.caption.setCharacterSize(char_size);
	node.caption.setColor(color);
	node.max_age = max_age;
	node.decay = 1.f / max_age.asMilliseconds();
	node.alpha = color.a / 255.f;
	
	// align bottomleft
	auto rect = node.caption.getLocalBounds();
	node.caption.setOrigin({rect.left, rect.top + rect.height});
}

void NotificationTexts::update(sf::Time const & elapsed) {
	for (auto & node: nodes) {
		node.update(elapsed);
	}
	utils::remove_if(nodes, [&](NotificationNode& node) {
		return node.age >= node.max_age;
	});
}

}  // ::ui
