#pragma once
#include <rpg/resources.hpp>

namespace game {

void randomize(float& factor, float min_base, float max_base, float exp);
void randomize(
	std::uint32_t& factor, float min_base, float max_base, float exp);

void randomize(rpg::ItemTemplate& item, std::size_t level);

}  // ::rage
