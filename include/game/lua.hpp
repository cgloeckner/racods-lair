#pragma once
#include <utils/lua_utils.hpp>

#include <core/common.hpp>
#include <core/entity.hpp>
#include <core/event.hpp>
#include <rpg/action.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>
#include <rpg/resources.hpp>
#include <rpg/session.hpp>
#include <game/common.hpp>
#include <game/event.hpp>
#include <game/path.hpp>
//#include <game/tracer.hpp>

namespace game {

/// @note out of order, needs reimplementation
/*
extern float const ENEMY_SPOT_SIGHT_RADIO;

struct ScriptData;

using ScriptManager = core::ComponentManager<ScriptData>;

// --------------------------------------------------------------------

/// Shared with lua script
struct LuaApi {
	core::LogContext& log;
	core::ObjectID const id;
	bool hostile;
	rpg::Session const& session;
	ScriptManager const& script;
	core::InputSender& input_sender;
	rpg::ActionSender& action_sender;
	rpg::ItemSender& item_sender;

	PathTracer tracer;

	std::future<Path> request;
	std::vector<sf::Vector2u> path;

	/// Create an instance of the LuaApi
	/// 
	/// 	The instance deals with a specific entity, which is identified by
	/// 	the given actor id.
	/// 
	/// 	@param log Reference to the logging context
	/// 	@param actor Object id of the actor
	/// 	@param hostile Determines whether enemy or player's minion
	/// 	@param session Session reference to access additional data
	/// 	@param script ScriptManager reference to query for other AIs
	/// 	@param input_sender InputEvent sender that is bound
	/// 	@param action_sender ActionEvent sender that is bound
	/// 	@param path Reference to PathSystem that will be used
	/// 
	LuaApi(core::LogContext& log, core::ObjectID actor, bool hostile,
		rpg::Session const& session, ScriptManager const& script,
		core::InputSender& input_sender, rpg::ActionSender& action_sender,
		rpg::ItemSender& item_sender, PathSystem& path);

	/// Query whether object is hostile towards players
	bool isHostile(core::ObjectID target) const;

	/// Returns the moving vector of the given object
	sf::Vector2f getMove(core::ObjectID target) const;

	/// Query whether actor has a path
	/// 
	/// 	@return true if the actor as either a pending or a tracing path
	/// 
	bool hasPath() const;

	/// Query whether path leads to the given position
	/// 
	/// 	@param pos Position that is checked for being the path's target
	/// 	@return true if the path leads to this position
	/// 
	bool isPathTarget(sf::Vector2u const & pos) const;

	/// Query tile position of the specified object
	/// 
	/// 	The object's current tile position is returned.
	/// 
	/// 	@param id Object id to query for
	/// 	@return tile position
	/// 
	sf::Vector2f getPosition(core::ObjectID id) const;

	/// Query scene id of the object
	utils::SceneID getScene(core::ObjectID id) const;

	/// Calculate looking direction to given object
	sf::Vector2f getDirection(core::ObjectID id) const;

	/// Query actor's focus
	/// 
	/// 	Returns the id of the object which is currently focused by the actor.
	/// 	If zero is returned, no object is currently focused.
	/// 
	/// 	@return object if of focused object
	/// 
	core::ObjectID getFocus() const;

	/// Calculate tile distance between actor and given object
	/// 
	/// 	The tile distance is calculated and returned. This distance
	/// 	is close to the Euclidian distance, but not referring to the
	/// 	beeline.
	/// 
	/// 	@param other Another object's id
	/// 	@return distance between actor and `other`
	/// 
	float getDistance(core::ObjectID other) const;
	
	/// Return maximum sight distance
	float getSight() const;

	/// Query all available enmies
	/// 
	/// 	This will query all enemies that are located inside the actor's sight.
	/// 	All enemies object ids are returned. Keep in mind that this operation
	/// 	is more expensive for players' minions than for hostile enemies.
	/// 
	/// 	@return array of object ids
	/// 
	std::vector<core::ObjectID> getEnemies() const;

	/// Query all available allies
	/// 
	/// 
	/// 	This will query all allies that are located inside the actor's sight.
	/// 	All allied objects' ids are returned. Keep in mind that this operation
	/// 	is more expensive for hostile bots than for players' minions
	/// 
	/// 	@return array of object ids
	/// 
	std::vector<core::ObjectID> getAllies() const;

	/// Query the actor's stats
	/// 
	/// 	This returns a const reference to the actor's stats data.
	/// 
	/// 	@return const reference to stats data
	/// 
	rpg::StatsData const& getStats() const;
	
	/// Query whether object is alive
	bool isAlive(core::ObjectID id) const;

	/// Query the given weapon's damage
	/// 
	/// 	This returns the given weapon's damage if it would be used by the
	/// 	actor. It can be calculated despite the actor owns such an item or
	/// 	not.
	/// 
	/// 	@param item Item template of the weapon
	/// 	@return calculated damage map
	/// 
	utils::EnumMap<rpg::DamageType, unsigned int> getWeaponDamage(rpg::ItemTemplate const & item) const;

	/// Query the given perk's damage
	/// 
	/// 	This returns the given perk's damage if it would be used by the
	/// 	actor. It can be calculated despite the actor learned the perk or not.
	/// 
	/// 	@param perk Perk template to calculate for
	/// 	@return calculated damage map
	/// 
	utils::EnumMap<rpg::DamageType, unsigned int> getPerkDamage(rpg::PerkTemplate const & perk) const;

	/// Query the given perk's recovery
	/// 
	/// 	This returns the given perk's recovery if it would be used by the
	/// 	actor. It can be calculated despite the actor learned the perk or not.
	/// 
	/// 	@param perk Perk template to calculate for
	/// 	@return calculated recovery map
	/// 
	utils::EnumMap<rpg::Stat, int> getPerkRecovery(rpg::PerkTemplate const & perk) const;

	/// Query item within equipment slot
	/// 
	/// 	This returns a pointer to the item template, which is located at the
	/// 	specified equipment slot. If not item is found, a nullptr is returned.
	/// 
	/// 	@param slot Equipment slot to query at
	/// 	@return pointer to item template or nullptr
	/// 
	rpg::ItemTemplate const* getEquipment(rpg::EquipmentSlot slot) const;

	/// Query all weapons from inventory
	/// 
	/// 	This returns all weapons from the actor's inventory, including each
	/// 	items' quantity.
	/// 
	/// 	@return all weapons from the inventory
	/// 
	std::vector<rpg::Item> getWeapons() const;

	/// Query all armors from inventory
	/// 
	/// 	This returns all armors from the actor's inventory, including each
	/// 	items' quantity.
	/// 
	/// 	@return all armors from the inventory
	/// 
	std::vector<rpg::Item> getArmors() const;

	/// Query all potions from inventory
	/// 
	/// 	This returns all potions from the actor's inventory, including each
	/// 	items' quantity.
	/// 
	/// 	@return all potions from the inventory
	/// 
	std::vector<rpg::Item> getPotions() const;

	/// Query all perks that were learned
	/// 
	/// 	This returns all perks that were learned by the actor, including each
	/// 	perks' level.
	/// 
	/// 	@return all perks learned by the actor
	/// 
	std::vector<rpg::Perk> getPerks() const;

	/// Move the actor to the given position
	/// 
	/// 	This triggers pathfinding towards the specified target position. The
	/// 	actual movement will start automatically once the calculated path is
	/// 	received and traversed using `update()`.
	/// 	Once another navigation move has been triggered but not performed yet,
	/// 	the previous request is replaced.
	/// 
	/// 	@param target Target tile position to go to
	/// 
	void navigate(sf::Vector2f const & target);

	/// Move the actor to the given direction
	/// The actor is triggered to move into the given direction. Once a
	/// navigation move has been triggered but not performed yet, each
	/// movement request is dropped.
	/// @param dir Movement direction to go to
	void move(sf::Vector2f dir);
	void moveTowards(sf::Vector2f pos);

	/// Turn the actor to look into the given direction
	/// This will make the actor face the given direction.
	/// @param dir Face direction to look to
	void look(sf::Vector2f dir);
	void lookTowards(sf::Vector2f pos);

	/// Stop current movement
	/// 
	/// 	This will stop the current movement and reset the path.
	/// 
	void stop();

	/// Start a single attack
	/// 
	/// 	This will trigger a single attack. The current weapon is used for
	/// 	this. No target is specified, because the target is implicitly
	/// 	selected by moving and facing.
	/// 
	void attack();

	/// Use an item
	/// 
	/// 	This will create an event for item usage. Whether the item can be
	/// 	used is not checked here; this is up to the item system.
	/// 
	/// 	@param item Item templat to use
	/// 
	void useItem(rpg::ItemTemplate const & item);

	/// Use a perk
	/// 
	/// 	This will create an event for perk usage. Whether the perk can be
	/// 	used is not checked here; this is up to the perk system.
	/// 
	/// 	@param perk Perk templat to use
	/// 
	void usePerk(rpg::PerkTemplate const & perk);

	/// Update the bot's internal data
	/// 
	/// 	Its main purpose is to handle movement traversal.
	/// 
	/// 	@param elapsed Elapsed time since last frame
	/// 	@return PathFailedEvent or boost::none
	/// 
	boost::optional<PathFailedEvent> update(sf::Time const& elapsed);
};

}  // ::game


// ---------------------------------------------------------------------------
// Binder for several types which are supposed to be used from lua, too

namespace utils {

template <>
struct Binder<sf::Vector2u> {
	static void execute(sol::state& lua) {
		lua.new_usertype<sf::Vector2u>("Position",
			sol::constructors<sol::types<>, sol::types<unsigned int, unsigned int>>(),
			"x", &sf::Vector2u::x,
			"y", &sf::Vector2u::y
		);
	}
};

template <>
struct Binder<sf::Vector2i> {
	static void execute(sol::state& lua) {
		lua.new_usertype<sf::Vector2i>("Direction",
			sol::constructors<sol::types<>, sol::types<int, int>>(),
			"x", &sf::Vector2i::x,
			"y", &sf::Vector2i::y
		);
	}
};

template <>
struct Binder<rpg::EquipmentSlot> {
	static void execute(sol::state& lua) {
		lua.new_enum("EquipmentSlot",
			"None", rpg::EquipmentSlot::None,
			"Weapon", rpg::EquipmentSlot::Weapon,
			"Extension", rpg::EquipmentSlot::Extension,
			"Body", rpg::EquipmentSlot::Body,
			"Head", rpg::EquipmentSlot::Head
		);
	}
};

template <>
struct Binder<rpg::ItemType> {
	static void execute(sol::state& lua) {
		lua.new_enum("ItemType",
			"Weapon", rpg::ItemType::Weapon,
			"Armor", rpg::ItemType::Armor,
			"Potion", rpg::ItemType::Potion,
			"Misc", rpg::ItemType::Misc
		);
	}
};

template <>
struct Binder<rpg::PerkType> {
	static void execute(sol::state& lua) {
		lua.new_enum("PerkType",
			"Self", rpg::PerkType::Self,
			"Enemy", rpg::PerkType::Enemy,
			"Allied", rpg::PerkType::Allied
		);
	}
};

template <>
struct Binder<rpg::FeedbackType> {
	static void execute(sol::state& lua) {
		lua.new_enum("FeedbackType",
			"ItemNotFound", rpg::FeedbackType::ItemNotFound,
			"CannotUseThis", rpg::FeedbackType::CannotUseThis,
			"EmptyShortcut", rpg::FeedbackType::EmptyShortcut,
			"NotEnoughMana", rpg::FeedbackType::NotEnoughMana,
			"NotEnoughAttribPoints", rpg::FeedbackType::NotEnoughAttribPoints,
			"NotEnoughPerkPoints", rpg::FeedbackType::NotEnoughPerkPoints
		);
	}
};

template <>
struct Binder<rpg::StatsBoni> {
	static void execute(sol::state& lua) {
		lua.new_usertype<rpg::StatsBoni>("StatsBoni",
			// member variables
			"properties", &rpg::StatsBoni::properties,
			"defense", &rpg::StatsBoni::defense
		);
	}
};

template <>
struct Binder<rpg::EffectTemplate> {
	static void execute(sol::state& lua) {
		lua.new_usertype<rpg::EffectTemplate>("Effect",
			// member variables
			"internal_name", sol::readonly(&rpg::BaseResource::internal_name),
			"duration", sol::readonly(&rpg::EffectTemplate::duration),
			"boni", sol::readonly(&rpg::EffectTemplate::boni),
			"recover", sol::readonly(&rpg::EffectTemplate::recover),
			"damage", sol::readonly(&rpg::EffectTemplate::damage)
		);
	}
};

template <>
struct Binder<rpg::ItemTemplate> {
	static void execute(sol::state& lua) {
		lua.new_usertype<rpg::ItemTemplate>("Item",
			// member variables
			"internal_name", sol::readonly(&rpg::BaseResource::internal_name),
			"type", sol::readonly(&rpg::ItemTemplate::type),
			"slot", sol::readonly(&rpg::ItemTemplate::slot),
			"melee", sol::readonly(&rpg::ItemTemplate::melee),
			"two_handed", sol::readonly(&rpg::ItemTemplate::two_handed),
			"worth", sol::readonly(&rpg::ItemTemplate::worth),
			"worth", sol::readonly(&rpg::ItemTemplate::worth),
			"damage", sol::readonly(&rpg::ItemTemplate::damage),
			"require", sol::readonly(&rpg::ItemTemplate::require),
			"recover", sol::readonly(&rpg::ItemTemplate::recover),
			"revive", sol::readonly(&rpg::ItemTemplate::revive),
			"boni", sol::readonly(&rpg::ItemTemplate::boni)
				// BulletEmitter bullet;
				// EffectEmitter effect;
		);
	}
};

template <>
struct Binder<rpg::Item> {
	static void execute(sol::state& lua) {
		lua.new_usertype<rpg::Item>("StackedItem",
			// member variables
			"item", sol::readonly(&rpg::Item::item),
			"quantity", sol::readonly(&rpg::Item::quantity)
		);
	}
};

template <>
struct Binder<rpg::PerkTemplate> {
	static void execute(sol::state& lua) {
		lua.new_usertype<rpg::PerkTemplate>("Perk",
			// member variables
			"internal_name", sol::readonly(&rpg::BaseResource::internal_name),
			"type", sol::readonly(&rpg::PerkTemplate::type),
			"revive", sol::readonly(&rpg::PerkTemplate::revive),
			"damage", sol::readonly(&rpg::PerkTemplate::damage),
			"recover", sol::readonly(&rpg::PerkTemplate::recover)
			// BulletEmitter bullet;
			// EffectEmitter effect;
		);
	}
};

template <>
struct Binder<rpg::Perk> {
	static void execute(sol::state& lua) {
		lua.new_usertype<rpg::Perk>("LearnedPerk",
			// member variables
			"perk", sol::readonly(&rpg::Perk::perk),
			"level", sol::readonly(&rpg::Perk::level)
		);
	}
};

template <>
struct Binder<rpg::StatsData> {
	static void execute(sol::state& lua) {
		lua.new_usertype<rpg::StatsData>("Stats",
			// member variables
			"level", sol::readonly(&rpg::StatsData::level),
			"attributes", sol::readonly(&rpg::StatsData::attributes),
			"properties", sol::readonly(&rpg::StatsData::properties),
			"stats", sol::readonly(&rpg::StatsData::stats)
		);
	}
};

template <>
struct Binder<game::LuaApi> {
	static void execute(sol::state& lua) {
		lua.new_usertype<game::LuaApi>("LuaApi",
			"id", sol::readonly(&game::LuaApi::id),
			"isHostile", &game::LuaApi::isHostile,
			"getMove", &game::LuaApi::getMove,
			"hasPath", &game::LuaApi::hasPath,
			"isPathTarget", &game::LuaApi::isPathTarget,
			"getDirection", &game::LuaApi::getDirection,
			"getSight", &game::LuaApi::getSight,
			"getPosition", &game::LuaApi::getPosition,
			"getScene", &game::LuaApi::getScene,
			"getFocus", &game::LuaApi::getFocus,
			"getDistance", &game::LuaApi::getDistance,
			"getEnemies", &game::LuaApi::getEnemies,
			"getAllies", &game::LuaApi::getAllies,
			"getStats", &game::LuaApi::getStats,
			"isAlive", &game::LuaApi::isAlive,
			// getter
			"getWeaponDamage", &game::LuaApi::getWeaponDamage,
			"getPerkDamage", &game::LuaApi::getPerkDamage,
			"getPerkRecovery", &game::LuaApi::getPerkRecovery,
			"getEquipment", &game::LuaApi::getEquipment,
			"getWeapons", &game::LuaApi::getWeapons,
			"getArmors", &game::LuaApi::getArmors,
			"getPotions", &game::LuaApi::getPotions,
			"getPerks", &game::LuaApi::getPerks,
			// actions
			"navigate", &game::LuaApi::navigate,
			"move", &game::LuaApi::move,
			"moveTowards", &game::LuaApi::moveTowards,
			"look", &game::LuaApi::look,
			"lookTowards", &game::LuaApi::lookTowards,
			"stop", &game::LuaApi::stop,
			"attack", &game::LuaApi::attack,
			"useItem", &game::LuaApi::useItem,
			"usePerk", &game::LuaApi::usePerk
		);
	}
};

/// Bind all supported types to a script
/// All supported binders are allied to the given script.
/// @param script Reference to a script
/// @see game/resources.cpp
void bindAll(Script& script);

*/

} // ::game

