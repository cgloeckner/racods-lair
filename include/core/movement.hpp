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

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	MoveSender& move_sender;
	MovementManager& movement_manager;
	DungeonSystem& dungeon_system;

	Context(LogContext& log, MoveSender& move_sender,
		MovementManager& movement_manager, DungeonSystem& dungeon_system);
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
	  , public MovementManager {

  protected:
	movement_impl::Context context;

  public:
	MovementSystem(LogContext& log, std::size_t max_objects,
		DungeonSystem& dungeon);

	void handle(InputEvent const& event);
	void handle(CollisionEvent const& event);

	void update(sf::Time const& elapsed);
};

namespace movement_impl {

// ---------------------------------------------------------------------------
// Internal Movement API

/// This set the movement (start, restart, stop)
/// The given movement vector is applied. Depending whether the vector is zero
/// or nor, MoveEvent::Start or MoveEvent::Stop are propagated.
/// MoveEvent::Start is triggerd if a standing object starts moving.
/// MoveEvent::Stop is trigged if a moving object stopped
/// @param context Movement context to work with
/// @param actor Movement data to work with
/// @param move Movement vector to apply
/// @param focus Looking vector to apply
void setMovement(Context& context, MovementData& actor, sf::Vector2f const & move,
	sf::Vector2f const & look);

/// This reacts on a collision event
/// If the collision interrupts the actor, he is stopped and reset to his
/// last position
/// @param context Movement context to work with
/// @param actor Movement data to work with
/// @param event Collision event to apply
void onCollision(Context& context, MovementData& actor, CollisionEvent const & event);

/// Determine movement style depending on movement and focus vectors
/// @param actor Movement data to consider
/// @return forward, sideward or backward based on movement and look vector
MoveStyle getMoveStyle(MovementData const & actor);

/// Calculates object's speed factor
/// The speed factor is determined by `num_speed_boni` and some constants.
/// @post The resulting speed_factor is located within [MIN_SPEEDFACTOR, MAX_SPEEDFACTOR]
/// @param actor Movement data to use for calculation
/// @return speed factor within specified bounds
float calcSpeedFactor(MovementData const& actor);

/// Calculate object's movement speed delta
/// @param actor Movement data to use for calculation
/// @param elapsed Duration to use
/// @return speed delta factor for interpolation
float getSpeedDelta(MovementData const & data, sf::Time const & elapsed);

/// Used to interpolate a movement
/// This function calculates a new world position. Be aware not to call this
/// method directly! It's part of `updateRange` and `updateSmallSteps` in
/// order to guarantee consistent state.
/// The size of an interpolation step can be modified by the component's
/// max_speed and its speed_factor. Consider the limitations of this values!
/// @pre data.max_speed >= 0.f
/// @pre data.max_speed <= MAX_SPEED
/// @param context Movement context to work with
/// @param data Component data to update
/// @param elapsed Duration to use for interpolation
void interpolate(Context& context, MovementData& data, sf::Time const& elapsed);

/// This will update a range of components in a primitive sense
/// This function is used to update the given components directly, so each
/// component is interpolated only once per this function's call. If the
/// component is supposed to move, a new position will be calculated using
/// the `interpolate` function.
/// @param context Movement context to work with
/// @param begin Iterator to the start of the component range
/// @param end Iterator to the end of the component range
/// @param elapsed Duration that is used for interpolation
void updateRange(Context& context, MovementManager::iterator begin,
	MovementManager::iterator end, sf::Time const& elapsed);

}  // ::movement_impl

}  // ::core
