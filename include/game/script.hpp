#pragma once
#include <utils/lua_utils.hpp>

#include <game/entity.hpp>

namespace game {

/// @note out of order, needs reimplementation
/*
namespace script_impl {

/// Prevent scripts from being update-notified too often; delay in ms
extern unsigned int const UPDATE_DELAY;

/// Context of the scripting system
struct Context {
	core::LogContext& log;
	ScriptManager& script_manager;
	sf::Time update_delay;

	Context(core::LogContext& log, ScriptManager& script_manager);
};

// ---------------------------------------------------------------------------

/// Notify about a collision event
/// 
///	This will query the corresponding objects' ScriptData and notify
///	about the upcomming event.
///	It might call onObjectCollision or onTileCollision.
///
///	@param context Reference to the scripting context
///	@param event Upcomming CollisionEvent
/// 
void onCollision(Context& context, core::CollisionEvent const& event);

/// Notify about a teleport event
/// 
///	This will query the corresponding objects' ScriptData and notify
///	about the upcomming event.
///	It might call onTeleport.
///
///	@param context Reference to the scripting context
///	@param event Upcomming TeleportEvent
/// 
void onTeleport(Context& context, core::TeleportEvent const& event);

/// Notify about idle
/// 
///	This will notify the given actor's ScriptData about idling. It is
///	usually invoked after an animation stopped.
///	It might call onIdle.
///
///	@param context Reference to the scripting context
///	@param actor Actor's ScriptData
/// 
void onIdle(Context& context, ScriptData& actor);

/// Notify about a movement event
/// 
///	This will query the corresponding objects' ScriptData and notify
///	about the upcomming event.
///	It might call onTileLeft or onTileReached.
///
///	@param context Reference to the scripting context
///	@param event Upcomming MoveEvent
/// 
void onMove(Context& context, core::MoveEvent const& event);

/// Notify about a effect event
/// 
///	This will query the corresponding objects' ScriptData and notify
///	about the upcomming event.
///	It might call onEffectReceived, onEffectFaded or onEffectInflicted.
///
///	@param context Reference to the scripting context
///	@param event Upcomming EffectEvent
/// 
void onEffect(Context& context, rpg::EffectEvent const& event);

/// Notify about a stats event
/// 
///	This will query the corresponding objects' ScriptData and notify
///	about the upcomming event.
///	It might call onStatsReceived or onStatsInflicted.
///
///	@param context Reference to the scripting context
///	@param event Upcomming StatsEvent
/// 
void onStats(Context& context, rpg::StatsEvent const& event);

/// Notify about a death event
/// 
///	This will query the corresponding objects' ScriptData and notify
///	about the upcomming event.
///	It might call onDeath or onEnemyKilled.
///
///	@param context Reference to the scripting context
///	@param event Upcomming DeathEvent
/// 
void onDeath(Context& context, rpg::DeathEvent const& event);

/// Notify about a respawn event
/// 
///	This will query the corresponding objects' ScriptData and notify
///	about the upcomming event.
///	It might call onHasSpawned or onDidSpawn
///
///	@param context Reference to the scripting context
///	@param event Upcomming SpawnEvent
/// 
void onSpawn(Context& context, rpg::SpawnEvent const& event);

/// Notify about a feedback event
/// @param context Reference to the scripting context
/// @param event Upcomming feedback event
void onFeedback(Context& context, rpg::FeedbackEvent const & event);

/// Notify about a failed pathfinding request
/// 
///	This will query the corresponding objects' ScriptData and notify
///	about the upcomming event.
///	It might call onPathFailed
///
///	@param context Reference to the scripting context
///	@param event Upcomming PathFailedEvent
/// 
void onPathFailed(Context& context, PathFailedEvent const& event);

/// Notify about an update
/// 
///	This will update the given actor's ScriptData. It is usually
///	invoked when the last update was enough ms ago.
///	It might call onUpdate.
///
///	@param context Reference to the scripting context
///	@param actor Actor's ScriptData
/// 
void onUpdate(Context& context, ScriptData& actor);

// ---------------------------------------------------------------------------

/// Try to update the context
/// 
///	Will update the given context. Once enough time passed by, all
///	scripts are updated.
///
///	@param context Reference to the scripting context
///	@param elapsed Time since last frame
/// 
void update(Context& context, sf::Time const& elapsed);

} // ::script_impl

// ---------------------------------------------------------------------------

/// System to handle all Lua script events
class ScriptSystem
	// Event API
	: public utils::EventListener<core::CollisionEvent, core::TeleportEvent,
		core::AnimationEvent, core::MoveEvent,
		rpg::EffectEvent, rpg::StatsEvent, rpg::DeathEvent, rpg::SpawnEvent,
		rpg::FeedbackEvent, PathFailedEvent>
	// Component API
	, public ScriptManager {

  private:
	script_impl::Context context;

  public:
	ScriptSystem(core::LogContext& log, std::size_t max_objects);

	void handle(core::CollisionEvent const& event);
	void handle(core::TeleportEvent const& event);
	void handle(core::AnimationEvent const& event);
	void handle(core::MoveEvent const& event);
	void handle(rpg::EffectEvent const& event);
	void handle(rpg::StatsEvent const& event);
	void handle(rpg::DeathEvent const& event);
	void handle(rpg::SpawnEvent const& event);
	void handle(rpg::FeedbackEvent const& event);
	void handle(PathFailedEvent const& event);

	void update(sf::Time const& elapsed);
};
*/

}  // ::game
