#include <rpg/balance.hpp>
#include <game/player.hpp>

namespace game {

PlayerTemplate createPlayer(std::string const & charname, std::uint32_t level) {
	PlayerTemplate tpl;
	tpl.display_name = charname;
	tpl.entity_name = "human";
	// todo: consider selected class
	tpl.level = level;
	tpl.exp = rpg::getNextExp(level);
	tpl.attributes[rpg::Attribute::Strength] = 10u;
	tpl.attributes[rpg::Attribute::Dexterity] = 5u;
	tpl.attributes[rpg::Attribute::Wisdom] = 5u;
	
	return tpl;
}

bool preparePlayer(PlayerTemplate& profile, Mod& mod, std::string const& fname) {
	PlayerTemplate tmp{profile};
	try {
		// load general data
		tmp.entity = &mod.get<rpg::EntityTemplate>(tmp.entity_name);
		// load inventory
		for (auto& node : tmp.inventory) {
			std::get<2>(node) = &mod.get<rpg::ItemTemplate>(std::get<0>(node));
		}
		// load equipment
		for (auto const& node : tmp.equipment) {
			if (node.second.empty()) {
				continue;
			}
			tmp.equip_ptr[node.first] =
				&mod.get<rpg::ItemTemplate>(node.second);
		}
		// load perks
		for (auto& node : tmp.perks) {
			std::get<2>(node) = &mod.get<rpg::PerkTemplate>(std::get<0>(node));
		}
		// load shortcuts
		for (auto& node : tmp.slots) {
			auto item_name = std::get<0>(node);
			auto perk_name = std::get<1>(node);
			if (!item_name.empty()) {
				std::get<2>(node) = &mod.get<rpg::ItemTemplate>(item_name);
			} else if (!perk_name.empty()) {
				std::get<3>(node) = &mod.get<rpg::PerkTemplate>(perk_name);
			}
		}
	} catch (utils::ptree_error const& error) {
		std::cerr << "[Game/Player] " << error.what() << "\n";
		return false;
	}

	// swap actual profile data
	std::swap(tmp, profile);
	return true;
}

}  // ::rage
