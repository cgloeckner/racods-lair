#include <utils/assert.hpp>
#include <ui/common.hpp>
#include <ui/input.hpp>

namespace ui {

Input::Input()
	: utils::Input{}
	, TextWidget{}
	, box{}
	, type_sfx{nullptr}
	, undo_sfx{nullptr} {
}

void Input::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(box, states);
	target.draw(label, states);
}

void Input::onStateChanged() {
	label.setString(caption);
	centerify(label);
}

void Input::onType() {
	ASSERT(channel != nullptr);
	ASSERT(type_sfx != nullptr);
	channel->setBuffer(*type_sfx);
	channel->play();
}

void Input::onUndo() {
	ASSERT(channel != nullptr);
	ASSERT(undo_sfx != nullptr);
	channel->setBuffer(*undo_sfx);
	channel->play();
}

void Input::setFocus(bool focused) {
	onFocused(focused);
}

void Input::setPosition(sf::Vector2f const & pos) {
	ui::setPosition(box, pos);
	ui::setPosition(label, pos);
}

sf::String Input::getContent() const {
	return getString();
}

void Input::setContent(sf::String const& string) {
	setString(string.toAnsiString());
}

void Input::setBoxTexture(sf::Texture const & tex) {
	box.setTexture(tex);
	centerify(box);
}

void Input::setTypeSfx(sf::SoundBuffer const & sfx) {
	type_sfx = &sfx;
}

void Input::setUndoSfx(sf::SoundBuffer const & sfx) {
	undo_sfx = &sfx;
}

void Input::update(sf::Time const & elapsed) {
	ani(label, elapsed);
}

// --------------------------------------------------------------------

FilenameInput::FilenameInput()
	: Input{} {
	whitelist.reserve(10u + 2u * 26u + 1u);
	for (sf::Uint32 i = 0u; i < 10u; ++i) {
		whitelist.push_back(i + u'0');
	}
	for (sf::Uint32 i = 0u; i < 26u; ++i) {
		whitelist.push_back(i + u'a');
		whitelist.push_back(i + u'A');
	}
	whitelist.push_back(u'_');
}

} // ::ui
