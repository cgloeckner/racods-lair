#pragma once
#include <core/common.hpp>
#include <core/entity.hpp>
#include <core/event.hpp>
#include <core/dungeon.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace projectile_impl {

/// Context of the projectile implementation
struct Context {
	core::LogContext& log;
	CombatSender& combat_sender;
	ProjectileSender& projectile_sender;

	ProjectileManager const& projectile;
	core::MovementManager const& movement;
	core::CollisionManager const& collision;
	core::DungeonSystem const& dungeons;

	Context(core::LogContext& log, CombatSender& combat_sender,
		ProjectileSender& projectile_sender,
		ProjectileManager const& projectile,
		core::MovementManager const& movement,
		core::CollisionManager const& collision,
		core::DungeonSystem const& dungeons);
};

/// Determines whether the given target can be hit by the given projectile
/**
 *	This determines whether the given target can be hit by the given
 *	projectile using the specified position of the projectile. Returning true
 *	requires the target to have a collision component, to have no projectile
 *	component and to be within the projectile's explosion radius.
 *
 *	@pre projectile.bullet != nullptr
 *	@param context Projectile context to deal with
 *	@param projectile Projectile data which should collide with the target
 *	@param origin Current position of the projectile
 *	@param target Target object's id
 *	@return true if the projectile can hit the target, else false is returned
 */
bool canHit(Context const& context, ProjectileData const& projectile,
	sf::Vector2f const& origin, core::ObjectID target);

/// Determines all targets of the given projectile
/**
 *	A projectile can hit multiple targets that are located within its
 *	explosion radius. Those targets need to satisfy canHit().
 *
 *	@pre projectile.bullet != nullptr
 *	@pre The projectile is located at the given dungeon
 *	@param context Projectile context to deal with
 *	@param projectile Projectile data which should collide with the target
 *	@param dungeon The projectile's current dungeon
 *	@param origin Current position of the projectile
 *	@return vector of targets' object ids
 *
std::vector<core::ObjectID> getTargets(Context const & context, ProjectileData
const & projectile, core::Dungeon const & dungeon, sf::Vector2f const & origin);
*/

/// Handles a collision event
/**
 *	This will handle a collision event for the specified projectile. If the
 *	collision event does not belong to a projectile, nothing happens.
 *	Also, the type of collision (whether tile or object collision) is not
 *	checked here. Each projectile will combat the surrounding objects, despite
 *	the projectile hit an object or collided with a wall.
 *	A combat event is generated for each target.
 *
 *	@param context Projectile context to deal with
 *	@param event Collision event which should be handled
 */
void onCollision(Context& context, core::CollisionEvent const& event);

}  // :: projectile_impl

// ---------------------------------------------------------------------------

/// The ProjectileSystem is used to cause bullets' explosion
/**
 *	CollisionEvents are transformed to CombatEvents if a bullet collided with
 *	a regular object or a tile. If such a collision is detected, all targets
 *	within the bullet's explosion radius are queried and (multiple)
 *	CombatEvents are propagated to trigger the calculates. Bullet destruction
 *	is NOT triggered here.
 *	The bullets might have whitelists of objects which should not be attacked
 *	by explosions. This can be used for enabling or disable friendly fire.
 *	If no targets are affected by the explosion, the projectile is destroyed
 *	directly. Otherwise, the projectile is destroyed after its combats.
 */
class ProjectileSystem
	// Event API
	: public utils::EventListener<core::CollisionEvent>,
	  public utils::EventSender<CombatEvent, ProjectileEvent>
	  // Component API
	  ,
	  public ProjectileManager {

  private:
	projectile_impl::Context context;

  public:
	ProjectileSystem(core::LogContext& log, std::size_t max_objects,
		core::MovementManager const& movement,
		core::CollisionManager const& collision,
		core::DungeonSystem const& dungeons);

	void handle(core::CollisionEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game
