#pragma once
#include <typeinfo>
#include <string>
#include <limits>
#include <ostream>
#include <boost/preprocessor.hpp>

#define ENUM_TO_STRING(unused, name, elem) \
	case name::elem:                       \
		return BOOST_PP_STRINGIZE(elem);

#define ENUM_FROM_STRING(unused, name, elem) \
	if (value == BOOST_PP_STRINGIZE(elem)) { \
		return name::elem;                   \
	}

// ENUM(OS, Linux, (Linux)(Apple)(Windows))
/*
	enum class OperatingSystem {
		Linux, Apple, Windows
	};

	string to_string(OS) { ... }
	OS from_string(string) { ... }
*/
#define ENUM(name, default_val, enumerators)                          \
	template <typename T>                                             \
	T from_string(std::string const& str);                            \
	template <typename T>                                             \
	T default_value();                                                \
	enum class name : std::size_t { BOOST_PP_SEQ_ENUM(enumerators) }; \
	inline std::string to_string(name value) {                        \
		switch (value) {                                              \
			BOOST_PP_SEQ_FOR_EACH(ENUM_TO_STRING, name, enumerators)  \
		}                                                             \
		throw std::bad_cast();                                        \
	}                                                                 \
	template <>                                                       \
	inline name from_string(std::string const& value) {               \
		BOOST_PP_SEQ_FOR_EACH(ENUM_FROM_STRING, name, enumerators)    \
		throw std::bad_cast();                                        \
	}                                                                 \
	template <>                                                       \
	inline name default_value() {                                     \
		return name::default_val;                                     \
	}																\

#define ENUM_STREAM(name)											\
	inline std::ostream& operator<<(std::ostream& lhs, name value) {\
		return lhs << to_string(value); 							\
	}

// Define limits using std::numeric_limits<T>::min() and ::max()
#define SET_ENUM_LIMITS(min_, max_)                                           \
	static_assert(min_ <= max_, "Minimum <= Maximum");                        \
	namespace std {                                                           \
	template <>                                                               \
	constexpr decltype(min_) numeric_limits<decltype(min_)>::min() noexcept { \
		return min_;                                                          \
	}                                                                         \
	template <>                                                               \
	constexpr decltype(max_) numeric_limits<decltype(max_)>::max() noexcept { \
		return max_;                                                          \
	}                                                                         \
	}

namespace utils {

template <typename T>
struct EnumRange {
	class iterator {
	  private:
		std::size_t value;

	  public:
		iterator(std::size_t value);
		T operator*() const;
		void operator++();
		bool operator!=(iterator const& other) const;
		bool operator==(iterator const& other) const;
	};
	iterator begin() const;
	iterator end() const;
};

template <typename T>
std::size_t constexpr getEnumCount() noexcept;

}  // ::utils

// include implementation details
#include <utils/enum_utils.inl>
