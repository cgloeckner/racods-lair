#include <algorithm>

#include <Thor/Math.hpp>
#include <core/collision.hpp>
#include <rpg/algorithm.hpp>

namespace rpg {

float extrapolate(float factor, float base, float exp) {
	return factor * std::pow(base, exp);
}

std::uint32_t extrapolateFloor(float factor, float base, float exp) {
	return static_cast<std::uint32_t>(std::floor(extrapolate(factor, base, exp)));
}

std::uint32_t extrapolateCeil(float factor, float base, float exp) {
	return static_cast<std::uint32_t>(std::ceil(extrapolate(factor, base, exp)));
}

std::uint64_t extrapolateExp(std::uint64_t factor, float base, float exp) {
	return static_cast<std::uint64_t>(std::round(extrapolate(factor, base, exp)));
}

}  // ::game
