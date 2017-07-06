#pragma once
#include <memory>
#include <algorithm>

namespace utils {

template <typename T>
Menu<T>::Menu()
	: widgets{}
	, focus{0u}
	, unicodes{}
	, input{}
	, binding{}
	, cooldown{sf::Time::Zero} {
}

template <typename T>
bool Menu<T>::isActive(MenuAction action) {
	auto i = binding.find(action);
	if (i == binding.end()) {
		return false;
	}
	for (auto const & action: i->second) {
		if (input.isActive(action)) {
			input.reset(action);
			return true;
		}
	}
	return false;
}

template <typename T>
typename Menu<T>::container::iterator Menu<T>::at(T key) {
	return std::find_if(begin(widgets), end(widgets),
		[&key](pair const& pair) { return (pair.first == key); });
}

template <typename T>
typename Menu<T>::container::const_iterator Menu<T>::at(T key) const {
	return std::find_if(begin(widgets), end(widgets),
		[&key](pair const& pair) { return (pair.first == key); });
}

template <typename T>
void Menu<T>::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	for (auto const& pair : widgets) {
		auto& w = *(pair.second);
		// check visibility
		if (!w.isVisible()) {
			continue;
		}
		// draw widget
		target.draw(w, states);
	}
}

template <typename T>
void Menu<T>::changeFocus(T key, bool forward) {
	auto i = at(key);
	// skip invisibe widget(s)
	auto j = i;
	while (!j->second->isVisible()) {
		if (forward) {
			++j;
			if (j == end(widgets)) {
				j = begin(widgets);
			}
		} else {
			if (j == begin(widgets)) {
				j = std::prev(end(widgets));
			} else {
				--j;
			}
		}
		if (j == i) {
			// not visible widget found
			return;
		}
	}
	// change focus
	auto prev = at(focus);
	if (prev != end(widgets)) {
		prev->second->setFocus(false);
	}
	j->second->setFocus(true);
	focus = j->first;
}

template <typename T>
template <typename W, typename... Args>
W& Menu<T>::acquire(T key, Args&&... args) {
	// create widget
	std::unique_ptr<Widget> widget = std::make_unique<W>(std::forward<Args>(args)...);
	auto& raw = *dynamic_cast<W*>(widget.get());
	// obtain ownership to gui container
	widgets.emplace_back(key, std::move(widget));
	if (widgets.size() == 1u) {
		// focus first widget
		changeFocus(key);
	}
	return raw;
}

template <typename T>
template <typename W>
W& Menu<T>::query(T key) {
	auto i = at(key);
	return dynamic_cast<W&>(*(i->second));
}

template <typename T>
template <typename W>
W const & Menu<T>::query(T key) const {
	auto i = at(key);
	return dynamic_cast<W&>(*(i->second));
}

template <typename T>
void Menu<T>::release(T key) {
	auto i = at(key);
	if (i != end(widgets)) {
		widgets.erase(i);
	}
}

template <typename T>
void Menu<T>::setFocus(T key) {
	auto i = at(key);
	if (i != end(widgets)) {
		changeFocus(i->first);
	}
}

template <typename T>
template <typename W>
void Menu<T>::setFocus(W const& widget) {
	auto i = std::find_if(begin(widgets), end(widgets),
		[&widget](pair const& pair) { return (pair.second.get() == &widget); });
	if (i != end(widgets)) {
		changeFocus(i->first);
	}
}

template <typename T>
T Menu<T>::queryFocus() const {
	return focus;
}

template <typename T>
void Menu<T>::clear(MenuAction const & action) {
	auto i = binding.find(action);
	if (i != binding.end()) {
		binding.erase(i);
	}
}

template <typename T>
void Menu<T>::bind(MenuAction const& action, InputAction const & input) {
	auto i = binding.find(action);
	if (i == binding.end()) {
		binding[action] = {input};
	} else {
		i->second.push_back(input);
	}
}

template <typename T>
void Menu<T>::handle(sf::Event const& event) {
	input.pushEvent(event);

	if (event.type == sf::Event::TextEntered) {
		unicodes.push_back(event.text.unicode);
	}
}

template <typename T>
void Menu<T>::update(sf::Time const & elapsed) {
	cooldown -= elapsed;
	if (cooldown < sf::Time::Zero) {
		cooldown = sf::Time::Zero;
	}
	bool wait{false};
	
	if (!widgets.empty()) {
		auto i = at(focus);
		
		if (cooldown == sf::Time::Zero) {
			// handle activation
			if (isActive(MenuAction::Activate)) {
				wait = true;
				i->second->handle(MenuAction::Activate);
			}
			
			// handle alternate
			if (isActive(MenuAction::AlternatePrev)) {
				wait = true;
				i->second->handle(MenuAction::AlternatePrev);
			}
			if (isActive(MenuAction::AlternateNext)) {
				wait = true;
				i->second->handle(MenuAction::AlternateNext);
			}

			// handle navigation
			if (isActive(MenuAction::NavigatePrev)) {
				wait = true;
				if (i == begin(widgets)) {
					i = std::prev(end(widgets));
				} else {
					--i;
				}
				changeFocus(i->first, false);
			}
			if (isActive(MenuAction::NavigateNext)) {
				wait = true;
				++i;
				if (i == end(widgets)) {
					i = begin(widgets);
				}
				changeFocus(i->first);
			}
		}

		// handle all read unicode characters
		for (auto unicode : unicodes) {
			i->second->handle(unicode);
		}
		unicodes.clear();
	}

	// update visible widgets
	for (auto& pair: widgets) {
		if (pair.second->isVisible()) {
			pair.second->update(elapsed);
		}
	}
	
	if (wait) {
		cooldown = sf::milliseconds(MENU_COOLDOWN);
	}
}

}  // ::sfext
