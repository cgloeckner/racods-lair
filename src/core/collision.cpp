#include <utils/algorithm.hpp>
#include <utils/math2d.hpp>
#include <utils/assert.hpp>
#include <core/collision.hpp>
#include <core/teleport.hpp>

namespace core {

CollisionResult::CollisionResult()
	: interrupt{false}
	, tile{false}
	, objects{} {
}

bool CollisionResult::meansCollision() const {
	return tile || !objects.empty();
}

// ---------------------------------------------------------------------------

namespace collision_impl {


// ---------------------------------------------------------------------------

Context::Context(LogContext& log, CollisionSender& collision_sender,
	TeleportSender& teleport_sender, CollisionManager& collision_manager,
	DungeonSystem& dungeon_system, MovementManager const & movement_manager)
	: log{log}
	, collision_sender{collision_sender}
	, teleport_sender{teleport_sender}
	, collision_manager{collision_manager}
	, dungeon_system{dungeon_system}
	, movement_manager{movement_manager}
	, collision_result{} {}

// ---------------------------------------------------------------------------

void checkAnyCollision(Context const & context, MovementData const & actor,
	CollisionResult& result) {
	ASSERT(actor.scene > 0u);
	auto const & scene = context.dungeon_system[actor.scene];
	
	checkAnyCollision(context.movement_manager, context.collision_manager,
		scene, actor, result);
}

bool updateCollisionMap(Context& context, MovementData const & actor) {
	auto src_pos = sf::Vector2u{actor.last_pos};
	auto dst_pos = sf::Vector2u{actor.pos};
	if (src_pos == dst_pos) {
		// no updated necessary
		return false;
	}
	
	ASSERT(actor.scene > 0u);
	auto& scene = context.dungeon_system[actor.scene];
	
	// move object between cells
	auto& source = scene.getCell(src_pos);
	auto& target = scene.getCell(dst_pos);
	bool found = utils::pop(source.entities, actor.id);
	ASSERT(found);
	target.entities.push_back(actor.id);
	return true;
}

void checkAllCollisions(Context& context) {
	auto& result = context.collision_result;
	CollisionEvent event;
	
	// iterate through MoveData to instantly determine whether is_moving or not
	for (auto const & move_data: context.movement_manager) {
		if (move_data.move == sf::Vector2f{}) {
			// object is not moving
			continue;
		}
		
		checkAnyCollision(context, move_data, result);
		
		if (result.tile) {
			context.log.debug << "Tille collision at " << move_data.pos << "\n";
		}
		if (!result.objects.empty()) {
			context.log.debug << "Object collision at " << move_data.pos << " with ";
			for (auto id: result.objects) {
				context.log.debug << id << " ";
			}
			context.log.debug << "\n";
		}
		
		if (result.meansCollision()) {
			ASSERT(context.collision_manager.has(move_data.id));
			auto& actor_coll = context.collision_manager.query(move_data.id);
			
			// propagate collision
			event.actor = move_data.id;
			event.interrupt = result.interrupt;
			if (result.objects.empty()) {
				// propagate tile collision
				event.collider = 0u;
				context.collision_sender.send(event);
			} else {
				// propagate all object collisions
				for (auto id: result.objects) {
					event.collider = id;
					context.collision_sender.send(event);
				}
				if (actor_coll.is_projectile) {
					utils::append(actor_coll.ignore, result.objects);
				}
			}
		}
		
		if (!result.interrupt) {
			/// @note no interrupting collision happened, hence the movement
			///		can be completed by updating the collision map
			auto changed = updateCollisionMap(context, move_data);
			if (changed) {
				// query and execute trigger
				auto& cell = context.dungeon_system[move_data.scene].getCell(sf::Vector2u{move_data.pos});
				if (cell.trigger != nullptr) {
					// note: this might stop the object
					cell.trigger->execute(move_data.id);
					if (cell.trigger->isExpired()) {
						// delete expired trigger
						cell.trigger = nullptr;
					}
				}
			}
		} else {
			/// @note the collision was propagated, so the movement will stop
			///		but the collision system NEEDS to reset the position
			///		immediately to avoid movement artifacts
			///		Both attributes are mutable (to ignore the constness here)
			/*
			context.log.debug << "[Core/Collision] Reset Position of #" << move_data.id << " from "
				<< move_data.pos << " to " << move_data.last_pos << "\n";
			*/
			move_data.pos         = move_data.last_pos;
			move_data.has_changed = true;
		}
	}
}

} // ::collision_impl

// ---------------------------------------------------------------------------

bool checkTileCollision(DungeonCell const& cell) {
	return cell.terrain != Terrain::Floor;
}

bool checkObjectCollision(CollisionManager const & collision_manager, ObjectID actor_id,
		sf::Vector2f actor_pos, ObjectID target_id, sf::Vector2f target_pos) {
	if (!collision_manager.has(actor_id) || !collision_manager.has(target_id)) {
		// at least one of the cannot collide, hence no collision
		return false;
	}
	
	// query data
	auto const & actor_coll = collision_manager.query(actor_id);
	auto const & target_coll = collision_manager.query(target_id);
	
	// perform shape check
	bool found{false};
	if (!actor_coll.shape.is_aabb) {
		// c1 is Circle
		if (!target_coll.shape.is_aabb) {
			found = utils::testCircCirc(actor_pos, actor_coll.shape, target_pos, target_coll.shape);
		} else {
			found = utils::testCircAABB(actor_pos, actor_coll.shape, target_pos, target_coll.shape);
		}
	} else {
		// c1 is AABB
		if (!target_coll.shape.is_aabb) {
			// note: lhs <-> rhs
			found = utils::testCircAABB(target_pos, target_coll.shape, actor_pos, actor_coll.shape);
		} else {
			found = utils::testAABBAABB(actor_pos, actor_coll.shape, target_pos, target_coll.shape);
		}
	}
	
	// consider ignore list
	if (found && utils::contains(actor_coll.ignore, target_id)) {
		found = false;
	}
	
	return found;
}

void checkAnyCollision(MovementManager const & movement_manager, CollisionManager const & collision_manager,
		Dungeon const & scene, MovementData const & actor, CollisionResult& result) {
	result.objects.clear();
	result.tile = false;
	result.interrupt = false;
	
	if (!collision_manager.has(actor.id)) {
		// nothing to handle
		return;
	}
	
	auto const & cell = scene.getCell(sf::Vector2u{actor.pos});
	auto const & coll = collision_manager.query(actor.id);
	
	// stage 1: perform tile collision check
	if (checkTileCollision(cell)) {
		result.tile = true;
		result.interrupt = true;
	}
	
	if (!result.tile || coll.is_projectile) {
		// stage 2: query relevant colliders
		// note: using 2x max collision radius to also detect rare edge cases
		utils::CircEntityQuery<ObjectID> coll_range{actor.pos, coll.shape.radius + MAX_COLLISION_RADIUS};
		scene.traverse(coll_range);
		
		// stage 3: perform multiple object collisions
		for (auto other: coll_range.entities) {
			if (other == actor.id) {
				// don't collide with yourself!
				continue;
			}
			auto const & target = movement_manager.query(other);
			if (checkObjectCollision(collision_manager, actor.id, actor.pos, target.id, target.pos)) {
				auto target_is_projectile = collision_manager.query(target.id).is_projectile;
				
				// test whether object types can collide this way
				if (!coll.is_projectile && target_is_projectile) {
					// ignore collision regular obj -> projectile
					continue;
				}
				
				// test whether it will interrupt a movement
				if (!coll.is_projectile && !target_is_projectile) {
					// regular obj vs. regular obj -> interrupt
					result.interrupt = true;
				}
				
				// object collision detected
				result.objects.push_back(other);
				
				if (!coll.is_projectile && result.interrupt) {
					// break if non-projectile object will be interrupt
					return;
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------

CollisionSystem::CollisionSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon,
	MovementManager const & movement_manager)
	// Event API
	: utils::EventSender<CollisionEvent, TeleportEvent>{}  // Component API
	, CollisionManager{max_objects}
	, passed{sf::Time::Zero}
	, context{log, *this, *this, *this, dungeon, movement_manager} {}

void CollisionSystem::update(sf::Time const& elapsed) {
	collision_impl::checkAllCollisions(context);

	propagate<CollisionEvent>();
	propagate<TeleportEvent>();
}

}  // ::core
