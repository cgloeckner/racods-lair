#pragma once
#include <boost/optional.hpp>

#include <core/entity.hpp>
#include <core/event.hpp>
#include <game/common.hpp>
#include <game/event.hpp>
#include <game/path.hpp>

namespace game {

/// @note out of order, needs reimplementation
/*
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
	enum { Idle, Trigger, Wait, Trace } state;

	core::LogContext& log;
	PathSystem& pathfinder;
	core::MovementManager const& movement_manager;
	core::InputSender& input_sender;
	core::ObjectID const actor;

	std::future<Path> request;
	std::vector<sf::Vector2u> path;
	sf::Vector2u current, target;

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
	void pathfind(sf::Vector2u const& target);

	/// Handle incomming movement event
	/// 
	/// This is used to detect whether the object is ready for the
	/// next step.
	/// 
	/// @param event MoveEvent to consider
	/// 
	void handle(core::MoveEvent const& event);

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
	std::vector<sf::Vector2u> const& getPath() const;
};

*/

}  // ::game
