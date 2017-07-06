#pragma once
#include <string>
#include <vector>
#include <array>
#include <boost/property_tree/ptree.hpp>

namespace utils {

using ptree_type = boost::property_tree::iptree;
using ptree_error = boost::property_tree::ptree_error;

template <typename T, typename Parser>
void parse_vector(ptree_type const& ptree, std::string const& root_tag,
	std::string const& item_tag, std::vector<T>& data, Parser func);

template <typename T, typename Parser>
void parse_vector(ptree_type const& ptree, std::string const& item_tag, std::vector<T>& data, Parser func);

template <typename T, typename Dumper>
void dump_vector(ptree_type& ptree, std::string const& root_tag,
	std::string const& item_tag, std::vector<T> const& data, Dumper func);

template <typename T, typename Dumper>
void dump_vector(ptree_type& ptree, std::string const& item_tag, std::vector<T> const& data, Dumper func);

// ---------------------------------------------------------------------------

template <typename T, std::size_t N, typename Parser>
void parse_array(ptree_type const& ptree, std::string const& root_tag,
	std::string const& item_tag, std::array<T, N>& data, Parser func);

template <typename T, std::size_t N, typename Dumper>
void dump_array(ptree_type& ptree, std::string const& root_tag,
	std::string const& item_tag, std::array<T, N> const& data, Dumper func);

// ---------------------------------------------------------------------------

template <typename M, typename Parser>
void parse_map(ptree_type const & ptree, std::string const & root_tag,
	std::string const & item_tag, M& map, Parser func);

template <typename M, typename Dumper>
void dump_map(ptree_type & ptree, std::string const & root_tag,
	std::string const & item_tag, M const & map, Dumper func);

}  // ::utils

// include implementation details
#include <utils/xml_utils.inl>
