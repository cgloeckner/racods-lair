#include <utils/algorithm.hpp>
#include <utils/assert.hpp>
#include <core/collision.hpp>
#include <core/teleport.hpp>

namespace core {

namespace collision_impl {

float const REGULAR_COLLISION_RADIUS = 0.5f;
float const MAX_PROJECTILE_RADIUS = 0.5f;

// ---------------------------------------------------------------------------

Context::Context(LogContext& log, CollisionSender& collision_sender,
	MoveSender& move_sender, TeleportSender& teleport_sender,
	CollisionManager& collision_manager, DungeonSystem& dungeon_system,
	MovementManager const& movement_manager)
	: log{log}
	, collision_sender{collision_sender}
	, move_sender{move_sender}
	, teleport_sender{teleport_sender}
	, collision_manager{collision_manager}
	, dungeon_system{dungeon_system}
	, movement_manager{movement_manager} {}

// ---------------------------------------------------------------------------

void updateCollisionMap(
	Context const& context, CollisionData const& data, MoveEvent const& event) {
	ASSERT(context.movement_manager.has(data.id));
	auto const& move_data = context.movement_manager.query(data.id);
	ASSERT(move_data.scene > 0u);
	auto& dungeon = context.dungeon_system[move_data.scene];

	// move object between cells
	auto& source = dungeon.getCell(event.source);
	auto& target = dungeon.getCell(event.target);
	bool found = utils::pop(source.entities, data.id);
	ASSERT(found);
	target.entities.push_back(data.id);
}

void onTileLeft(
	Context& context, CollisionData const& data, MoveEvent const& event) {
	ASSERT(context.movement_manager.has(data.id));
	auto const& move_data = context.movement_manager.query(data.id);
	if (move_data.scene == 0u) {
		// object has been removed yet
		return;
	}
	auto& dungeon = context.dungeon_system[move_data.scene];
	ASSERT(dungeon.has(move_data.target));
	auto& cell = dungeon.getCell(move_data.target);

	// perform collision check only for regular objects
	if (!data.is_projectile) {
		// check for terrain collision
		bool tile_collision = checkTileCollision(cell);
		std::vector<ObjectID> colliders;
		if (!tile_collision) {
			// check for object collision if no tile collision occured
			colliders = checkObjectCollision(context.collision_manager, cell, data);
		}
		if (tile_collision || !colliders.empty()) {
			// propagate actor's collision
			CollisionEvent coll;
			coll.actor = data.id;
			if (colliders.empty()) {
				coll.collider = 0u;
			} else {
				// pick any collider
				coll.collider = colliders.front();
			}
			coll.pos = event.target;
			coll.reset_to = event.source;
			coll.reset = true;
			context.collision_sender.send(coll);
			// break to avoid changing collision map and event forwarding
			return;
		}
	}

	// modify collision flag
	updateCollisionMap(context, data, event);

	// forward move event
	context.move_sender.send(event);
}

void onTileReached(
	Context& context, CollisionData const& data, MoveEvent const& event) {
	ASSERT(context.movement_manager.has(data.id));
	auto const& move_data = context.movement_manager.query(data.id);
	if (move_data.scene == 0u) {
		// object already vanished
		return;
	}

	ASSERT(context.dungeon_system[move_data.scene].has(event.target));
	auto& cell = context.dungeon_system[move_data.scene].getCell(event.target);

	if (data.is_projectile) {
		// test for bullet's tile collision
		if (checkTileCollision(cell)) {
			// propagate bullet's tile collision
			CollisionEvent coll;
			coll.actor = data.id;
			coll.collider = 0;
			coll.pos = event.target;
			coll.reset_to = event.target;
			coll.reset = true;
			context.collision_sender.send(coll);
			// break to avoid event forwarding
			return;
		}
	}

	// forward move event
	context.move_sender.send(event);
}

void onBulletCheck(Context& context, CollisionData& data) {
	if (!data.is_projectile) {
		// regular object's collision is not checked here
		return;
	}
	// check for closest target
	auto objects = checkBulletCollision(context.collision_manager,
		context.movement_manager, context.dungeon_system, data);
	for (auto id : objects) {
		// propagate collision
		CollisionEvent event;
		event.actor = data.id;
		event.collider = id;
		event.reset = false;
		context.collision_sender.send(event);
	}
	// ignore objects for future checks
	utils::append(data.ignore, objects);
}

}  // ::collision_impl

// ---------------------------------------------------------------------------

bool checkTileCollision(DungeonCell const& cell) {
	return cell.terrain != Terrain::Floor;
}

std::vector<ObjectID> checkObjectCollision(CollisionManager const& manager,
	DungeonCell const& cell, CollisionData const& data) {
	ASSERT(!data.is_projectile);
	std::vector<ObjectID> result;
	result.reserve(cell.entities.size());
	for (auto id : cell.entities) {
		if (id == data.id) {
			// do not collide with yourself
			continue;
		}
		if (utils::contains(data.ignore, id)) {
			// object is ignored by the actor
			continue;
		}
		if (!manager.has(id)) {
			// object has no collision component
			continue;
		}
		auto& other = manager.query(id);
		if (other.is_projectile) {
			// bullet collision is handled somewhere else
			continue;
		}
		result.push_back(id);
	}

	return result;
}

std::vector<ObjectID> checkBulletCollision(CollisionManager const& collision,
	MovementManager const& movement, DungeonSystem const& dungeon,
	CollisionData const& data) {
	ASSERT(data.is_projectile);
	std::vector<ObjectID> objects;

	auto move_data = movement.query(data.id);
	if (move_data.scene == 0u) {
		// bullet has already been vanished
		return objects;
	}
	auto const& scene = dungeon[move_data.scene];
	sf::Vector2i dir;
	auto distance =
		data.radius + core::collision_impl::REGULAR_COLLISION_RADIUS;
	auto max_delta = static_cast<int>(std::ceil(distance));
	distance *= distance;  // using squared distances

	// search neighborhood for any object in collision range
	for (dir.y = -max_delta; dir.y <= max_delta; ++dir.y) {
		for (dir.x = -max_delta; dir.x <= max_delta; ++dir.x) {
			auto pos = sf::Vector2u{sf::Vector2i{move_data.pos} + dir};
			if (!scene.has(pos)) {
				// ignore invalid cell
				continue;
			}
			auto const& cell = scene.getCell(pos);
			for (auto other : cell.entities) {
				if (other == data.id) {
					// cannot collide with self
					continue;
				}
				if (!collision.has(other)) {
					// object cannot collide
					continue;
				}
				if (collision.query(other).is_projectile) {
					// bullets never collide each other
					continue;
				}
				if (utils::contains(data.ignore, other)) {
					// object was already hit by this bullet
					continue;
				}
				auto current =
					utils::distance(move_data.pos, movement.query(other).pos);
				if (current <= distance) {
					objects.push_back(other);
				}
			}
		}
	}

	return objects;
}

// ---------------------------------------------------------------------------

CollisionSystem::CollisionSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon,
	MovementManager const& movement_manager)
	// Event API
	: utils::EventListener<MoveEvent>{}
	, utils::EventSender<CollisionEvent, MoveEvent, TeleportEvent>{}  // Component API
	, CollisionManager{max_objects}
	, passed{sf::Time::Zero}
	, context{log, *this, *this, *this, *this, dungeon, movement_manager} {}

void CollisionSystem::handle(MoveEvent const& event) {
	if (!has(event.actor)) {
		// object has no collision component
		// note: object might have been deleted
		return;
	}
	auto const& data = query(event.actor);

	switch (event.type) {
		case MoveEvent::Left:
			collision_impl::onTileLeft(context, data, event);
			break;

		case MoveEvent::Reached:
			collision_impl::onTileReached(context, data, event);
			break;
	}
}

void CollisionSystem::update(sf::Time const& elapsed) {
	dispatch<MoveEvent>(*this);

	passed += elapsed;
	if (passed.asMilliseconds() >= MAX_FRAMETIME_MS) {
		passed -= sf::milliseconds(static_cast<unsigned int>(MAX_FRAMETIME_MS));
		// test for bullets' collisions
		for (auto& data : *this) {
			collision_impl::onBulletCheck(context, data);
		}
	}

	propagate<CollisionEvent>();
	propagate<MoveEvent>();
	propagate<TeleportEvent>();
}

}  // ::core
