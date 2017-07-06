#pragma once
#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>
#include <core/entity.hpp>

namespace core {

namespace physics_impl {

extern float const MOVEMENT_VELOCITY;
extern float const MIN_SPEEDFACTOR;
extern float const MAX_SPEEDFACTOR;
extern float const DELTA_SPEEDFACTOR;
extern float const SIDEWARD_SPEEDFACTOR;
extern float const BACKWARD_SPEEDFACTOR;

extern float const MAX_COLLISION_RADIUS;

// to determine whether an object is centered on a cell or not
extern float const CELL_CENTER_DIVERGENCE;

// determines maximum step (with lowest frametime)
float const MAX_SPEED = MAX_TILE_STEP / (MAX_FRAMETIME_MS * MOVEMENT_VELOCITY);

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	CollisionSender& collision_sender;
	FocusSender& focus_sender;
	PhysicsManager& physics_manager;
	DungeonSystem& dungeon_system;

	Context(LogContext& log, CollisionSender& collision_sender,
		FocusSender& focus_sender, PhysicsManager& physics_manager,
		DungeonSystem& dungeon_system);
};

enum class MoveStyle {
	Forward, Sideward, Backward
};

}  // ::physics_impl

// ---------------------------------------------------------------------------
// Movement System

/// PhysicsSystem handling movement interpolation and move event propagation
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
class PhysicsSystem
	// Event API
	: public utils::EventListener<InputEvent>
	, public utils::EventSender<CollisionEvent, FocusEvent>
	// Component API
	, public PhysicsManager {

  protected:
	physics_impl::Context context;

  public:
	PhysicsSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon);

	void handle(InputEvent const& event);
	void handle(CollisionEvent const& event);

	void update(sf::Time const& elapsed);
};

namespace physics_impl {

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
void updateRange(Context& context, PhysicsManager::iterator begin,
	PhysicsManager::iterator end, sf::Time const& elapsed);

/// This will trigger or schedule a new movement for the given object
void start(Context& context, PhysicsData& data, InputEvent const& event);

/// Determine movement style
/// @param data Component data to use for getting move style
/// @return forward, sideward or backward based on movement and look vector
MoveStyle getMoveStyle(PhysicsData const & data);

/// Calculates object's speed factor
/// The speed factor is determined by `num_speed_boni` and some constants.
/// @post The resulting speed_factor is located within [MIN_SPEEDFACTOR,
///   MAX_SPEEDFACTOR]
/// @param data Component data to use for calculation
/// @return speed factor within specified bounds
float calcSpeedFactor(PhysicsData const& data);

/// Checks for collision and triggers relevant CollisionEvent
bool checkCollision(Context& context, PhysicsData const & data, sf::Vector2f const & pos);

/// Used to interpolate a movement
/**
 *	This function calculates a new world position. Be aware not to call this
 *	method directly! It's part of `updateRange` and `updateSmallSteps` in
 *	order to guarantee consistent state.
 *	The size of an interpolation step can be modified by the component's
 *	max_speed and its speed_factor. Consider the limitations of this values!
 *	This might cause a collision event
 *
 *	@pre data.max_speed >= 0.f
 *	@pre data.max_speed <= MAX_SPEED
 *	@param context Movement context to work with
 *	@param data Component data to update
 *	@param elapsed Duration to use for interpolation
 */
void interpolate(Context& context, PhysicsData& data, sf::Time const& elapsed);

}  // ::physics_impl

}  // ::core
