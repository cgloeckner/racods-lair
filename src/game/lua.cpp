#include <utils/math2d.hpp>
#include <core/algorithm.hpp>
#include <core/focus.hpp>
#include <core/teleport.hpp>
#include <core/focus.hpp>
#include <rpg/combat.hpp>
#include <rpg/perk.hpp>
#include <game/entity.hpp>
#include <game/lua.hpp>

namespace game {

float const ENEMY_SPOT_SIGHT_RADIO = 0.75f;

// --------------------------------------------------------------------

LuaApi::LuaApi(core::LogContext& log, core::ObjectID actor, bool hostile,
	rpg::Session const& session, ScriptManager const& script,
	core::InputSender& input_sender, rpg::ActionSender& action_sender,
	rpg::ItemSender& item_sender, PathSystem& path)
	: log{log}
	, id{actor}
	, hostile{hostile}
	, session{session}
	, script{script}
	, input_sender{input_sender}
	, action_sender{action_sender}
	, item_sender{item_sender}
	, tracer{log, path, session.movement, input_sender, actor} {
	ASSERT(actor > 0u);
}

bool LuaApi::isHostile(core::ObjectID target) const {
	if (!script.has(target)) {
		// not a script, not a bot
		return false;
	}
	return script.query(target).api->hostile;
}

sf::Vector2i LuaApi::getMove(core::ObjectID target) const {
	return session.movement.query(target).move;
}

bool LuaApi::hasPath() const {
	return tracer.isRunning() && !tracer.getPath().empty();
}

bool LuaApi::isPathTarget(sf::Vector2u const & pos) const {
	if (!hasPath()) {
		return false;
	}
	auto const& path = tracer.getPath();
	ASSERT(!path.empty());
	return path.front() == pos;
}

sf::Vector2u LuaApi::getPosition(core::ObjectID other) const {
	auto p = session.movement.query(other).pos;
	// return rounded position
	return sf::Vector2u(std::round(p.x), std::round(p.y));
}

utils::SceneID LuaApi::getScene(core::ObjectID other) const {
	return session.movement.query(other).scene;
}

sf::Vector2i LuaApi::getDirection(core::ObjectID id) const {
	auto src = session.movement.query(this->id);
	auto dst = session.movement.query(id);
	auto dir = sf::Vector2i{dst.target} - sf::Vector2i{src.target};
	core::fixDirection(dir);
	return dir;
}

core::ObjectID LuaApi::getFocus() const {
	ASSERT(session.movement.has(id));
	ASSERT(session.focus.has(id));
	
	auto const & stats = session.stats.query(id);
	auto const & move  = session.movement.query(id);
	ASSERT(move.scene > 0u);
	auto const & dungeon = session.dungeon[move.scene];
	
	// query focus
	return core::focus_impl::getFocus(id, dungeon, session.focus, session.movement);
}

float LuaApi::getDistance(core::ObjectID other) const {
	ASSERT(session.movement.has(id));
	ASSERT(session.movement.has(other));
	auto const& self = session.movement.query(id);
	auto const& data = session.movement.query(other);
	return std::sqrt(utils::distance(self.pos, data.pos));
}

float LuaApi::getSight() const {
	return session.focus.query(id).sight;
}

std::vector<core::ObjectID> LuaApi::getEnemies() const {
	ASSERT(session.movement.has(id));
	ASSERT(session.focus.has(id));
	auto const& move = session.movement.query(id);
	auto const& focus = session.focus.query(id);
	auto const max_dist = focus.sight * ENEMY_SPOT_SIGHT_RADIO;

	std::vector<core::ObjectID> enemies;
	if (hostile) {
		// search players directly
		for (auto const& player : session.player) {
			ASSERT(session.movement.has(player.id));
			auto const& other = session.movement.query(player.id);
			auto const dist = utils::distance(move.pos, other.pos);
			if (other.scene == move.scene && dist <= max_dist * max_dist) {
				enemies.push_back(player.id);
			}
			for (auto minion : player.minions) {
				ASSERT(session.movement.has(minion));
				auto const& data = session.movement.query(minion);
				auto const dist = utils::distance(move.pos, data.pos);
				if (data.scene == move.scene && dist <= max_dist * max_dist) {
					enemies.push_back(minion);
				}
			}
		}
	} else {
		// spatial search (e.g. if actor is player's minion)
		ASSERT(move.scene > 0u);
		auto const& dungeon = session.dungeon[move.scene];
		sf::Vector2i d;
		for (d.y = -max_dist; d.y <= max_dist; ++d.y) {
			for (d.x = -max_dist; d.x <= max_dist; ++d.x) {
				auto pos = sf::Vector2u{sf::Vector2i{move.target} + d};
				if (!dungeon.has(pos)) {
					continue;
				}
				auto const& cell = dungeon.getCell(pos);
				for (auto other : cell.entities) {
					if (session.player.has(other)) {
						// igore players
						continue;
					}
					ASSERT(session.movement.has(other));
					auto const& data = session.movement.query(other);
					auto const dist = utils::distance(move.pos, data.pos);
					if (dist <= max_dist * max_dist) {
						enemies.push_back(other);
					}
				}
			}
		}
	}
	return enemies;
}

std::vector<core::ObjectID> LuaApi::getAllies() const {
	ASSERT(session.movement.has(id));
	ASSERT(session.focus.has(id));
	auto const& move = session.movement.query(id);
	auto const& focus = session.focus.query(id);
	auto const max_dist = focus.sight;

	std::vector<core::ObjectID> allies;
	if (hostile) {
		// query players' minions as blacklist
		std::vector<core::ObjectID> blacklist;
		for (auto const& player : session.player) {
			utils::append(blacklist, player.minions);
		}
		// spatial search for other hostile objects
		ASSERT(move.scene > 0u);
		auto const& dungeon = session.dungeon[move.scene];
		sf::Vector2i d;
		for (d.y = -max_dist; d.y <= max_dist; ++d.y) {
			for (d.x = -max_dist; d.x <= max_dist; ++d.x) {
				auto pos = sf::Vector2u{sf::Vector2i{move.target} + d};
				if (!dungeon.has(pos)) {
					continue;
				}
				auto const& cell = dungeon.getCell(pos);
				for (auto other : cell.entities) {
					if (other == id) {
						// ignore self
						continue;
					}
					if (!script.has(other)) {
						// igore objects without ai
						continue;
					}
					if (utils::contains(blacklist, other)) {
						// ignore player's minion
						continue;
					}
					ASSERT(session.movement.has(other));
					auto const& data = session.movement.query(other);
					auto const dist = utils::distance(move.pos, data.pos);
					if (dist <= max_dist * max_dist) {
						allies.push_back(other);
					}
				}
			}
		}
	} else {
		// search for players and their minions
		for (auto const& player : session.player) {
			ASSERT(session.movement.has(player.id));
			auto const& other = session.movement.query(player.id);
			auto const dist = utils::distance(move.pos, other.pos);
			if (other.scene == move.scene && dist <= max_dist) {
				allies.push_back(player.id);
			}
			for (auto minion : player.minions) {
				if (minion == id) {
					// ignore self
					continue;
				}
				ASSERT(session.movement.has(minion));
				auto const& data = session.movement.query(minion);
				auto const dist = utils::distance(move.pos, data.pos);
				if (data.scene == move.scene && dist <= max_dist * max_dist) {
					allies.push_back(minion);
				}
			}
		}
	}
	return allies;
}

rpg::StatsData const& LuaApi::getStats() const {
	ASSERT(session.stats.has(id));
	return session.stats.query(id);
}

bool LuaApi::isAlive(core::ObjectID id) const {
	if (!session.stats.has(id)) {
		return false;
	}
	return session.stats.query(id).stats[rpg::Stat::Life] > 0u;
}

utils::EnumMap<rpg::DamageType, unsigned int> LuaApi::getWeaponDamage(rpg::ItemTemplate const & item) const {
	ASSERT(session.stats.has(id));
	auto const& stats = session.stats.query(id);
	return rpg::combat_impl::getWeaponDamage(stats, item);
}

utils::EnumMap<rpg::DamageType, unsigned int> LuaApi::getPerkDamage(rpg::PerkTemplate const & perk) const {
	ASSERT(session.stats.has(id));
	ASSERT(session.perk.has(id));
	auto const& stats = session.stats.query(id);
	auto const& perks = session.perk.query(id);
	return rpg::combat_impl::getPerkDamage(perks, stats, perk);
}

utils::EnumMap<rpg::Stat, int> LuaApi::getPerkRecovery(rpg::PerkTemplate const & perk) const {
	ASSERT(session.stats.has(id));
	ASSERT(session.perk.has(id));
	auto const& stats = session.stats.query(id);
	auto const& perks = session.perk.query(id);
	return rpg::combat_impl::getPerkRecovery(perks, stats, perk);
}

rpg::ItemTemplate const* LuaApi::getEquipment(rpg::EquipmentSlot slot) const {
	ASSERT(slot != rpg::EquipmentSlot::None);
	ASSERT(session.item.has(id));
	auto const& data = session.item.query(id);
	return data.equipment[slot];
}

std::vector<rpg::Item> LuaApi::getWeapons() const {
	ASSERT(session.item.has(id));
	auto const& data = session.item.query(id);
	return data.inventory[rpg::ItemType::Weapon];
}

std::vector<rpg::Item> LuaApi::getArmors() const {
	ASSERT(session.item.has(id));
	auto const& data = session.item.query(id);
	return data.inventory[rpg::ItemType::Armor];
}

std::vector<rpg::Item> LuaApi::getPotions() const {
	ASSERT(session.item.has(id));
	auto const& data = session.item.query(id);
	return data.inventory[rpg::ItemType::Potion];
}

std::vector<rpg::Perk> LuaApi::getPerks() const {
	ASSERT(session.perk.has(id));
	auto const& data = session.perk.query(id);
	return data.perks;
}

void LuaApi::navigate(sf::Vector2u const & target) {
	ASSERT(session.movement.has(id));
	// stop previous current movement
	stop();
	
	if (tracer.isRunning()) {
		return;
	}
	
	auto const& data = session.movement.query(id);
	if (navigator_impl::distance(data.target, target) >= 2.f) {
		tracer.pathfind(target);
	}
}

void LuaApi::move(sf::Vector2i dir) {
	if (hasPath()) {
		tracer.reset();
	}
	core::fixDirection(dir);
	core::InputEvent event;
	event.actor = id;
	event.move = dir;
	input_sender.send(event);
}

void LuaApi::moveTowards(sf::Vector2u pos) {
	auto origin = getPosition(id);
	auto dir = sf::Vector2i{pos} - sf::Vector2i{origin};
	move(dir);
}

void LuaApi::look(sf::Vector2i dir) {
	if (hasPath()) {
		tracer.reset();
	}
	core::fixDirection(dir);
	core::InputEvent event;
	event.actor = id;
	event.look = dir;
	input_sender.send(event);
}

void LuaApi::lookTowards(sf::Vector2u pos) {
	auto origin = getPosition(id);
	auto dir = sf::Vector2i{pos} - sf::Vector2i{origin};
	look(dir);
}

void LuaApi::stop() {
	core::InputEvent event;
	event.actor = id;
	event.move = {};
	event.look = {};
	input_sender.send(event);
	tracer.reset();
}

void LuaApi::attack() {
	rpg::ActionEvent event;
	event.actor = id;
	event.idle = false;
	event.action = rpg::PlayerAction::Attack;
	action_sender.send(event);
}

void LuaApi::useItem(rpg::ItemTemplate const & item) {
	rpg::ItemEvent event;
	event.actor = id;
	event.item = &item;
	event.type = rpg::ItemEvent::Use;
	event.slot = item.slot;
	item_sender.send(event);
}

void LuaApi::usePerk(rpg::PerkTemplate const & perk) {
	auto const & perk_data = session.perk.query(id);
	if (!rpg::hasPerk(perk_data, perk)) {
		return;
	}
	
	// assign this perk to quickslot
	auto& qslot = session.quickslot.query(id);
	++qslot.slot_id;
	qslot.slot_id %= rpg::MAX_QUICKSLOTS;
	qslot.slots[qslot.slot_id] = rpg::Shortcut{perk};
	
	// trigger slot use
	rpg::ActionEvent event;
	event.actor = id;
	event.idle = false;
	event.action = rpg::PlayerAction::UseSlot;
	action_sender.send(event);
}

boost::optional<PathFailedEvent> LuaApi::update(sf::Time const& elapsed) {
	// update path tracer
	return tracer.update();
}

}  // ::rage
