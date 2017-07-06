#pragma once
#include <rpg/common.hpp>
#include <rpg/resources.hpp>

namespace rpg {

// ---------------------------------------------------------------------------
// gameplay objects

/// Perk
/**
 *	Describes a perk within an avatar's set of perks. Each perk refers to a
 *	const and shared but not owned perk template. Also, each perk has a level,
 *	which determines the mana consume and the perk's damage etc.
 */
struct Perk {
	PerkTemplate const *perk;
	std::size_t level;

	Perk();
	Perk(PerkTemplate const &perk, std::size_t level = 1u);
};

/// Item
/**
 *	Describes an item within an avatar's inventory. Each item refers to a
 *	const and shared but not owned item template. Also, each item has a
 *	quantity counter, which is used to stack items of the same item template.
 */
struct Item {
	ItemTemplate const *item;
	std::size_t quantity;

	Item();
	Item(ItemTemplate const &item, std::size_t quantity = 1u);
};

/// Shortcut
/**
 *	Describes a shortcut within an avatar's quickslot bar. Each shortcut holds
 *	either a perk or an item or nothing. In case the shortcut is not empty,
 *	a const and shared but not owned (either item or perk) template is
 *	specified.
 */
struct Shortcut {
	PerkTemplate const *perk;
	ItemTemplate const *item;

	Shortcut();
	Shortcut(PerkTemplate const &perk);
	Shortcut(ItemTemplate const &item);
};

/// Effect
/**
 *	Describes an effect within an avatar's list of active effects. Each effect
 *	refers to a const and shared but non owned effect template. Also, an
 *	effect has a counter for its remaining time.
 */
struct Effect {
	EffectTemplate const *effect;
	sf::Time remain;

	Effect();
	Effect(EffectTemplate const &effect);
};

/// Combat Metadata
/**
 *	This data is interchanged between different events. It holds detailed
 *	information about a scheduled combat. The EmitterType describes, whether
 *	the combat/damage is triggered by a projectile, an effect or directly
 *	through non-bullet weapon or perk. The primary (and possibly secondary)
 *	weapons are stored in case of weapon-based combat (despite a bullet was
 *	created or not). This is used to determine the original bow used if the
 *	actor changed its weapon after shooting. In case the combat was caused by
 *	a trap, the trap template is specified.
 *	All referenced templates are const and shared, but are not owning
 *	pointers.
 */
struct CombatMetaData {
	EmitterType emitter;
	ItemTemplate const *primary, *secondary;
	PerkTemplate const *perk;
	EffectTemplate const *effect;
	TrapTemplate const *trap;

	CombatMetaData();
};

}  // ::game
