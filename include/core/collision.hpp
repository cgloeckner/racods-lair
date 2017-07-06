#pragma once
#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>
#include <core/entity.hpp>

namespace core {

namespace collision_impl {

/// maximum radius for regular object collision
extern float const REGULAR_COLLISION_RADIUS;
/// maximum radius for projectile object collision
extern float const MAX_PROJECTILE_RADIUS;
/// maximum radius for collision detection
float const MAX_COLLISION_RANGE =
	REGULAR_COLLISION_RADIUS + MAX_PROJECTILE_RADIUS;

// ---------------------------------------------------------------------------

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	CollisionSender& collision_sender;
	MoveSender& move_sender;
	TeleportSender& teleport_sender;
	CollisionManager& collision_manager;
	DungeonSystem& dungeon_system;
	MovementManager const& movement_manager;

	Context(LogContext& log, CollisionSender& collision_sender,
		MoveSender& move_sender, TeleportSender& teleport_sender,
		CollisionManager& collision_manager, DungeonSystem& dungeon_system,
		MovementManager const& movement_manager);
};

}  // ::collision_impl

// ---------------------------------------------------------------------------
// External Collision API

/// Checks for a tile collision
/**
 *	This function can be used to check for a tile collision. It can also be
 *	used from external systems such like AI.
 *	No actual data about the actor object is necessary, because each object
 *	will collide with non-accessible terrain.
 *
 *	@param cell To check for collision
 *	@return true if a collision was detected
 */
bool checkTileCollision(DungeonCell const& cell);

/// Checks for a object collision
/**
 *	This function can be used to check for a object collision. It can also be
 *	used from external systems such like AI.
 *
 *	@pre !data.is_projectile
 *	@param manager Which holds the collision components
 *	@param cell To check for collision
 *	@param data CollisionData of the actor object
 *	@return Array of ObjectID of colliders
 */
std::vector<ObjectID> checkObjectCollision(CollisionManager const& manager,
	DungeonCell const& cell, CollisionData const& data);

/// Checks for a bullet collision
/**
 *	Bullets can collide with regular objects once their distance is undershot.
 *	For this scenario all neighbor cell's objects are fetched and the distance
 *	are checked for the current bullet. This is a broadphase collision.
 *
 *	@pre data.is_projectile
 *	@param collision Manager that holds the collision components
 *	@param movement Manager that holds the movement components
 *	@param dungeon System that holds all dungeons
 *	@param data CollisionData of the bullet object
 *	@return ObjectIDs of all possible colliders in range
 */
std::vector<ObjectID> checkBulletCollision(CollisionManager const& collision,
	MovementManager const& movement, DungeonSystem const& dungeon,
	CollisionData const& data);

// ---------------------------------------------------------------------------
// Collision System

/// CollisionSystem handling collision detection and propagation
/**
 *	Each object can be extended by a collision component. This component
 *	specifies whether it's a projectile (aka bullet) object or a regular
 *	entity. Bullets' collision is checked on each frame using its distance to
 *	all objects in the neighborhood. The collision of regular entities is only
 *	checked when they leave a tile. In both cases a `CollisionEvent` is
 *	genereated if a collision was detected. This event can be used to let other
 *	systems react on the collision. In order to be able to track movements and
 *	detect collisions, a reference to the underlying dungeon system is
 *	attached to this system. So the collision-relevant data (which object is
 *	located on which tile) is automatically adjusted by this system each time
 *	an object (regardless bullet or regular entity) leaves a tile.
 *	All tile events can be forwarded to e.g. a focusing system
 *	A typical behavior on collisions could be:
 *	- Regular entities will be forced to stop their movement and stay at
 *		their source position.
 *	- Bullet entities will collide at their target position when reaching
 *		the tile (e.g. as explosion with a wall or player).
 *	In addition, a trigger can be executed if an object (despite regular or
 *	bullet) reaches a tile. If there's a trigger, it will be executed. The
 *	trigger might cause additional events, e.g. to teleport the object to a
 *	different location.
 */
class CollisionSystem
	// Event API
	: public utils::EventListener<MoveEvent>,
	  public utils::EventSender<CollisionEvent, MoveEvent, TeleportEvent>
	  // Component API
	  ,
	  public CollisionManager {

  protected:
	sf::Time passed;  // since last collision check
	collision_impl::Context context;

  public:
	CollisionSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon,
		MovementManager const& movement);

	void handle(MoveEvent const& event);

	void update(sf::Time const& elapsed);
};

// ---------------------------------------------------------------------------
// Internal Collision API

namespace collision_impl {

/// Update the corresponding collision flags inside the scene object
/**
 *	This function will modify the corresponding collision flags inside the
 *	scene object's collision structure. The object is moved out of its current
 *	grid cell into its new one.
 *
 *	@pre The actor has a movement component
 *	@pre The actor is attached to a scene
 *	@param context Context of the collision scene
 *	@param data CollisionData of the actor object
 *	@param event MoveEvent with additional information
 */
void updateCollisionMap(
	Context const& context, CollisionData const& data, MoveEvent const& event);

/// Reacts on leaving a tile
/**
 *	This function contains logic to react on leaving a tile. In case of a
 *	regular objects it will trigger a collision check to guarantee a collision
 *	free movement to the target position given by the event.
 *	If no collision occured, the collision flag within the scene obejct is
 *	modified.
 *	If no collision occured, the move event is forwarded to the context's
 *	move sender.
 *
 *	@param context Context of the collision scene
 *	@param data CollisionData of the actor object
 *	@param event MoveEvent with additional information
 */
void onTileLeft(
	Context& context, CollisionData const& data, MoveEvent const& event);

/// Reacts on reaching a tile
/**
 *	This function contains logic to react on reaching a tile. In case of a
 *	bullet object it will trigger a collision check to guarantee a collision
 *	free movement reaching the target position given by the event. In this
 *	case only terrain collisions are relevant.
 *	If no collision occured, the move event is forwarded to the context's
 *	move sender.
 *
 *	@param context Context of the collision scene
 *	@param data CollisionData of the actor object
 *	@param event MoveEvent with additional information
 */
void onTileReached(
	Context& context, CollisionData const& data, MoveEvent const& event);

/// Performs a bullet's object collision check
/**
 *	This function is usually called by the collision system once per frame and
 *	object. If a regular object is passed, the function returns without
 *	performing any checks. If a bullet object is passed, the bullet's object
 *	collision is checked and propagated if occured.
 *
 *	@param context Context of the collision scene
 *	@param data CollisionData of the actor object
 */
void onBulletCheck(Context& context, CollisionData& data);

/// Checks whether the given object collides at the given position
/**
 *	This function will check for object collision within the given context.
 *	In case of a collision, either the `tile_collision` or `object_collision`
 *	result parameters will be filled.
 *	Both out parameters will be initialized with false (no tile collision)
 *	and 0 (no object collision) at the beginning. In case of a tile collision
 *	the corresponding parameter will be true; in case of an object collision
 *	the corresponding parameter will contain the collider's object id (>0).
 *
 *	@pre The actor has a movement component
 *	@pre The actor is attached to a scene
 *	@param context Context of the collision scene
 *	@param data CollisionData of the actor object
 *	@param pos Cellposition of the actor to check for
 *	@param tile_collision Determines whether a tile collision was detected
 *	@param object_collision Determiens whether an object collision was detected
 */
void checkCollision(Context const& context, CollisionData const& data,
	sf::Vector2u const& pos, bool& tile_collision, ObjectID& object_collider);

}  // ::collision_impl

}  // ::core
