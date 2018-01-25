#pragma once
#include <functional>
#include <vector>
#include <array>
#include <string>
#include <SFML/System.hpp>

#include <rpg/common.hpp>
#include <rpg/resources.hpp>
#include <rpg/gameplay.hpp>

namespace rpg {

struct InputData : core::ComponentData {
	utils::Keybinding<PlayerAction> keys;
	bool is_active, auto_look;
	sf::Time cooldown;  // about toggling auto-look

	InputData();
};

struct ActionData : core::ComponentData {
	bool idle, dead;

	ActionData();
};

struct ItemData : core::ComponentData {
	utils::EnumMap<ItemType, std::vector<Item>> inventory;
	utils::EnumMap<EquipmentSlot, ItemTemplate const *> equipment;

	ItemData();
};

struct PerkData : core::ComponentData {
	std::vector<Perk> perks;

	PerkData();
};

struct StatsData : core::ComponentData {
	// "internal" values
	utils::EnumMap<Property, std::uint32_t> base_props;
	utils::EnumMap<Property, std::int32_t> prop_boni;
	utils::EnumMap<DamageType, float> base_def;
	float factor;
	
	// debug value
	bool godmode;

	// "official" values
	std::uint32_t level;
	utils::EnumMap<Attribute, std::uint32_t> attributes;
	utils::EnumMap<Property, std::uint32_t> properties;
	utils::EnumMap<Stat, std::uint32_t> stats;

	StatsData();
};

struct EffectData : core::ComponentData {
	std::vector<Effect> effects;
	sf::Time cooldown;  // to apply effects each 250ms

	EffectData();
};

struct QuickslotData : core::ComponentData {
	std::size_t slot_id;
	std::array<Shortcut, MAX_QUICKSLOTS> slots;
	sf::Time cooldown;

	QuickslotData();
};

struct PlayerData : core::ComponentData {
	PlayerID player_id;
	std::vector<core::ObjectID> minions;
	std::uint64_t exp, base_exp, next_exp, stacked_exp;
	std::uint32_t attrib_points, perk_points;

	PlayerData();
};

struct ProjectileData : core::ComponentData {
	core::ObjectID owner;
	std::vector<core::ObjectID> ignore;  // are not damaged
	BulletTemplate const *bullet;		 // required
	CombatMetaData meta_data;

	ProjectileData();
};

struct InteractData : core::ComponentData {
	using Loot = std::vector<Item>;

	InteractType type;
	bool moving;
	std::vector<Loot> loot;  // per player

	InteractData();
};

// ---------------------------------------------------------------------------
// enhanced component managers

using InputManager = core::ComponentManager<InputData>;
using ActionManager = core::ComponentManager<ActionData>;
using ItemManager = core::ComponentManager<ItemData>;
using PerkManager = core::ComponentManager<PerkData>;
using StatsManager = core::ComponentManager<StatsData>;
using EffectManager = core::ComponentManager<EffectData>;
using QuickslotManager = core::ComponentManager<QuickslotData>;
using PlayerManager = core::ComponentManager<PlayerData>;
using ProjectileManager = core::ComponentManager<ProjectileData>;
using InteractManager = core::ComponentManager<InteractData>;

}  // ::game
