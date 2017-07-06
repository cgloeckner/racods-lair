#pragma once
#include <utils/enum_map.hpp>

namespace rpg {

/// Extrapolate values
/**
 *	Extrapolation is done by factor * base ^ exp. The resulting value is
 *	floored or ceiled and returned as integer value.
 *
 *	@param factor Factor to multiply the power with
 *	@param base Base to exponentiate
 *	@param exp Exp to exponentiate
 *	@return extrapolated value
 */
float extrapolate(float factor, float base, float exp);
std::uint32_t extrapolateFloor(float factor, float base, float exp);
std::uint32_t extrapolateCeil(float factor, float base, float exp);

/// Extrapolation overload for unsigned int64 factors (e.g. player experience)
std::uint64_t extrapolateExp(std::uint64_t factor, float base, float exp);

// ---------------------------------------------------------------------------

/// Add two integer-based enum maps
/**
 *	This adds all values of rhs to the corresponding values of lhs. The
 *	addition is done component-wise referring to enum values. lhs is changed
 *	in place. If the resulting lhs component would exceed zero, the value is
 *	cut to zero.
 *
 *	@param lhs Target and lhs operator
 *	@param rhs Rhs operator
 */
template <typename T>
void operator+=(
	utils::EnumMap<T, std::uint32_t>& lhs, utils::EnumMap<T, int> const& rhs);

/// Add two integer-based enum maps
/**
 *	This adds all values of rhs to the corresponding values of lhs. The
 *	addition is done component-wise referring to enum values. lhs is changed
 *	in place.
 *
 *	@param lhs Target and lhs operator
 *	@param rhs Rhs operator
 */
template <typename T>
void operator+=(utils::EnumMap<T, std::uint32_t>& lhs,
	utils::EnumMap<T, std::uint32_t> const& rhs);

/// Add two integer-based enum maps
/**
 *	This adds all values of rhs to the corresponding values of lhs. The
 *	addition is done component-wise referring to enum values. lhs is changed
 *	in place.
 *
 *	@param lhs Target and lhs operator
 *	@param rhs Rhs operator
 */
template <typename T>
void operator+=(utils::EnumMap<T, int>& lhs, utils::EnumMap<T, int> const& rhs);

/// Add two float-based enum maps
/// This adds all values of rhs to the corresponding values of lhs. The
/// addition is done component-wise referring to enum values. lhs is changed
/// in place.
/// @param lhs Target and lhs operator
/// @param rhs Rhs operator
template <typename T>
void operator+=(utils::EnumMap<T, float>& lhs, utils::EnumMap<T, float> const& rhs);

/// Subtract two integer-based enum maps
/**
 *	This adds all values of rhs to the corresponding values of lhs. The
 *	subtraction is done component-wise referring to enum values. lhs is
 *	changed in place.
 *
 *	@param lhs Target and lhs operator
 *	@param rhs Rhs operator
 */
template <typename T>
void operator-=(
	utils::EnumMap<T, std::uint32_t>& lhs, utils::EnumMap<T, int> const& rhs);

/// Subtract two float-based enum maps
/// @param lhs Target and lhs operator
/// @param rhs Rhs operator
template <typename T>
void operator-=(utils::EnumMap<T, std::uint32_t>& lhs, utils::EnumMap<T, float> const& rhs);

}  // ::game

// include implementation details
#include <rpg/algorithm.inl>
