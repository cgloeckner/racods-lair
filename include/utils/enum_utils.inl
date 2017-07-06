namespace utils {

template <typename T>
EnumRange<T>::iterator::iterator(std::size_t value)
	: value{value} {}

template <typename T>
T EnumRange<T>::iterator::operator*() const {
	return static_cast<T>(value);
}

template <typename T>
void EnumRange<T>::iterator::operator++() {
	auto max_val = static_cast<std::size_t>(std::numeric_limits<T>::max());
	if (value <= max_val) {
		++value;
	}
}

template <typename T>
bool EnumRange<T>::iterator::operator!=(
	EnumRange<T>::iterator const& other) const {
	return value != other.value;
}

template <typename T>
bool EnumRange<T>::iterator::operator==(
	EnumRange<T>::iterator const& other) const {
	return value == other.value;
}

template <typename T>
typename EnumRange<T>::iterator EnumRange<T>::begin() const {
	return static_cast<std::size_t>(std::numeric_limits<T>::min());
}

template <typename T>
typename EnumRange<T>::iterator EnumRange<T>::end() const {
	return static_cast<std::size_t>(std::numeric_limits<T>::max()) + 1u;
}

template <typename T>
std::size_t constexpr getEnumCount() noexcept {
	return static_cast<std::size_t>(std::numeric_limits<T>::max())
		- static_cast<std::size_t>(std::numeric_limits<T>::min()) + 1u;
}

}  // ::utils
