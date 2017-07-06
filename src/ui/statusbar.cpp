#include <utils/assert.hpp>
#include <ui/common.hpp>
#include <ui/statusbar.hpp>

namespace ui {

BaseBar::BaseBar(sf::Drawable& box, sf::Drawable& fill)
	: sf::Drawable{}
	, sf::Transformable{}
	, box{box}
	, fill{fill} {
}

void BaseBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	target.draw(fill, states);
	target.draw(box, states);
}

// --------------------------------------------------------------------

ExperienceBar::ExperienceBar()
	: BaseBar{box, fill}
	, box{}
	, fill{}
	, thickness{5.f}
	, value{0.f} {
	box.setFillColor(sf::Color::Transparent);
}

void ExperienceBar::updateValue() {
	auto size = box.getSize();
	size.x *= value;
	fill.setSize(size);
}

sf::Vector2u ExperienceBar::getSize() const {
	return sf::Vector2u{box.getSize()};
}

void ExperienceBar::setSize(sf::Vector2u const & size) {
	box.setSize(sf::Vector2f{size});
	fill.setSize(sf::Vector2f{size});
	ui::centerify(box);
	ui::centerify(fill);
	
	updateValue();
}

void ExperienceBar::setOutlineColor(sf::Color const & color) {
	box.setOutlineColor(color);
}

void ExperienceBar::setFillColor(sf::Color const & color) {
	fill.setFillColor(color);
}

void ExperienceBar::setOutlineThickness(float thickness) {
	this->thickness = thickness;
	box.setOutlineThickness(-thickness);
}

void ExperienceBar::update(std::uint64_t current, std::uint64_t maximum) {
	ASSERT(maximum > 0u);
	value = (1.f * current) / maximum;
	
	updateValue();
}

// --------------------------------------------------------------------

StatsBar::StatsBar(bool vertical)
	: BaseBar{box, fill}
	, box{}
	, fill{}
	, value{0.f}
	, valid{false}
	, vertical{vertical} {
}

void StatsBar::updateValue() {
	auto clip = box.getTextureRect();
	auto pos = box.getPosition();
	
	if (vertical) {
		clip.width *= value;
	} else {
		pos.y += clip.height * (1.f - value);
		clip.top = clip.height * (1.f - value);
		clip.height *= value;
	}
	
	fill.setPosition(pos);
	fill.setTextureRect(clip);
}

sf::Vector2u StatsBar::getSize() const {
	auto ptr = box.getTexture();
	ASSERT(ptr != nullptr);
	return ptr->getSize();
}

void StatsBar::setBoxTexture(sf::Texture const & tex) {
	box.setTexture(tex);
	ui::centerify(box);
}

void StatsBar::setFillTexture(sf::Texture const & tex) {
	fill.setTexture(tex);
	ui::centerify(fill);
	
	updateValue();
}

void StatsBar::setFillColor(sf::Color const & color) {
	fill.setColor(color);
}

bool StatsBar::isValid() const {
	return valid;
}

void StatsBar::update(std::uint64_t current, std::uint64_t maximum) {
	if (maximum == 0u) {
		valid = false;
		return;
	}
	valid = true;
	ASSERT(maximum >= 0u);
	value = (1.f * current) / maximum;
	
	updateValue();
}

}  // ::ui
