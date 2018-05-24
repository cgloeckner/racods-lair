#pragma once
#include <cstdint>

#include <utils/enum_utils.hpp>
#include <utils/enum_map.hpp>

#include <core/dungeon.hpp>

namespace rpg {

using PlayerID = std::size_t;

/// Number of quickslots
std::size_t const MAX_QUICKSLOTS = 10u;

/// Max Interact distance (squared!)
float const MAX_DISTANCE = 1.5f * 1.5f;

struct SpawnMetaData {
	utils::SceneID scene;
	sf::Vector2f pos, direction;

	SpawnMetaData();
};

// ---------------------------------------------------------------------------
// enumerations

/// actions that are triggered by each player
/**	Example Usage:
 *	- L Stick: Move (+ Look if Auto-Look on)
 *	- R Stick: Look (if Auto-Look off)
 *	- L+R Stick: Strafe
 *	- Dreieck: Use Slot (Item, Perk)
 *	- Kreis: Attack (Weapon)
 *	- Kreuz: Interact (Chest, Corpse etc.)
 *	- Viereck: ---
 *	- L1: Prev Slot
 *	- R1: Next Slot
 *	- L2: ---
 *	- R2: ---
 *	- Start: Pause Menu
 *	- Select: Toggle Auto-Look
 */
ENUM(PlayerAction, ToggleAutoLook,
	(MoveN)(MoveS)(MoveW)(MoveE)(LookN)(LookS)(LookW)(LookE)(Attack)(Interact)(Pause)(UseSlot)(PrevSlot)(NextSlot)(ToggleAutoLook))

// referring to equipment (extension: shield or second one-handed weapon)
ENUM(EquipmentSlot, None, (None)(Weapon)(Extension)(Body)(Head))

// referring to inventory page
ENUM(ItemType, Misc, (Weapon)(Armor)(Potion)(Misc))

// used to specify whether offensive/defensive/etc. perk
ENUM(PerkType, Self, (Self)(Enemy)(Allied))

ENUM(DamageType, Blade, (Blade)(Blunt)(Bullet)(Magic)(Fire)(Ice)(Poison))

ENUM(Stat, Life, (Life)(Mana)(Stamina))

ENUM(Attribute, Strength, (Strength)(Dexterity)(Wisdom))

ENUM(Property, MaxLife,
	(MaxLife)(MaxMana)(MaxStamina)(MeleeBase)(RangeBase)(MagicBase))

ENUM(EmitterType, Weapon, (Weapon)(Perk)(Effect)(Trap))

ENUM(InteractType, Barrier, (Barrier)(Corpse))

ENUM(FeedbackType, ItemNotFound, (ItemNotFound)(CannotUseThis)
	(EmptyShortcut)(NotEnoughMana)(NotEnoughAttribPoints)
	(NotEnoughPerkPoints))

}  // ::game

ENUM_STREAM(rpg::PlayerAction)
ENUM_STREAM(rpg::EquipmentSlot)
ENUM_STREAM(rpg::ItemType)
ENUM_STREAM(rpg::PerkType)
ENUM_STREAM(rpg::DamageType)
ENUM_STREAM(rpg::Stat)
ENUM_STREAM(rpg::Attribute)
ENUM_STREAM(rpg::Property)
ENUM_STREAM(rpg::EmitterType)
ENUM_STREAM(rpg::InteractType)
ENUM_STREAM(rpg::FeedbackType)

SET_ENUM_LIMITS(rpg::PlayerAction::MoveN, rpg::PlayerAction::ToggleAutoLook)
SET_ENUM_LIMITS(rpg::EquipmentSlot::None, rpg::EquipmentSlot::Head)
SET_ENUM_LIMITS(rpg::ItemType::Weapon, rpg::ItemType::Misc)
SET_ENUM_LIMITS(rpg::PerkType::Self, rpg::PerkType::Allied)
SET_ENUM_LIMITS(rpg::DamageType::Blade, rpg::DamageType::Poison)
SET_ENUM_LIMITS(rpg::Stat::Life, rpg::Stat::Stamina)
SET_ENUM_LIMITS(rpg::Attribute::Strength, rpg::Attribute::Wisdom)
SET_ENUM_LIMITS(rpg::Property::MaxLife, rpg::Property::MagicBase)
SET_ENUM_LIMITS(rpg::EmitterType::Weapon, rpg::EmitterType::Trap)
SET_ENUM_LIMITS(rpg::InteractType::Barrier, rpg::InteractType::Corpse)
SET_ENUM_LIMITS(rpg::FeedbackType::ItemNotFound, rpg::FeedbackType::NotEnoughPerkPoints)

namespace rpg {}  // ::game
