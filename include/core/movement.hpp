#pragma once
#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>
#include <core/entity.hpp>

namespace core {

namespace movement_impl {

extern float const MIN_SPEEDFACTOR;
extern float const MAX_SPEEDFACTOR;
extern float const DELTA_SPEEDFACTOR;
extern float const SIDEWARD_SPEEDFACTOR;
extern float const BACKWARD_SPEEDFACTOR;

// to fix floating point inaccuracy when reaching tile
/// @DEPRECATED
extern float const MOVEMENT_ACCURACY;

// to determine whether an object is centered on a cell or not
/// @DEPRECATED
extern float const CELL_CENTER_DIVERGENCE;

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	MoveSender& move_sender;
	MovementManager& movement_manager;
	DungeonSystem& dungeon_system;

	Context(LogContext& log, MoveSender& move_sender,
		MovementManager& movement_manager, DungeonSystem& dungeon_system);
};

enum class MoveStyle {
	Forward, Sideward, Backward
};

}  // ::movement_impl

// ---------------------------------------------------------------------------
// Movement System

/// MovementSystem handling movement interpolation and move event propagation
/**
 *	Each object with max_speed>0 can be moved within its scene. Furthermore
 *	this movement speed can be manipulated by changing the speed_factor field
 *	of a movement component. While each interpolation, the current movement
 *	vector is used to calculate a new world position.
 *	When starting a movement, an event is propagated which tells about which
 *	object is leaving which tile. So e.g. the collision system can be notified
 *	and triggered to perform collision checks. If a collision was detected,
 *	the movement system can be notified about this, the movement is canceled
 *	and the object's position is reset to the previous value.
 *	Once an object reached a new tile, another event is propagated which tells
 *	other systems about which object reached which tile. This can be used e.g.
 *	for bullet collisions.
 *	Keep in mind that this system will NOT change any of the scene related
 *	settings (e.g. the relationship to an object position's corresponding
 *	tile). Note also, that large frametimes are cut into multiple chunks of
 *	smalle frametimes to guarantee that all relevant tiles are visited during
 *	a movement. Also a maximum max_speed is applied if a component provides
 *	an exceeding value.
 *	There is no notification about each position in between. To let the render
 *	system know about each position (e.g. to calculate proper screen positions)
 *	is done via dirty flag. The dirty flag is set by the movement system after
 *	it changed a world position. If not, the flag isn't set. It is reset by
 *	the render system after processing the new position.
 */
class MovementSystem
	// Event API
	: public utils::EventListener<InputEvent, CollisionEvent>,
	  public utils::EventSender<MoveEvent>
	  // Component API
	  ,
	  public MovementManager {

  protected:
	movement_impl::Context context;

  public:
	MovementSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon);

	void handle(InputEvent const& event);
	void handle(CollisionEvent const& event);

	void update(sf::Time const& elapsed);
};

namespace movement_impl {

// ---------------------------------------------------------------------------
// Internal Movement API

/// This will update a range of components in a primitive sense
/**
 *	This function is used to update the given components directly, so each
 *	component is interpolated only once per this function's call. If the
 *	component is supposed to move, a new position will be calculated using
 *	the `interpolate` function.
 *
 *	@param context Movement context to work with
 *	@param begin Iterator to the start of the component range
 *	@param end Iterator to the end of the component range
 *	@param elapsed Duration that is used for interpolation
 */
void updateRange(Context& context, MovementManager::iterator begin,
	MovementManager::iterator end, sf::Time const& elapsed);

/// This will trigger or schedule a new movement for the given object
/**
 *	Applying a new movement direction while moving is dangerous, because this
 *	leads to inconsistent world positions by ignoring the fact, that movement
 *	is done (here) in a strictly discrete sense.
 *	So this function triggers a new movement if and only if the object is
 *	currently located at a cell's center. Otherwise the movement is scheduled
 *	for later execution. For instance: The object is currently located at
 *	<3.7,1.7> moving by <1,1> and should go <1,0>. It will continue <1,1>
 *	until it reaches <4,2> and will then go to <5,2> by <1,0>.
 *	If two directions are scheduled while moving, the first one will be lost
 *	and overriden by the second one and so forth.
 *	If no movement is specified, the object will stop moving as soon as
 *	possible.
 *
 *	@param context Movement context to work with
 *	@param data Component data to update
 *	@param event InputEvent that triggered a movement
 */
void start(Context& context, MovementData& data, InputEvent const& event);

/// This will trigger a new movement for the given object
/**
 *	This function will adjust the movement target to the tile that is
 *	specified by the current position and movement direction. Do NEVER change
 *	that direction directly and even NEVER call this function afterwards.
 *	This function will propagate a MoveEvent if a tile was left.
 *
 *	@param context Movement context to work with
 *	@param data Component data to update
 */
void moveToTarget(Context& context, MovementData& data);

/// This will stop the current movement immediately
/**
 *	This function is used to react on collisions in order to avoid forbidden
 *	movements. The object will be stopped and its position will be reset to
 *	the position specified in the collision event.
 *
 *	@param context Movement context to work with
 *	@param data Component data to update
 *	@param event CollisionEvent that should cause the object to stop
 */
void stop(Context& context, MovementData& data, CollisionEvent const& event);

/// Determine movement style
/// @param data Component data to use for getting move style
/// @return forward, sideward or backward based on movement and look vector
MoveStyle getMoveStyle(MovementData const & data);

/// Calculates object's speed factor
/**
 *	The speed factor is determined by `num_speed_boni` and some constants.
 *
 *	@post The resulting speed_factor is located within [MIN_SPEEDFACTOR,
 *MAX_SPEEDFACTOR]
 *	@param data Component data to use for calculation
 *	@return speed factor within specified bounds
 */
float calcSpeedFactor(MovementData const& data);

/// Used to interpolate a movement
/**
 *	This function calculates a new world position. Be aware not to call this
 *	method directly! It's part of `updateRange` and `updateSmallSteps` in
 *	order to guarantee consistent state.
 *	The size of an interpolation step can be modified by the component's
 *	max_speed and its speed_factor. Consider the limitations of this values!
 *	Once a tile is reached, a MoveEvent is propagated about which object
 *	reached which tile. If a zero movement is scheduled, the movement will be
 *	stopped. Otherwise the movement is continued and a MoveEvent about leaving
 *	the tile is propagated by calling `moveToTarget`.
 *
 *	@pre data.max_speed >= 0.f
 *	@pre data.max_speed <= MAX_SPEED
 *	@param context Movement context to work with
 *	@param data Component data to update
 *	@param elapsed Duration to use for interpolation
 */
void interpolate(Context& context, MovementData& data, sf::Time const& elapsed);

}  // ::movement_impl

}  // ::core
