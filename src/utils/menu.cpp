#include <utils/menu.hpp>

#include <iostream>

namespace utils {

unsigned int const MENU_COOLDOWN = 100u;

// ---------------------------------------------------------------------------

Widget::Widget()
	: visible{true} {
}

Widget::~Widget() {
}

void Widget::setVisible(bool visible) {
	this->visible = visible;
}

bool Widget::isVisible() const {
	return visible;
}

void Widget::handle(sf::Uint32 unicode) {
	// not implemented by most widgets
}

void Widget::update(sf::Time const & elapsed) {
}

// ---------------------------------------------------------------------------

void Button::onActivate() {}

void Button::handle(MenuAction action) {
	if (action == MenuAction::Activate) {
		onActivate();
		if (activate != nullptr) {
			activate();
		}
	}
}

// ---------------------------------------------------------------------------

Select::Select() : Widget{}, index{0u} {}

void Select::onActivate() {}

void Select::handle(MenuAction action) {
	if (empty()) {
		return;
	}
	if (action == MenuAction::AlternatePrev) {
		if (index > 0u) {
			--index;
		} else {
			return;
		}
	} else if (action == MenuAction::AlternateNext) {
		if (index < size() - 1) {
			++index;
		} else {
			return;
		}
	}
	onUpdate();
	if (action == MenuAction::Activate) {
		if (activate != nullptr) {
			onActivate();
			activate();
		}
	} else if (action == MenuAction::AlternatePrev ||
			   action == MenuAction::AlternateNext) {
		if (change != nullptr) {
			onChanged();
			change();
		}
	}
}

void Select::setIndex(std::size_t index) {
	this->index = index;
	onUpdate();
}

std::size_t Select::getIndex() const { return index; }

// ---------------------------------------------------------------------------

Input::Input()
	: Widget{}
	, max_len{0u}
	, whitelist{}
	, blacklist{}
	, typing{nullptr} {}

void Input::onTyping() {}

bool Input::isAllowed(sf::Uint32 unicode) const {
	auto i = std::find(begin(blacklist), end(blacklist), unicode);
	if (i != blacklist.end()) {
		// symbol is explicitly forbidden
		return false;
	}
	if (whitelist.empty()) {
		// symbol is implicitly allowed by empty whitelist
		return true;
	}
	i = std::find(begin(whitelist), end(whitelist), unicode);
	if (i == whitelist.end()) {
		// symbol is implicitly forbidden by non-empty whitelist
		return false;
	}
	// symbol is explicitly allowed by non-empty whitelist
	return true;
}

void Input::handle(utils::MenuAction action) {
	// nothing to do
}

void Input::handle(sf::Uint32 unicode) {
	auto text = getContent();
	bool allowed = true;
	if (unicode == '\b') {
		// handle backspace
		text = text.substring(0u, text.getSize() - 1u);
		onUndo();
	} else {
		allowed = isAllowed(unicode);
		if (max_len > 0u) {
			allowed = allowed && text.getSize() < max_len;
		}
		if (allowed) {
			// add character
			text += unicode;
			onType();
		}
	}
	if (allowed) {
		setContent(text);
	}
	if (typing != nullptr) {
		typing(unicode, allowed);
	}
}

void Input::setMaxLength(std::size_t max_len) {
	this->max_len = max_len;
}

std::size_t Input::getMaxLength() const {
	return max_len;
}

}  // ::sfext
