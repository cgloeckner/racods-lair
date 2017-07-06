namespace ui {

template <typename T>
void centerify(T& widget) {
	auto rect = widget.getLocalBounds();
	sf::Vector2i origin;
	origin.x = rect.left + rect.width / 2.f;
	origin.y = rect.top + rect.height / 2.f;
	widget.setOrigin(sf::Vector2f{origin});
}

template <typename T>
void setPosition(T& widget, sf::Vector2f const & pos) {
	widget.setPosition(sf::Vector2f{sf::Vector2i{pos}});
}

} // :: ui
