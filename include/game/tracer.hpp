#pragma once
#include <core/entity.hpp>
#include <core/event.hpp>
#include <game/common.hpp>
#include <game/entity.hpp>
#include <game/event.hpp>
#include <game/path.hpp>

namespace game {

namespace tracer_impl {

extern float const WAYPOINT_REACHED_THRESHOLD;

struct Context {
	core::LogContext& log;
	core::InputSender& input_sender;
	core::MovementManager const & movement;
	
	PathSystem& pathfinder;
	
	Context(core::LogContext& log, core::InputSender& input_sender, core::MovementManager const & movement,
		PathSystem& pathfinder);
};

/// Retrigger pathfinding on collision
/// @param context Tracer context to use
/// @param data TracerData to use
void onCollision(Context& context, TracerData& data);

/// Clears the path on teleport
/// @param data TracerData to clear for
void onTeleport(TracerData& data);

/// Disable tracing on death
/// @param data TracerData to disable
void onDeath(TracerData& data);

/// Enable tracing on (re-)spawn
/// @param data TracerData to enable
void onSpawn(TracerData& data);

/// Update the pathtracing for a single entity
/// @param context Tracer context to use
/// @param data TracerData to use
void onUpdate(Context& context, TracerData& data);

} // ::tracer_impl

/// Trigger pathfinding to given target
/// @param log LogContext
/// @param pathfinder Pathfinding System to use
/// @param move MovementData (like sceneID, startPos)
/// @param trace TracerData to use
/// @param target Pos to move to
void tracer(core::LogContext& log, PathSystem& pathfinder, core::MovementData const & move, TracerData& tracer, sf::Vector2f const & target);

// -----------------------------------------------------------------------------------------

class TracerSystem
	: public utils::EventListener<core::CollisionEvent, core::TeleportEvent, rpg::DeathEvent, rpg::SpawnEvent>
	, public utils::EventSender<core::InputEvent>
	// Component API
	, public TracerManager {

  private:
	tracer_impl::Context context;

  public:
	TracerSystem(core::LogContext& log, std::size_t max_objects, core::MovementManager const & movement,
		PathSystem& pathfinder);

	void handle(core::CollisionEvent const& event);
	void handle(core::TeleportEvent const& event);
	void handle(rpg::DeathEvent const& event);
	void handle(rpg::SpawnEvent const& event);

	void update(sf::Time const& elapsed);
};








	
/// @note out of order, needs reimplementation
/// Helper to trace a path
/// 
/// A tracer is used once per AI object to simplify the entire movement
/// process. Once the path is acquired, the tracer will propagate
/// input events to move the actor object to the next position.
/// 
/// @note The current implementation ignores broadphase pathfinding.
/// 
class PathTracer {
  private:
	TraceState state;

	core::LogContext& log;
	PathSystem& pathfinder;
	core::MovementManager const& movement_manager;
	core::InputSender& input_sender;
	core::ObjectID const actor;

	std::future<Path> request;
	std::vector<sf::Vector2f> path;
	sf::Vector2f start, finish;

	bool requestIsReady() const;

  public:
	/// Create a new path tracer
	/// 
	/// The given input_sender is used to propagate upcomming
	/// input events.
	/// 
	/// @param log Reference to the logging context
	/// @param pathfinder Reference to a PathSystem
	/// @param movement_manager Const reference to movement manager
	/// @param input_sender Reference to an input sender
	/// @param actor Actor's object ID
	/// 
	PathTracer(core::LogContext& log, PathSystem& pathfinder,
		core::MovementManager const& movement_manager,
		core::InputSender& input_sender, core::ObjectID actor);

	/// Reset the tracer's state
	/// 
	/// This will reset the tracer's internal state to the default.
	/// So no path is traced anymore, pending paths are dropped.
	/// 
	void reset();

	/// Acquire a new path
	/// 
	/// This causes the pathfinder to provide a path to the given
	/// target, based on the actor's current position.
	/// 
	/// @param target Target position to move to
	/// 
	void pathfind(sf::Vector2f const& target);

	/// Handle incomming collision event
	/// 
	/// This is used to detect collisions and reset the path trace.
	/// This forces the path to be calculated again.
	/// 
	/// @param event CollisionEvent to consider
	/// 
	void handle(core::CollisionEvent const& event);

	/// Update the tracer
	/// 
	/// This invokes both pathfinding and pathtracing.
	/// 
	/// @return PathFailedEvent or boost::none
	/// 
	boost::optional<PathFailedEvent> update();

	/// Determine whether the tracer is currently tracing
	/// 
	/// @return true if is active
	/// 
	bool isRunning() const;

	/// Return actual path
	std::vector<sf::Vector2f> const& getPath() const;
};

}  // ::game
