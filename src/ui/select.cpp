#include <utils/assert.hpp>
#include <ui/common.hpp>
#include <ui/select.hpp>

namespace ui {

Select::Select()
	: utils::Select{}
	, TextWidget{}
	, activate_sfx{nullptr}
	, alternate_sfx{nullptr} {
}

void Select::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(left, states);
	target.draw(right, states);
	target.draw(label, states);
}

void Select::onActivate() {
	ASSERT(channel != nullptr);
	ASSERT(activate_sfx != nullptr);
	channel->setBuffer(*activate_sfx);
	channel->play();
}

void Select::onStateChanged() {
	label.setString(caption);
	centerify(label);
}

void Select::onChanged() {
	ASSERT(channel != nullptr);
	ASSERT(alternate_sfx != nullptr);
	channel->setBuffer(*alternate_sfx);
	channel->play();
}

void Select::onUpdate() {
	auto index = getIndex();
	if (index > 0u) {
		left.setColor(sf::Color::White);
	} else {
		left.setColor({50, 50, 50});
	}
	if (index < size() - 1) {
		right.setColor(sf::Color::White);
	} else {
		right.setColor({50, 50, 50});
	}
	setString(at(index));
}

void Select::setFocus(bool focused) {
	onFocused(focused);
}

void Select::setPosition(sf::Vector2f const & pos) {
	ui::setPosition(left, {pos.x - width / 2.f, pos.y});
	ui::setPosition(right, {pos.x + width / 2.f, pos.y});
	ui::setPosition(label, pos);
}

void Select::setWidth(unsigned int width) {
	this->width = width;
}

void Select::setArrowTexture(sf::Texture const & tex) {
	left.setTexture(tex);
	right.setTexture(tex);
	right.setScale({-1, 1});
	centerify(left);
	centerify(right);
}

void Select::setActivateSfx(sf::SoundBuffer const & sfx) {
	activate_sfx = &sfx;
}

void Select::setAlternateSfx(sf::SoundBuffer const & sfx) {
	alternate_sfx = &sfx;
}

void Select::update(sf::Time const & elapsed) {
	ani(label, elapsed);
}

} // ::ui
