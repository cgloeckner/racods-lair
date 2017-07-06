namespace utils {

template <typename Key, typename Value>
EnumMap<Key, Value>::EnumMap() {
	static_assert(std::is_enum<Key>::value, "Key needs to be an enumeration");
	for (std::size_t i = 0u; i < size(); ++i) {
		data[i].first = static_cast<Key>(i);
	}
}

template <typename Key, typename Value>
EnumMap<Key, Value>::EnumMap(Value const& default_value) {
	static_assert(std::is_enum<Key>::value, "Key needs to be an enumeration");
	for (std::size_t i = 0u; i < size(); ++i) {
		data[i].first = static_cast<Key>(i);
		data[i].second = default_value;
	}
}

template <typename Key, typename Value>
Value const& EnumMap<Key, Value>::operator[](Key const& key) const {
	return data[static_cast<std::size_t>(key) - MinKey].second;
}

template <typename Key, typename Value>
Value& EnumMap<Key, Value>::operator[](Key const& key) {
	return data[static_cast<std::size_t>(key) - MinKey].second;
}

template <typename Key, typename Value>
std::size_t EnumMap<Key, Value>::size() const {
	return NumKeys;
}

template <typename Key, typename Value>
typename EnumMap<Key, Value>::iterator EnumMap<Key, Value>::begin() {
	return data.begin();
}

template <typename Key, typename Value>
typename EnumMap<Key, Value>::iterator EnumMap<Key, Value>::end() {
	return data.end();
}

template <typename Key, typename Value>
typename EnumMap<Key, Value>::const_iterator EnumMap<Key, Value>::begin()
	const {
	return data.begin();
}

template <typename Key, typename Value>
typename EnumMap<Key, Value>::const_iterator EnumMap<Key, Value>::end() const {
	return data.end();
}

// ----------------------------------------------------------------------------

template <typename Key, typename Value>
inline typename EnumMap<Key, Value>::iterator begin(EnumMap<Key, Value>& map) {
	return map.begin();
}

template <typename Key, typename Value>
inline typename EnumMap<Key, Value>::iterator end(EnumMap<Key, Value>& map) {
	return map.end();
}

template <typename Key, typename Value>
inline typename EnumMap<Key, Value>::const_iterator begin(
	EnumMap<Key, Value> const& map) {
	return map.begin();
}

template <typename Key, typename Value>
inline typename EnumMap<Key, Value>::const_iterator end(
	EnumMap<Key, Value> const& map) {
	return map.end();
}

template <typename Key, typename Value>
inline bool operator==(
	EnumMap<Key, Value> const& lhs, EnumMap<Key, Value> const& rhs) {
	for (auto const& u : lhs) {
		if (rhs[u.first] != u.second) {
			return false;
		}
	}
	return true;
}

template <typename Key, typename Value>
inline bool operator!=(
	EnumMap<Key, Value> const& lhs, EnumMap<Key, Value> const& rhs) {
	for (auto const& u : lhs) {
		if (rhs[u.first] != u.second) {
			return true;
		}
	}
	return false;
}

}  // ::utils
