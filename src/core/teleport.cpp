#include <utils/assert.hpp>
#include <core/collision.hpp>
#include <core/teleport.hpp>

namespace core {

void spawn(Dungeon& dungeon, MovementData& data, sf::Vector2f const& pos) {
	ASSERT(data.scene == 0u);

	// add to new cell
	auto& cell = dungeon.getCell(sf::Vector2u{pos});
	cell.entities.push_back(data.id);

	// update object
	data.pos = pos;
	data.last_pos = data.pos;
	data.scene = dungeon.id;
	data.has_changed = true;
}

void vanish(Dungeon& dungeon, MovementData& data) {
	ASSERT(dungeon.id == data.scene);

	// remove from previous cell
	auto& cell = dungeon.getCell(sf::Vector2u{data.pos});
	auto ok = utils::pop(cell.entities, data.id);
	ASSERT(ok);

	// update object
	data.scene = 0u;
	data.has_changed = true;
}

// ---------------------------------------------------------------------------

SpawnHelper::SpawnHelper(CollisionManager const& collision, MovementManager const & movement,
		Dungeon const& dungeon, ObjectID actor)
	: collision{collision}
	, movement{movement}
	, dungeon{dungeon}
	, actor{actor}
	, result{} {
}

bool SpawnHelper::operator()(sf::Vector2f const& pos) {
	if (!dungeon.has(sf::Vector2u{pos})) {
		// invalid pos
		return false;
	}
	
	// fake movement to target
	MovementData data{movement.query(actor)};
	data.scene = dungeon.id;
	data.pos = pos;
	
	// check collision
	checkAnyCollision(movement, collision, dungeon, data, result);
	return !result.meansCollision();
}

TriggerHelper::TriggerHelper(Dungeon const& dungeon) : dungeon{dungeon} {}

bool TriggerHelper::operator()(sf::Vector2f const& pos) {
	auto& cell = dungeon.getCell(sf::Vector2u{pos});
	if (checkTileCollision(cell)) {
		// ignore unaccessable position
		return false;
	}
	// ignore position if a trigger is already set
	return cell.trigger == nullptr;
}

// ---------------------------------------------------------------------------

TeleportTrigger::TeleportTrigger(TeleportSender& teleport_sender, MovementManager& movement,
	CollisionManager const& collision, DungeonSystem& dungeon,
	utils::SceneID target, sf::Vector2f pos)
	: BaseTrigger{}
	, teleport_sender{teleport_sender}
	, movement{movement}
	, collision{collision}
	, dungeon{dungeon}
	, target{target}
	, pos{pos} {}

void TeleportTrigger::execute(core::ObjectID actor) {
	auto const& coll_data = collision.query(actor);
	if (coll_data.is_projectile) {
		// projectiles cannot be teleported
		return;
	}
	auto& dst = dungeon[target];
	auto p = pos;

	SpawnHelper helper{collision, movement, dst, actor};
	if (!getFreePosition(helper, p, 5u)) {
		// no suitable position found
		return;
	}

	// teleport
	auto& move_data = movement.query(actor);
	auto from = move_data.pos;
	auto& src = dungeon[move_data.scene];
	vanish(src, move_data);
	spawn(dst, move_data, p);

	// propagate teleport
	TeleportEvent tele;
	tele.actor = actor;
	tele.src_scene = src.id;
	tele.src_pos = from;
	tele.dst_scene = target;
	tele.dst_pos = p;
	teleport_sender.send(tele);
}

bool TeleportTrigger::isExpired() const {
	return false;
}

}  // ::core
