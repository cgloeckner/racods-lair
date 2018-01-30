#pragma once
#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>
#include <core/entity.hpp>

namespace core {

struct CollisionResult {
	bool interrupt, tile;
	std::vector<ObjectID> objects;
	
	CollisionResult();
	
	bool meansCollision() const;
};

// ---------------------------------------------------------------------------

namespace collision_impl {

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	CollisionSender& collision_sender;
	MoveSender& move_sender;
	TeleportSender& teleport_sender;
	CollisionManager& collision_manager;
	DungeonSystem& dungeon_system;
	MovementManager const & movement_manager;
	
	CollisionResult collision_result;
	
	Context(LogContext& log, CollisionSender& collision_sender,
		MoveSender& move_sender, TeleportSender& teleport_sender,
		CollisionManager& collision_manager, DungeonSystem& dungeon_system,
		MovementManager const & movement_manager);
};

// ---------------------------------------------------------------------------
// Internal Collision API

/// Checks whether any collision occures
/// If the function returns a collision result containing more information.
/// In case of a regular object collision test, no object collisions are
/// tested after a tile collision occured. If no tile collision was found,
/// the first object collision is reported. Others are ignored.
/// In case of a projectile collision test, all relevant objects are
/// always tested (including a true tile collision and/or already found
/// object collisions). The full list of colliders is returned through
/// the vector.
/// Each projectile collides only once with each other object.
/// @pre the actor is attached to a valid scene
/// @param context Collision context
/// @param actor MoveData of the moving actor
/// @param result Collision information
void checkAnyCollision(Context const & context, MovementData const & actor, CollisionResult& result);

/// Update the corresponding collision flags inside the scene object
/// The actor is moved within the scene's grid to the corresponding cell.
/// @pre The actor is attached to a valid scene
/// @param context Context of the collision scene
/// @param data MoveData of the moving actor
/// @return true if something was actually updated
bool updateCollisionMap(Context& context, MovementData const & actor);

/// Perform a full collision check on all relevant objects
/// Each moving object is tested for collision. Not moving objects are skipped.
/// Once a collision was detected, a CollisionEvent is triggered. If an object
/// does not collide, the collision map is updated corresponding the object's
/// position.
/// Each object collision is reported using a CollisionEvent. If an object
/// caused a tile but not an object collision, a CollisionEvent is triggered
/// for that tile collision. If both, only the object collisions are propagated.
/// Regular object collisions cause a position reset. This is not triggered for
/// projectiles.
/// @param context Collision context
void checkAllCollisions(Context& context);

}  // ::collision_impl

// ---------------------------------------------------------------------------
// External Collision API

/// Checks for a tile collision
/// A tile collision occures if the actor's position enteres a wall tile.
/// So only the target tile is passed here, which was previously queried
/// using the suspected position. Hence no actor is needed here.
/// @note This can be easily called from external code
/// @param cell To check for tile collision
/// @return true if a collision was detected
bool checkTileCollision(DungeonCell const& cell);

/// Check whether a collision occured
/// The actor is assumed to be moving into the target. For calling this,
/// the movement data is usually queried just before. Because it's also
/// needed inside, its just passed. Additional collision data is queried
/// on demand. If at least one actor has no collision data, no collision
/// occures. Colliders on the ignore list are ignored.
/// @note This can be easily called from external code
/// @param collision_manager CollisionManager to query CollisionData with
/// @param actor MoveData of the moving actor
/// @param target MoveData of the suspected collider
/// @param[out] interrupt Specifies whether collision will interrupt movement
/// @return true in case of collision
bool checkObjectCollision(CollisionManager const & collision_manager, ObjectID actor_id,
		sf::Vector2f actor_pos, ObjectID target_id, sf::Vector2f target_pos);

/// Overload to determine if any collision can be found
/// @note This can be easily called from external code
/// @param movement_manager Manager to query suspect collider's exact position
/// @param collision_manager Manager to query object's collision information
/// @param scene Dungeon to search at
/// @param actor MoveData of moving actor
/// @param result Collision information
void checkAnyCollision(MovementManager const & movement_manager,
		CollisionManager const & collision_manager, Dungeon const & scene,
		MovementData const & actor, CollisionResult& result);

// ---------------------------------------------------------------------------
// Collision System

/// CollisionSystem handling collision detection, solving and propagation
/// Each object with a collision component is either an AABB or a circle.
/// Once an object collides with a tile or another object, its movement is
/// interrupted. Some objects are projectiles, that pierce other objects.
/// Hence they can collide multiple times.
/// Regular objects do not collide with projectiles, but projectiles do
/// collide with everything.
/// After successfully updating the collision map, possible triggers are
/// invoked.
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
		MovementManager const & movement);

	void handle(MoveEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::core
