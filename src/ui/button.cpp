#include <iostream>
#include <utils/assert.hpp>
#include <ui/common.hpp>
#include <ui/button.hpp>

namespace ui {

Button::Button()
	: utils::Button{}
	, TextWidget{}
	, activate_sfx{nullptr} {
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(label, states);
}

void Button::onActivate() {
	ASSERT(channel != nullptr);
	ASSERT(activate_sfx != nullptr);
	channel->setBuffer(*activate_sfx);
	channel->play();
}

void Button::onStateChanged() {
	label.setString(caption);
	centerify(label);
}

void Button::setFocus(bool focused) {
	onFocused(focused);
}

void Button::setActivateSfx(sf::SoundBuffer const & sfx) {
	activate_sfx = &sfx;
}

void Button::setPosition(sf::Vector2f const & pos) {
	ui::setPosition(label, pos);
}

void Button::update(sf::Time const & elapsed) {
	ani(label, elapsed);
}

} // ::ui
