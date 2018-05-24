#include <utils/math2d.hpp>
#include <core/animation.hpp>
#include <core/focus.hpp>
#include <rpg/delay.hpp>
#include <rpg/combat.hpp>

namespace rpg {

namespace delay_impl {

Context::Context(core::LogContext& log, core::AnimationSender& animation_sender,
	CombatSender& combat_sender, ProjectileSender& projectile_sender,
	InteractSender& interact_sender, core::DungeonSystem const& dungeon,
	core::MovementManager const& movement, core::FocusManager const& focus,
	core::AnimationManager const& animation, ItemManager const& item,
	StatsManager const& stats, InteractManager const& interact,
	PlayerManager const& player)
	: log{log}
	, animation_sender{animation_sender}
	, combat_sender{combat_sender}
	, projectile_sender{projectile_sender}
	, interact_sender{interact_sender}
	, dungeon{dungeon}
	, movement{movement}
	, focus{focus}
	, animation{animation}
	, item{item}
	, stats{stats}
	, interact{interact}
	, player{player}
	, combats{}
	, projectiles{}
	, interacts{} {}

// ---------------------------------------------------------------------------

sf::Time getDelayDuration(core::AnimationManager const& animation, core::ObjectID actor, core::AnimationAction action) {
	auto const& data = animation.query(actor);
	return core::getDuration(data, action) * 0.75f;
}

core::ObjectID queryInteractable(Context& context, core::ObjectID actor) {
	auto const& actor_move = context.movement.query(actor);
	auto const& actor_focus = context.focus.query(actor);
	ASSERT(actor_move.scene > 0u);
	auto const& dungeon = context.dungeon[actor_move.scene];
	
	// query all entities in range
	utils::CircEntityQuery<core::ObjectID> query{actor_move.pos, combat_impl::MAX_MELEE_DISTANCE};
	dungeon.traverse(query);
	
	// seek interactables
	std::vector<InteractData const*> objects;
	bool found_barrier{false};
	for (auto id : query.entities) {
		if (!context.interact.has(id)) {
			continue;
		}
		// check fov
		auto const & move = context.movement.query(id);
		if (utils::isWithinFov(actor_move.pos, sf::Vector2f{actor_move.look}, actor_focus.fov, combat_impl::MAX_MELEE_DISTANCE, move.pos)) {
			auto ptr = &context.interact.query(id);
			objects.push_back(ptr);
			found_barrier = found_barrier || (ptr->type == InteractType::Barrier);
		}
	}

	// pick most recent interactable
	if (found_barrier) {
		// search any barrier
		for (auto ptr : objects) {
			if (ptr->type == InteractType::Barrier) {
				return ptr->id;
			}
		}

	} else if (context.player.has(actor)) {
		core::ObjectID best_id{0u};
		float best_pos{-1.f};
		// search corpse that has not been looted yet
		auto player_id = context.player.query(actor).player_id;
		for (auto ptr : objects) {
			ASSERT(ptr->loot.size() <= player_id);
			if (ptr->loot[player_id - 1u].empty()) {
				// ignore corpse without loot for this player
				continue;
			}
			// search for best/closest target
			auto const & move_other = context.movement.query(ptr->id);
			auto pos_eval = utils::evalPos(actor_move.pos, sf::Vector2f{actor_move.look}, actor_focus.fov, actor_focus.sight, move_other.pos);
			
			auto delta = move_other.pos - actor_move.pos;
			auto dist  = thor::squaredLength(delta);
			auto angle = std::abs(thor::signedAngle(sf::Vector2f{actor_move.look}, delta));
			
			if ((pos_eval < best_pos) || (best_pos < 0.f)) {
				best_pos = pos_eval;
				best_id  = ptr->id;
			}
		}
		return best_id;
	
	}
	
	// nothing found
	return 0u;
}

core::ObjectID queryAttackable(Context& context, core::ObjectID actor) {
	auto const& actor_move = context.movement.query(actor);
	auto const& actor_focus = context.focus.query(actor);
	ASSERT(actor_move.scene > 0u);
	auto const& dungeon = context.dungeon[actor_move.scene];
	
	// query all entities in range
	utils::CircEntityQuery<core::ObjectID> query{actor_move.pos, combat_impl::MAX_MELEE_DISTANCE};
	dungeon.traverse(query);
	
	core::ObjectID target{0u};
	float closest{std::numeric_limits<float>::max()};
	for (auto id : query.entities) {
		if (id == actor) {
			continue;
		}
		// check attackability
		if (!context.stats.has(id)) {
			continue;
		}
		// check fov
		auto const & move = context.movement.query(id);
		auto value = utils::evalPos(actor_move.pos, sf::Vector2f{actor_move.look}, actor_focus.fov, combat_impl::MAX_MELEE_DISTANCE, move.pos);
		if (value >= 0.f && value < closest) {
			auto const & stats = context.stats.query(id);
			if (stats.stats[Stat::Life] > 0) {
				target = id;
				closest = value;
			}
		}
	}
	
	return target;
}

void onAttack(Context& context, core::ObjectID actor) {
	core::AnimationEvent ani;
	auto const& ani_data = context.animation.query(actor);
	// auto const & focus = context.focus.query(actor);
	auto const& item = context.item.query(actor);
	auto primary = item.equipment[EquipmentSlot::Weapon];
	if (primary == nullptr || primary->melee) {
		ani.action = core::AnimationAction::Melee;
		auto delay = core::getDuration(ani_data, ani.action) * 0.75f;

		/*
		if (focus.focus > 0u) {
			// delay combat calculation
			CombatEvent event;
			event.actor = actor;
			event.target = focus.focus;
			event.meta_data.emitter = EmitterType::Weapon;
			event.meta_data.primary = primary;
			event.meta_data.secondary =
		item.equipment[EquipmentSlot::Extension];
			context.combats.push(event, delay);
		}
		*/

		// delay combat calculation
		CombatEvent event;
		event.actor = actor;
		event.target = 0u;  // target is specified later
		event.meta_data.emitter = EmitterType::Weapon;
		event.meta_data.primary = primary;
		event.meta_data.secondary = item.equipment[EquipmentSlot::Extension];
		context.combats.push(event, delay);

	} else {
		ani.action = core::AnimationAction::Range;
		auto delay = core::getDuration(ani_data, ani.action) * 0.75f;
		auto const& move_data = context.movement.query(actor);

		/*
		auto pos = move_data.pos;
		pos.x = std::round(pos.x);
		pos.y = std::round(pos.y);
		*/

		// delay projectile creation
		ProjectileEvent event;
		event.type = ProjectileEvent::Create;
		event.id = actor;  // owner
		event.spawn.scene = move_data.scene;
		// note: position and direction are assigned later, because owner
		// specifies it
		// event.spawn.pos = sf::Vector2u{sf::Vector2i{pos} + focus.look};
		// event.spawn.direction = focus.look;
		event.meta_data.emitter = EmitterType::Weapon;
		event.meta_data.primary = primary;
		context.projectiles.push(event, delay);
	}

	// trigger suitable animation
	ani.actor = actor;
	ani.type = core::AnimationEvent::Action;
	context.animation_sender.send(ani);
}

void onInteract(Context& context, core::ObjectID actor) {
	// trigger use animation
	core::AnimationEvent ani_event;
	ani_event.actor = actor;
	ani_event.type = core::AnimationEvent::Action;
	ani_event.action = core::AnimationAction::Use;
	context.animation_sender.send(ani_event);

	// calculate delay
	auto delay = delay_impl::getDelayDuration(context.animation, actor, ani_event.action);

	// schedule interact event
	InteractEvent event;
	event.actor = actor;
	event.target = 0;  // target is specified later
	context.interacts.push(event, delay);
}

void onPerk(Context& context, core::ObjectID actor, PerkTemplate const& perk) {
	// trigger casting animation
	core::AnimationEvent ani_event;
	ani_event.actor = actor;
	ani_event.type = core::AnimationEvent::Action;
	ani_event.action = core::AnimationAction::Magic;
	context.animation_sender.send(ani_event);

	// calculate delay
	auto const& ani_data = context.animation.query(actor);
	auto delay = core::getDuration(ani_data, ani_event.action) * 0.75f;

	if (perk.bullet.bullet == nullptr) {
		// specify target
		core::ObjectID target = 0u;
		switch (perk.type) {
			case PerkType::Self:
				// apply to self
				target = actor;
				break;

			default:
				// real target is specified later
				break;
		}

		// trigger delayed combat
		CombatEvent combat_event;
		combat_event.actor = actor;
		combat_event.target = target;
		combat_event.meta_data.emitter = EmitterType::Perk;
		combat_event.meta_data.perk = &perk;
		context.combats.push(combat_event, delay);

	} else {
		auto const& move_data = context.movement.query(actor);

		// trigger projectile creation
		ProjectileEvent proj_event;
		proj_event.type = ProjectileEvent::Create;
		proj_event.id = actor;  // owner
		proj_event.spawn.scene = move_data.scene;
		// note: position and direction are assigned later, because owner
		// specifies it;
		proj_event.meta_data.emitter = EmitterType::Perk;
		proj_event.meta_data.perk = &perk;
		context.projectiles.push(proj_event, delay);
	}
}

void onUpdate(Context& context, sf::Time const& elapsed) {
	// cooldown scheduled events
	context.combats(elapsed);
	context.projectiles(elapsed);
	context.interacts(elapsed);

	// propagate recent combat events
	for (auto event : context.combats.ready) {
		// seek suitable targets
		if (event.target == 0u) {
			// trigger target
			auto target = queryAttackable(context, event.actor);
			if (target > 0u) {
				event.target = target;
				context.combat_sender.send(event);
			}
		} else {
			// use specified target (e.g. defensive perk)
			context.combat_sender.send(event);
		}
	}
	context.combats.ready.clear();

	// propagate recent projectile events
	for (auto const& event : context.projectiles.ready) {
		context.projectile_sender.send(event);
	}
	context.projectiles.ready.clear();

	// propagate recent interact event
	for (auto event : context.interacts.ready) {
		// seek suitable target
		event.target = queryInteractable(context, event.actor);
		if (event.target == 0u) {
			continue;
		}
		context.interact_sender.send(event);
	}
	context.interacts.ready.clear();
}

}  // ::delay_impl

// ---------------------------------------------------------------------------

DelaySystem::DelaySystem(core::LogContext& log,
	core::DungeonSystem const& dungeon, core::MovementManager const& movement,
	core::FocusManager const& focus, core::AnimationManager const& animation,
	ItemManager const& item, StatsManager const& stats,
	InteractManager const& interact, PlayerManager const& player)
	: utils::EventListener<ActionEvent, PerkEvent>{}
	, utils::EventSender<core::AnimationEvent, CombatEvent, ProjectileEvent,
		  InteractEvent>{}
	, context{log, *this, *this, *this, *this, dungeon, movement, focus,
		  animation, item, stats, interact, player} {}

void DelaySystem::reset() {
	context.combats.reset();
	context.projectiles.reset();
	context.interacts.reset();
}

void DelaySystem::handle(ActionEvent const& event) {
	if (!context.item.has(event.actor)) {
		// no such component
		return;
	}

	switch (event.action) {
		case PlayerAction::Attack:
			delay_impl::onAttack(context, event.actor);
			break;

		case PlayerAction::Interact:
			delay_impl::onInteract(context, event.actor);
			break;

		default:
			// not handled here
			break;
	}
}

void DelaySystem::handle(PerkEvent const& event) {
	if (!context.movement.has(event.actor)) {
		// no such component
		return;
	}

	if (event.type == PerkEvent::Use) {
		ASSERT(event.perk != nullptr);
		delay_impl::onPerk(context, event.actor, *event.perk);
	}
}

void DelaySystem::update(sf::Time const& elapsed) {
	dispatch<ActionEvent>(*this);
	dispatch<PerkEvent>(*this);

	delay_impl::onUpdate(context, elapsed);

	propagate<core::AnimationEvent>();
	propagate<CombatEvent>();
	propagate<ProjectileEvent>();
	propagate<InteractEvent>();
}

}  // ::game
