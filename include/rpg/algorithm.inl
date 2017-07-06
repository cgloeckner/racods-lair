namespace rpg {

template <typename T>
void operator+=(
	utils::EnumMap<T, unsigned int>& lhs, utils::EnumMap<T, int> const& rhs) {
	for (auto& pair : lhs) {
		auto value = rhs[pair.first];
		if (value > 0 || std::abs(value) < pair.second) {
			pair.second += value;
		} else {
			pair.second = 0;
		}
	}
}

template <typename T>
void operator+=(utils::EnumMap<T, unsigned int>& lhs,
	utils::EnumMap<T, unsigned int> const& rhs) {
	for (auto& pair : lhs) {
		pair.second += rhs[pair.first];
	}
}

template <typename T>
void operator+=(
	utils::EnumMap<T, int>& lhs, utils::EnumMap<T, int> const& rhs) {
	for (auto& pair : lhs) {
		pair.second += rhs[pair.first];
	}
}

template <typename T>
void operator+=(
	utils::EnumMap<T, float>& lhs, utils::EnumMap<T, float> const& rhs) {
	for (auto& pair : lhs) {
		pair.second += rhs[pair.first];
	}
}

template <typename T>
void operator-=(
	utils::EnumMap<T, int>& lhs, utils::EnumMap<T, int> const& rhs) {
	for (auto& pair : lhs) {
		pair.second -= rhs[pair.first];
	}
}

template <typename T>
void operator-=(
	utils::EnumMap<T, float>& lhs, utils::EnumMap<T, float> const& rhs) {
	for (auto& pair : lhs) {
		pair.second -= rhs[pair.first];
	}
}

}  // ::game
