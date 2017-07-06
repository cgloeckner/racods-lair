#include <utils/assert.hpp>
#include <ui/common.hpp>
#include <ui/checkbox.hpp>

namespace ui {

Checkbox::Checkbox()
	: utils::Button{}
	, TextWidget{}
	, box{}
	, mark{}
	, checked{false}
	, activate_sfx{nullptr}
	, deactivate_sfx{nullptr} {
}

void Checkbox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(box, states);
	if (checked) {
		target.draw(mark, states);
	}
	target.draw(label, states);
}

void Checkbox::onStateChanged() {
	label.setString(caption);
	centerify(label);
}

void Checkbox::onActivate() {
	checked = !checked;
	onStateChanged();
	
	ASSERT(channel != nullptr);
	if (checked) {
		ASSERT(activate_sfx != nullptr);
		channel->setBuffer(*activate_sfx);
	} else {
		ASSERT(deactivate_sfx != nullptr);
		channel->setBuffer(*deactivate_sfx);
	}
	channel->play();
}

void Checkbox::setChecked(bool checked) {
	this->checked = checked;
	onStateChanged();
}

bool Checkbox::isChecked() const {
	return checked;
}

void Checkbox::setFocus(bool focused) {
	onFocused(focused);
}

void Checkbox::setPosition(sf::Vector2f const & pos) {
	ui::setPosition(box, {pos.x - width / 2.f, pos.y});
	ui::setPosition(mark, {pos.x - width / 2.f, pos.y});
	ui::setPosition(label, pos);
}

void Checkbox::setWidth(unsigned int width) {
	this->width = width;
}

void Checkbox::setBoxTexture(sf::Texture const & tex) {
	box.setTexture(tex);
	centerify(box);
}

void Checkbox::setMarkTexture(sf::Texture const & tex) {
	mark.setTexture(tex);
	centerify(mark);
}

void Checkbox::setActivateSfx(sf::SoundBuffer const & sfx) {
	activate_sfx = &sfx;
}

void Checkbox::setDeactivateSfx(sf::SoundBuffer const & sfx) {
	deactivate_sfx = &sfx;
}

void Checkbox::update(sf::Time const & elapsed) {
	ani(label, elapsed);
}

} // ::ui
