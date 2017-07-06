#pragma once
#include <rpg/common.hpp>

namespace rpg {

using AttribsMap = utils::EnumMap<Attribute, std::uint32_t>;
using PropertyMap = utils::EnumMap<Property, std::uint32_t>;

/// Number of attribute points that are achieved per levelup
std::uint32_t const ATTRIB_POINTS_PER_LEVEL = 5u;

/// Number of perk points that are achieved per levelup
std::uint32_t const PERK_POINTS_PER_LEVEL = 1u;

/// Calculate maximum life
/**
 *	This will calculate and return maximum life based on the actor's
 *	attributes and level.
 *
 *	@param attrib Attributes of the actor
 *	@param level Level of the actor
 *	@return maximum life
 */
std::uint32_t getMaxLife(AttribsMap const& attribs, std::uint32_t level);

/// Calculate maximum mana
/**
 *	This will calculate and return maximum mana based on the actor's
 *	attributes and level.
 *
 *	@param attrib Attributes of the actor
 *	@param level Level of the actor
 *	@return maximum mana
 */
std::uint32_t getMaxMana(AttribsMap const& attribs, std::uint32_t level);

/// Calculate maximum stamina
/**
 *	This will calculate and return maximum stamina based on the actor's
 *	attributes and level.
 *
 *	@param attrib Attributes of the actor
 *	@param level Level of the actor
 *	@return maximum life
 */
std::uint32_t getMaxStamina(AttribsMap const& attribs, std::uint32_t level);

/// Calculate melee base damage
/**
 *	This will calculate and return melee base damage based on the actor's
 *	attributes and level. The melee base damage is used for melee-based
 *	weapons' damage calculation and fistfighting damage calculation.
 *
 *	@param attrib Attributes of the actor
 *	@param level Level of the actor
 *	@return melee base damage
 */
std::uint32_t getMeleeBase(AttribsMap const& attribs, std::uint32_t level);

/// Calculate range base damage
/**
 *	This will calculate and return range base damage based on the actor's
 *	attributes and level. The range base damage is used for range-based
 *	weapons' damage calculation.
 *
 *	@param attrib Attributes of the actor
 *	@param level Level of the actor
 *	@return range base damage
 */
std::uint32_t getRangeBase(AttribsMap const& attribs, std::uint32_t level);

/// Calculate magic base damage
/**
 *	This will calculate and return magic base damage based on the actor's
 *	attributes and level. The magic base damage is used for perk-based damage
 *	and recovery calculation.
 *
 *	@param attrib Attributes of the actor
 *	@param level Level of the actor
 *	@return magic base damage
 */
std::uint32_t getMagicBase(AttribsMap const& attribs, std::uint32_t level);

// ----------------------------------------------------------------------------

std::uint32_t getPropertyValue(float base, std::uint32_t level);

// ----------------------------------------------------------------------------

/// Calculate stacked experience
/**
 *	This calculates how many experience points are stacked for a given
 *	scenario of actor's and target's level.
 *
 *	@param actor_level Level of the actor object, who triggered the experience
 *	@param target_level Level of the target object, who stacks the experience
 *	@param exp Initial amount of experience
 *	@return amount of stacked experience for the target
 */
std::uint64_t getStackedExp(
	std::uint32_t actor_level, std::uint32_t target_level, std::uint64_t exp);

/// Calculate gained exp
/// @param total_damage Damage or recovery caused
/// @param lvl Own level
/// @return exp gained
std::uint64_t getExpGain(std::uint32_t total_damage, std::uint32_t lvl);

/// Calculate required exp
/**
 *	This calculates how many experience is necessary to achieve the given
 *	level.
 *
 *	@param level Level that is be achieved
 *	@return required experience
 */
std::uint64_t getNextExp(std::uint32_t level);

/// Calculate weapon damage
/// @param dmg damage of the weapon
/// @param level Level of the avatgar
/// @return total damage
std::uint32_t getDamageBonus(float dmg, std::uint32_t level);

/// Calculate armor defense
/// @param base Base defense of the armor
/// @param level Level of the avatar
/// @return total defense
std::uint32_t getDefenseBonus(float def, std::uint32_t level);

/// Calculate the perk bonus
/// The perk bonus is added to a perk's damage or recovery while combat
/// calculation. It depends on the perk's base damage/recovery and level.
/// @param base Base bonus of the perk
/// @param bonus Property bonus of the actor
/// @param perk Level of the perk
/// @return total bonus
std::uint32_t getPerkBonus(float base, float bonus, std::uint32_t level);

/// Calculate the effect
/// @param value Value of the effect
/// @param level Level of the avatar
/// @return actual value
std::uint32_t getEffectValue(std::uint32_t value, std::uint32_t level);

unsigned int getPerkCosts(float dmg, std::uint32_t level);

} // ::rpg
