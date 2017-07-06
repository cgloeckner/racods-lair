namespace rpg {

template <typename Enum, typename T>
void parse(utils::ptree_type const& ptree, utils::EnumMap<Enum, T>& map,
	std::string const& prefix) {
	for (auto& pair : map) {
		pair.second =
			ptree.get<T>(prefix + ".<xmlattr>." + to_string(pair.first));
	}
}

template <typename Enum, typename T, typename Handle>
void parseEnumMap(utils::ptree_type const & ptree, utils::EnumMap<Enum, T>& map,
	std::string const& prefix, Handle func) {
	for (auto& pair: map) {
		auto opt_child = ptree.get_child_optional(prefix + "." + to_string(pair.first));
		if (opt_child) {
			func(*opt_child, pair.second);
		}
	}
}

template <typename Enum, typename T>
void parse(utils::ptree_type const& ptree, utils::EnumMap<Enum, T>& map,
	std::string const& prefix, T default_value) {
	for (auto& pair : map) {
		pair.second = ptree.get<T>(
			prefix + ".<xmlattr>." + to_string(pair.first), default_value);
	}
}

template <typename Enum, typename T>
void dump(utils::ptree_type& ptree, utils::EnumMap<Enum, T> const& map,
	std::string const& prefix) {
	for (auto const& pair : map) {
		ptree.put(prefix + ".<xmlattr>." + to_string(pair.first), pair.second);
	}
}

template <typename Enum, typename T, typename Handle>
void dumpEnumMap(utils::ptree_type& ptree, utils::EnumMap<Enum, T> const & map,
	std::string const& prefix, Handle func) {
	for (auto const & pair: map) {
		utils::ptree_type child;
		func(child, pair.second);
		ptree.add_child(prefix + "." + to_string(pair.first), child);
	}
}

template <typename Enum, typename T>
void dump(utils::ptree_type& ptree, utils::EnumMap<Enum, T> const& map,
	std::string const& prefix, T default_value) {
	for (auto const& pair : map) {
		if (pair.second != default_value) {
			ptree.put(
				prefix + ".<xmlattr>." + to_string(pair.first), pair.second);
		}
	}
}

}  // ::game
