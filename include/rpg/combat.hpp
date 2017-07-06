#pragma once
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace combat_impl {

/// note: squared distance
extern float const MAX_MELEE_DISTANCE;

using DamageMap = utils::EnumMap<DamageType, unsigned int>;
using StatsMap = utils::EnumMap<Stat, int>;

/// Combat context
struct Context {
	core::LogContext& log;
	StatsSender& stats_sender;
	ExpSender& exp_sender;
	EffectSender& effect_sender;
	ProjectileSender& projectile_sender;
	SpawnSender& respawn_sender;

	core::MovementManager const& movement;
	ProjectileManager const& projectile;
	PerkManager const& perk;
	StatsManager const& stats;
	InteractManager const& interact;
	float const variance;
	std::vector<core::ObjectID> projectiles;

	Context(core::LogContext& log, StatsSender& stats_sender,
		ExpSender& exp_sender, EffectSender& effect_sender,
		ProjectileSender& projectile_sender, SpawnSender& respawn_sender,
		core::MovementManager const& movement,
		ProjectileManager const& projectile, PerkManager const& perk,
		StatsManager const& stats, InteractManager const& interact,
		float variance);
};

// ---------------------------------------------------------------------------

/// Query attacker's stats
/**
 *	This returns the StatData of the attacker avatar for this given
 *	CombatEvent. If the event's actor is a projectile, the projectile's owner
 *	is considered. If no actor (in terms of an avatar) exists, a nullptr is
 *	returned. This happens if the combat is triggered by a trap-based
 *	projectile or an active effect.
 *
 *	@param context Combat context to use
 *	@param event CombatEvent to query for
 *	@return Pointer to the attacker's StatsData (or nullptr if no attacker)
 */
StatsData const* getAttacker(Context const& context, CombatEvent const& event);

/// Query single weapon's damage
/**
 *	This queries the actor's weapon damage for the given weapon. The damage
 *	depends on some properties of the given actor. No randomization is applied
 *	here.
 *
 *	@param actor StatsData of the actor
 *	@param weapon ItemTemplate of the weapon
 *	@return weapon's damage
 */
DamageMap getWeaponDamage(StatsData const& actor, ItemTemplate const& weapon);

/// Query weapons' damage
/**
 *	This queries the actor's total weapon damage for either no, one or two
 *	weapons. The single weapon damages are calculated and summarizeded here.
 *	No randomization is applied. If no weapon is specified, the damage for
 *	fistfight is specified by the actor's basic melee damage as blunt damage.
 *
 *	@param actor StatsData of the actor
 *	@param primary Pointer to the primary weapon or nullptr
 *	@param secondary Pointer to the secondary weapon or nullptr
 *	@return total weapons' damage
 */
DamageMap getWeaponDamage(StatsData const& actor, ItemTemplate const* primary,
	ItemTemplate const* secondary);

/// Query perk's damage
/**
 *	This queries the actor's perk damage for the given perk. The damage
 *	depends on the perk, its level and some properties of the actor. No
 *	randomization is applied here.
 *
 *	@param perkdata PerkData of the actor
 *	@param actor StatsData of the actor
 *	@param perk PerkTemplate to query for
 *	@return perk damage
 */
DamageMap getPerkDamage(PerkData const& perk_data, StatsData const& actor,
	PerkTemplate const& perk);

/// Query effect's damage
/// This determines the actual damage for the given actor
/// @param actor StatsData of the actor
/// @param effect EffectTemplate of the effect
/// return effect damage
DamageMap getEffectDamage(StatsData const & actor, EffectTemplate const & effect);

/// Query perk's recovery
/**
 *	This queries the actor's perk recovery for the given perk. The recovery
 *	depends on the perk, its level and some properties of the actor. No
 *	randomization is applied here.
 *
 *	@param perkdata PerkData of the actor
 *	@param actor StatsData of the actor
 *	@param perk PerkTemplate to query for
 *	@return perk recovery
 */
StatsMap getPerkRecovery(PerkData const& perk_data, StatsData const& actor,
	PerkTemplate const& perk);

/// Query effect's recovery
/// This determines the actual recover for the given actor
/// @param actor StatsData of the actor
/// @param effect EffectTemplate of the effect
/// return effect recovery
StatsMap getEffectRecovery(StatsData const & actor, EffectTemplate const & effect);

/// Query damage for the given actor
/**
 *	This queries the total damage for the given actor. In case of avatar-based
 *	attacks, the corresponding functions are used to calculate either weapon
 *	or perk damage. No randomization is applied here.
 *
 *	@param context Combat context to use
 *	@param data CombatMetaData about the current combat
 *	@param actor Pointer to the actor's StatsData or nullptr
 *	@param target Reference to the target's StatsData
 *	@return total damage
 */
DamageMap getDamage(Context const& context, CombatMetaData const& data,
	StatsData const* actor, StatsData const & target);

DamageMap getDefense(StatsData const & target);

/// Query recovery for the given actor
/**
 *	This queries the total recovery for the given actor. Especially this
 *	includes effect's recovery.
 *
 *	@param context Combat context to use
 *	@param data CombatMetaData about the current combat
 *	@param actor Pointer to the actor's StatsData or nullptr
 *	@param target Reference to the target's StatsData
 *	@return total recovery
 */
StatsMap getRecovery(Context const& context, CombatMetaData const& data,
	StatsData const* actor, StatsData const & target);

/// Query all posible EffectEmitters
/**
 *	This queries all possible EffectEmitters. In most cases the returning
 *	vector has zero or one element. In case of two-weapons-based combat where
 *	two weapons might inflict effects, the vector might contain two elements.
 *
 *	@param context Combat context to use
 *	@return vector of EffectEmitters
 */
std::vector<EffectEmitter> getEffectEmitters(CombatMetaData const& data);

/// Randomize an integer value
/**
 *	This applies in-place randomization to a given integer. The resulting
 *	value is varied by variance specified in the context.
 *
 *	@param context Combat context to use
 *	@param value Will be randomized in-place
 */
void randomize(Context const& context, int& value);

/// Handle the given combat
/**
 *	This will calculate the given combat and propagate all necessary events.
 *	Damage and recovery will be calculated, randomized and applied through
 *	StatsEvent. If possible, the actor will gain experience and can inflict
 *	effect(s) to its target.
 *
 *	@param context Combat context to use
 *	@param event CombatEvent to handle
 */
void onCombat(Context& context, CombatEvent const& event);

/// Update and cleanup
/**
 *	This will update and cleanup the system by triggering projectiles'
 *	destruction after their combats were calculated and applied.
 *
 *	@param context Combat context to use
 *	@param elapsed Time since last frame
 */
void onUpdate(Context& context, sf::Time const& elapsed);

}  // ::combat_impl

// ---------------------------------------------------------------------------

/// The CombatSystem is used to perform combat calculations
/**
 *	CombatEvents are handled by combat calculations. Each calculation results
 *	in a StatsEvent. Additional, an ExpEvent and EffectEvent can be propagated
 *	as well.
 *	After a projectile exploded and the corresponding combat was calculated,
 *	the projectile's id is stored. Later, its destruction is triggered via a
 *	ProjectileEvent.
 *	The CombatSystem does not contain object-specific data like other systems
 *	do.
 */
class CombatSystem
	// Event API
	: public utils::EventListener<CombatEvent>,
	  public utils::EventSender<StatsEvent, ExpEvent, EffectEvent,
		  ProjectileEvent, SpawnEvent> {

  private:
	combat_impl::Context context;

  public:
	CombatSystem(core::LogContext& log, core::MovementManager const& movement,
		ProjectileManager const& projectile, PerkManager const& perk,
		StatsManager const& stats, InteractManager const& interact,
		float variance);

	void handle(CombatEvent const& event);

	void update(sf::Time const& elapsed);
	
	void clear();
};

}  // ::game
