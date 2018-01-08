#include <utils/assert.hpp>
#include <ui/common.hpp>

namespace ui {

void centerify(sf::Sprite& sprite) {
	sprite.setOrigin(sf::Vector2f{sprite.getTexture()->getSize()} / 2.f);
}

// --------------------------------------------------------------------

TextWidget::TextWidget()
	: label{}
	, ani{}
	, color{sf::Color::White}
	, highlight{sf::Color::Yellow}
	, navigate_sfx{nullptr} {
}

void TextWidget::onFocused(bool focused) {
	if (focused) {
		label.setFillColor(highlight);
		label.setOutlineColor(highlight);
		ani.startAnimation();
		
		if (channel != nullptr && navigate_sfx != nullptr) {
			channel->setBuffer(*navigate_sfx);
			channel->play();
		}
	} else {
		label.setFillColor(color);
		label.setOutlineColor(color);
		ani.stopAnimation();
	}
	onStateChanged();
}

void TextWidget::setChannel(sf::Sound& channel) {
	this->channel = &channel;
}

void TextWidget::setString(std::string const & caption) {
	this->caption = caption;
	onStateChanged();
}

void TextWidget::setFont(sf::Font const & font) {
	label.setFont(font);
	onStateChanged();
}

void TextWidget::setCharacterSize(unsigned int size) {
	label.setCharacterSize(size);
	onStateChanged();
}

void TextWidget::setDefaultColor(sf::Color const & color) {
	this->color = color;
	onStateChanged();
}

void TextWidget::setHighlightColor(sf::Color const & color) {
	highlight = color;
	onStateChanged();
}

void TextWidget::setNavigateSfx(sf::SoundBuffer const & sfx) {
	navigate_sfx = &sfx;
}

std::string TextWidget::getString() const {
	return caption;
}

sf::Font const * TextWidget::getFont() const {
	return label.getFont();
}

unsigned int TextWidget::getCharacterSize() const {
	return label.getCharacterSize();
}

sf::Color TextWidget::getDefaultColor() const {
	return color;
}

sf::Color TextWidget::getHighlightColor() const {
	return highlight;
}

// --------------------------------------------------------------------

Menu::Menu()
	: utils::Menu<std::size_t>{} {
	refreshMenuControls();
}

void Menu::refreshMenuControls() {
	// determine first joystick (or use 0u if none)
	unsigned int gamepad_id{0u};
	for (auto i = 0u; i < sf::Joystick::Count; ++i) {
		if (sf::Joystick::isConnected(i)) {
			gamepad_id = i;
			break;
		}
	}
	
	// bind controls
	clear(utils::MenuAction::Activate);
	bind(utils::MenuAction::Activate, {sf::Keyboard::Return});
	bind(utils::MenuAction::Activate, {sf::Keyboard::Space});
	bind(utils::MenuAction::Activate, {gamepad_id, 0u});
	
	clear(utils::MenuAction::NavigatePrev);
	bind(utils::MenuAction::NavigatePrev, {sf::Keyboard::Up});
	bind(utils::MenuAction::NavigatePrev, {gamepad_id, sf::Joystick::Axis::Y, -25.f});
	
	clear(utils::MenuAction::NavigateNext);
	bind(utils::MenuAction::NavigateNext, {sf::Keyboard::Down});
	bind(utils::MenuAction::NavigateNext, {gamepad_id, sf::Joystick::Axis::Y, 25.f});
	
	clear(utils::MenuAction::AlternatePrev);
	bind(utils::MenuAction::AlternatePrev, {sf::Keyboard::Left});
	bind(utils::MenuAction::AlternatePrev, {gamepad_id, sf::Joystick::Axis::X, -25.f});
	
	clear(utils::MenuAction::AlternateNext);
	bind(utils::MenuAction::AlternateNext, {sf::Keyboard::Right});
	bind(utils::MenuAction::AlternateNext, {gamepad_id, sf::Joystick::Axis::X, 25.f});
}

} // :: ui
