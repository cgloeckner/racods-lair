#include <utils/math2d.hpp>
#include <rpg/algorithm.hpp>
#include <rpg/balance.hpp>

namespace rpg {

std::uint32_t getMaxLife(AttribsMap const& attribs, std::uint32_t level) {
	auto base = 2 * attribs[Attribute::Strength]
				+ 1 * attribs[Attribute::Dexterity]
				+ 1 * attribs[Attribute::Wisdom];
	return extrapolateFloor(base, 1.01f, level);
}

std::uint32_t getMaxMana(AttribsMap const& attribs, std::uint32_t level) {
	auto base = 1 * attribs[Attribute::Strength]
				+ 1 * attribs[Attribute::Dexterity]
				+ 2 * attribs[Attribute::Wisdom];
	return extrapolateFloor(base, 1.01f, level);
}

std::uint32_t getMaxStamina(AttribsMap const& attribs, std::uint32_t level) {
	auto base = 1 * attribs[Attribute::Strength]
				+ 2 * attribs[Attribute::Dexterity]
				+ 1 * attribs[Attribute::Wisdom];
	return extrapolateFloor(base, 1.01f, level);
}

std::uint32_t getMeleeBase(AttribsMap const& attribs, std::uint32_t level) {
	return extrapolateFloor(attribs[Attribute::Strength] / 5.f, 1.01f, level);
}

std::uint32_t getRangeBase(AttribsMap const& attribs, std::uint32_t level) {
	return extrapolateFloor(attribs[Attribute::Dexterity] / 5.f, 1.01f, level);
}

std::uint32_t getMagicBase(AttribsMap const& attribs, std::uint32_t level) {
	return extrapolateFloor(attribs[Attribute::Wisdom] / 5.f, 1.01f, level);
}

// --------------------------------------------------------------------

std::uint32_t getPropertyValue(float base, std::uint32_t level) {
	return extrapolateFloor(base * level / 5.f, 1.0025f, level);
}

// --------------------------------------------------------------------

std::uint64_t getStackedExp(
	std::uint32_t actor_level, std::uint32_t target_level, std::uint64_t exp) {
	auto delta_level = utils::distance(actor_level, target_level);
	if (delta_level > 10u) {
		exp = extrapolateExp(exp, 0.85f, delta_level - 10u);
	}
	return exp;
}

std::uint64_t getExpGain(std::uint32_t total_damage, std::uint32_t lvl) {
	return std::ceil(2.f * total_damage / std::sqrt(lvl + 1.f));
}

std::uint64_t getNextExp(std::uint32_t level) {
	std::uint64_t tmp = level * 10u;
	return tmp * tmp;
}

std::uint32_t getDamageBonus(float dmg, std::uint32_t level) {
	return extrapolateCeil(dmg, 1.005f, level);
}

std::uint32_t getDefenseBonus(float def, std::uint32_t level) {
	return extrapolateFloor(def, 1.005f, level);
}

std::uint32_t getPerkBonus(float base, float bonus, std::uint32_t level) {
	// note: use ceil instead floor
	return extrapolateCeil(base * bonus, 1.005f, level);
}

std::uint32_t getEffectValue(std::uint32_t value, std::uint32_t level) {
	// note: use ceil instead floor
	return extrapolateCeil(value * level / 5.f, 1.005f, level);
}

unsigned int getPerkCosts(float dmg, std::uint32_t level) {
	// note: use ceil instead floor
	return extrapolateCeil(dmg, 1.5f, std::sqrt(1.f * level));
}

}  // ::game
