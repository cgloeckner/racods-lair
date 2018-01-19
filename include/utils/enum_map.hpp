#pragma once
#include <limits>
#include <array>

namespace utils {

// key requires std::numeric_limits<Key>::min(); and ::max();
template <typename Key, typename Value>
class EnumMap {
  public:
	static std::size_t constexpr MinKey =
		static_cast<std::size_t>(std::numeric_limits<Key>::min());
	static std::size_t constexpr MaxKey =
		static_cast<std::size_t>(std::numeric_limits<Key>::max());
	static std::size_t constexpr NumKeys = MaxKey - MinKey + 1u;
	
	using pair = std::pair<Key, Value>;
	using container = std::array<pair, NumKeys>;
	using iterator = typename container::iterator;
	using const_iterator = typename container::const_iterator;
	
  private:
	container data;

  public:
	// fix for exposing to lua via sol
	using value_type = typename container::value_type;
	
	EnumMap();
	EnumMap(Value const& default_value);

	Value const& operator[](Key const& key) const;
	Value& operator[](Key const& key);

	std::size_t size() const;
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
};

}  // ::utils

// include implementation details
#include <utils/enum_map.inl>
