namespace utils {

template <typename T, typename Parser>
void parse_vector(ptree_type const& ptree, std::string const& root_tag,
	std::string const& item_tag, std::vector<T>& data, Parser func) {
	parse_vector(ptree.get_child(root_tag), item_tag, data, func);
}

template <typename T, typename Parser>
void parse_vector(ptree_type const& ptree, std::string const& item_tag,
	std::vector<T>& data, Parser func) {
	data.clear();
	for (auto const& child : ptree) {
		if (child.first != item_tag) {
			// ignore child
			continue;
		}
		data.emplace_back();
		func(child.second, data.back());
	}
}

template <typename T, typename Dumper>
void dump_vector(ptree_type& ptree, std::string const& root_tag,
	std::string const& item_tag, std::vector<T> const& data, Dumper func) {
	ptree_type array;
	dump_vector(array, item_tag, data, func);
	ptree.add_child(root_tag, array);
}

template <typename T, typename Dumper>
void dump_vector(ptree_type& ptree, std::string const& item_tag,
	std::vector<T> const& data, Dumper func) {
	for (auto const& elem : data) {
		ptree_type child;
		func(child, elem);
		ptree.add_child(item_tag, child);
	}
}

// ---------------------------------------------------------------------------

template <typename T, std::size_t N, typename Parser>
void parse_array(ptree_type const& ptree, std::string const& root_tag,
	std::string const& item_tag, std::array<T, N>& data, Parser func) {
	auto i = 0u;
	for (auto const& child : ptree.get_child(root_tag)) {
		if (child.first != item_tag) {
			// ignore child
			continue;
		}
		func(child.second, data[i++]);
	}
}

template <typename T, std::size_t N, typename Dumper>
void dump_array(ptree_type& ptree, std::string const& root_tag,
	std::string const& item_tag, std::array<T, N> const& data, Dumper func) {
	ptree_type array;
	for (auto const& elem : data) {
		ptree_type child;
		func(child, elem);
		array.add_child(item_tag, child);
	}
	ptree.add_child(root_tag, array);
}

// --------------------------------------------------------------------

template <typename M, typename Parser>
void parse_map(ptree_type const & ptree, std::string const & root_tag,
	std::string const & item_tag, M& map, Parser func) {
	map.clear();
	for (auto const & child: ptree.get_child(root_tag)) {
		if (child.first != item_tag) {
			// ignore child
			continue;
		}
		typename M::key_type key;
		typename M::mapped_type value;
		func(child.second, key, value);
		map[key] = std::move(value);
	}
}

template <typename M, typename Dumper>
void dump_map(ptree_type& ptree, std::string const & root_tag,
	std::string const & item_tag, M const & map, Dumper func) {
	ptree_type array;
	for (auto const & pair: map) {
		ptree_type child;
		func(child, pair.first, pair.second);
		array.add_child(item_tag, child);
	}
	ptree.add_child(root_tag, array);
}

}  // ::utils
