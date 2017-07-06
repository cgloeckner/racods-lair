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
	auto const& target_move = context.movement.query(target);
	auto dist = utils::distance(target_move.pos, origin);
	// using squared distance with respect to the max collision radius (for
	// regular objects)
	auto max_dist = projectile.bullet->radius +
					core::collision_impl::REGULAR_COLLISION_RADIUS;
	max_dist *= max_dist;
	// determine whether target is in range
	return dist <= max_dist;
}

/*
std::vector<core::ObjectID> getTargets(Context const & context, ProjectileData
const & projectile, core::Dungeon const & dungeon, sf::Vector2f const & origin)
{
	ASSERT(projectile.bullet != nullptr);

	std::vector<core::ObjectID> objects;
	if (projectile.bullet->radius == 0.f) {
		return objects;
	}

	auto pos = sf::Vector2i{origin};
	auto max_delta = static_cast<int>(std::ceil(projectile.bullet->radius));
	sf::Vector2i dir;
	for (dir.y = -max_delta; dir.y <= max_delta; ++dir.y) {
		for (dir.x = -max_delta; dir.x <= max_delta; ++dir.x) {
			auto tmp = sf::Vector2u{pos + dir};
			if (!dungeon.has(tmp)) {
				// ignore invalid position
				continue;
			}
			auto const & cell = dungeon.getCell(tmp);
			for (auto id: cell.entities) {
				if (canHit(context, projectile, origin, id)) {
					objects.push_back(id);
				}
			}
		}
	}
	return objects;
}
*/

void onCollision(Context& context, core::CollisionEvent const& event) {
	ASSERT(event.actor > 0u);
	auto const& coll = context.collision.query(event.actor);
	ASSERT(coll.is_projectile);
	auto const& projectile = context.projectile.query(event.actor);
	ASSERT(projectile.bullet != nullptr);

	/*
	ASSERT(context.movement.has(event.actor));
	auto const & move = context.movement.query(event.actor);
	auto const & coll = context.collision.query(event.actor);
	ASSERT(coll.is_projectile);
	ASSERT(move.scene > 0u);
	auto const & dungeon = context.dungeons[move.scene];
	auto const & projectile = context.projectile.query(event.actor);
	ASSERT(projectile.bullet != nullptr);

	// determine all targets
	auto targets = getTargets(context, projectile, dungeon, move.pos);

	// trigger combat
	for (auto id: targets) {
		// propagate combat event
		CombatEvent ev;
		ev.actor = event.actor;
		ev.target = id;
		ev.meta_data = projectile.meta_data;
		context.combat_sender.send(ev);
	}

	if (event.collider == 0u && targets.empty()) {
		// trigger bullet destruction in case of tile collision
		ProjectileEvent ev;
		ev.id = event.actor;
		ev.type = ProjectileEvent::Destroy;
		context.projectile_sender.send(ev);
	}
	*/

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
