#include <rpg/algorithm.hpp>
#include <game/item.hpp>

namespace game {

float const MIN_WORTH_MODIFIER = 0.8;
float const MAX_WORTH_MODIFIER = 1.1;

float const MIN_DAMAGE_MODIFIER = 0.5;
float const MAX_DAMAGE_MODIFIER = 1.1;

void randomize(float& factor, float min_base, float max_base, float exp) {
	factor = rpg::extrapolate(factor, thor::random(min_base, max_base), exp);
}

void randomize(
	std::uint32_t& factor, float min_base, float max_base, float exp) {
	factor = rpg::extrapolate(factor, thor::random(min_base, max_base), exp);
}

void randomize(rpg::ItemTemplate& item, std::size_t level) {
	randomize(item.worth, MIN_WORTH_MODIFIER, MAX_WORTH_MODIFIER, level);
	for (auto& dmg : item.damage) {
		randomize(dmg.second, MIN_DAMAGE_MODIFIER, MAX_DAMAGE_MODIFIER, level);
	}

	// [TODO] also modify:
	//	std::string display_name;
	//	EffectEmitter effect;
	//	utils::EnumMap<Attribute, int> require; // for equipment
	//	utils::EnumMap<Stat, int> recover; // by potion
	//	StatsBoni boni;
}

}  // ::rage
