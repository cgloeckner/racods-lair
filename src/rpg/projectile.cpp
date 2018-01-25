#include <utils/math2d.hpp>
#include <core/collision.hpp>
#include <rpg/projectile.hpp>

namespace rpg {

namespace projectile_impl {

Context::Context(core::LogContext& log, CombatSender& combat_sender,
	ProjectileSender& projectile_sender, ProjectileManager const& projectile,
	core::MovementManager const& movement,
	core::CollisionManager const& collision,
	core::DungeonSystem const& dungeons)
	: log{log}
	, combat_sender{combat_sender}
	, projectile_sender{projectile_sender}
	, projectile{projectile}
	, movement{movement}
	, collision{collision}
	, dungeons{dungeons} {}

bool canHit(Context const& context, ProjectileData const& projectile,
	sf::Vector2f const& origin, core::ObjectID target) {
	ASSERT(projectile.bullet != nullptr);
	if (utils::contains(projectile.ignore, target)) {
		// cannot combat whitelisted object
		return false;
	}
	if (!context.collision.has(target)) {
		// cannot combat object that cannot collide
		return false;
	}
	if (context.projectile.has(target)) {
		// cannot combat another projectile
		// (especially not with itself)
		return false;
	}
	utils::Collider shape;
	shape.radius = projectile.bullet->radius;
	auto pos2 = context.movement.query(target).pos;
	auto const & shape2 = context.collision.query(target).shape;
	
	if (!shape2.is_aabb) {
		return utils::testCircCirc(origin, shape, pos2, shape2);
	} else {
		return utils::testCircAABB(origin, shape, pos2, shape2);
	}
}

void onCollision(Context& context, core::CollisionEvent const& event) {
	ASSERT(event.actor > 0u);
	auto const& coll = context.collision.query(event.actor);
	ASSERT(coll.is_projectile);
	auto const& projectile = context.projectile.query(event.actor);
	ASSERT(projectile.bullet != nullptr);

	if (event.collider > 0u) {
		if (utils::contains(projectile.ignore, event.collider)) {
			// target cannot be damaged by this projectile
			return;
		}

		// propagate combat event
		CombatEvent ev;
		ev.actor = event.actor;
		ev.target = event.collider;
		ev.meta_data = projectile.meta_data;
		context.combat_sender.send(ev);

	} else {
		// trigger bullet destruction in case of tile collision
		ProjectileEvent ev;
		ev.id = event.actor;
		ev.type = ProjectileEvent::Destroy;
		context.projectile_sender.send(ev);
	}
}

}  // :: projectile_impl

// ---------------------------------------------------------------------------

ProjectileSystem::ProjectileSystem(core::LogContext& log, std::size_t max_objects,
	core::MovementManager const& movement,
	core::CollisionManager const& collision,
	core::DungeonSystem const& dungeons)
	: utils::EventListener<core::CollisionEvent>{}
	, utils::EventSender<CombatEvent, ProjectileEvent>{}
	, ProjectileManager{max_objects}
	, context{log, *this, *this, *this, movement, collision, dungeons} {}

void ProjectileSystem::handle(core::CollisionEvent const& event) {
	if (!has(event.actor)) {
		// object has no projectile component
		return;
	}
	projectile_impl::onCollision(context, event);
}

void ProjectileSystem::update(sf::Time const& elapsed) {
	dispatch<core::CollisionEvent>(*this);

	propagate<CombatEvent>();
	propagate<ProjectileEvent>();
}

}  // ::game
