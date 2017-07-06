#include <rpg/balance.hpp>
#include <rpg/entity.hpp>

namespace rpg {

InputData::InputData()
	: core::ComponentData{}
	, keys{}
	, is_active{true}
	, auto_look{true}
	, cooldown{sf::Time::Zero} {}

ActionData::ActionData()
	: core::ComponentData{}
	, idle{true}
	, moving{false}
	, dead{false} {
}

ItemData::ItemData() : core::ComponentData{}, inventory{}, equipment{nullptr} {}

PerkData::PerkData() : core::ComponentData{}, perks{} {}

StatsData::StatsData()
	: core::ComponentData{}
	, base_props{0u}
	, prop_boni{0}
	, base_def{0.f}
	, factor{1.f}
	, godmode{false}
	, level{0u}
	, attributes{0u}
	, properties{0u}
	, stats{0u} {
}

EffectData::EffectData()
	: core::ComponentData{}, effects{}, cooldown{sf::Time::Zero} {}

QuickslotData::QuickslotData()
	: core::ComponentData{}, slot_id{0u}, slots{}, cooldown{sf::Time::Zero} {}

PlayerData::PlayerData()
	: core::ComponentData{}
	, player_id{0u}
	, minions{}
	, exp{0u}
	, base_exp{0u}
	, next_exp{getNextExp(1u)}
	, stacked_exp{0u}
	, attrib_points{0u}
	, perk_points{0u} {}

ProjectileData::ProjectileData()
	: core::ComponentData{}
	, owner{0u}
	, ignore{}
	, bullet{nullptr}
	, meta_data{} {}

InteractData::InteractData()
	: core::ComponentData{}
	, type{InteractType::Barrier}
	, moving{false}
	, loot{} {}

}  // ::game

// ---------------------------------------------------------------------------
// Template instatiations

namespace utils {

template class ComponentSystem<core::ObjectID, rpg::InputData>;
template class ComponentSystem<core::ObjectID, rpg::ActionData>;
template class ComponentSystem<core::ObjectID, rpg::ItemData>;
template class ComponentSystem<core::ObjectID, rpg::PerkData>;
template class ComponentSystem<core::ObjectID, rpg::StatsData>;
template class ComponentSystem<core::ObjectID, rpg::EffectData>;
template class ComponentSystem<core::ObjectID, rpg::QuickslotData>;
template class ComponentSystem<core::ObjectID, rpg::PlayerData>;
template class ComponentSystem<core::ObjectID, rpg::ProjectileData>;
template class ComponentSystem<core::ObjectID, rpg::InteractData>;

}  // ::utils
