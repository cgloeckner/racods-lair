template <typename T>
utils::Logger& operator<<(utils::Logger& lhs, T const& rhs) {
	for (auto& ptr : lhs) {
		(*ptr) << rhs;
	}
	return lhs;
}

// --------------------------------------------------------------------

template <typename T>
std::ostream& operator<<(std::ostream& lhs, sf::Vector2<T> const& rhs) {
	return lhs << thor::toString(rhs);
}

template <typename T>
std::ostream& operator<<(std::ostream& lhs, sf::Vector3<T> const& rhs) {
	return lhs << thor::toString(rhs);
}

template <typename T>
std::ostream& operator<<(std::ostream& lhs, sf::Rect<T> const& rhs) {
	return lhs << thor::toString(rhs);
}
