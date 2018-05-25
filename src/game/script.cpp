#include <game/script.hpp>

namespace game {

/// @note out of order, needs reimplementation
/*
namespace script_impl {

unsigned int const UPDATE_DELAY = 200; // ms

Context::Context(core::LogContext& log, ScriptManager& script_manager)
	: log{log}
	, script_manager{script_manager}
	, update_delay{sf::Time::Zero} {
}

// ---------------------------------------------------------------------------

void onCollision(Context& context, core::CollisionEvent const& event) {
	if (!context.script_manager.has(event.actor)) {
		// no such component
		return;
	}

	auto& actor = context.script_manager.query(event.actor);
	if (!actor.is_active) {
		// ai is currently disabled
		return;
	}
	ASSERT(actor.api != nullptr);
	ASSERT(actor.script != nullptr);
	auto& script = *actor.script;

	if (event.collider > 0u) {
		script("onObjectCollision", actor.api.get(), event.collider, event.pos);

	} else {
		script("onTileCollision", actor.api.get(), event.pos);
	}

	// notify tracer
	actor.api->tracer.handle(event);
}

void onTeleport(Context& context, core::TeleportEvent const& event) {
	if (!context.script_manager.has(event.actor)) {
		// no such component
		return;
	}

	auto& actor = context.script_manager.query(event.actor);
	if (!actor.is_active) {
		// ai is currently disabled
		return;
	}
	ASSERT(actor.api != nullptr);
	ASSERT(actor.script != nullptr);
	auto& script = *actor.script;
	
	script("onTeleport", actor.api.get(), event.src_scene, event.src_pos,
		event.dst_scene, event.dst_pos);
}

void onIdle(Context& context, ScriptData& actor) {
	if (!actor.is_active) {
		// ai is currently disabled
		return;
	}
	ASSERT(actor.api != nullptr);
	ASSERT(actor.script != nullptr);
	auto& script = *actor.script;

	script("onIdle", actor.api.get());
}

void onMove(Context& context, core::MoveEvent const& event) {
	if (!context.script_manager.has(event.actor)) {
		// no such component
		return;
	}

	auto& actor = context.script_manager.query(event.actor);
	if (!actor.is_active) {
		// ai is currently disabled
		return;
	}
	ASSERT(actor.api != nullptr);
	ASSERT(actor.script != nullptr);
	auto& script = *actor.script;

	switch (event.type) {
		case core::MoveEvent::Left:
			script("onTileLeft", actor.api.get(), event.source);
			break;

		case core::MoveEvent::Reached:
			script("onTileReached", actor.api.get(), event.target);
			break;
	}

	// notify tracer
	actor.api->tracer.handle(event);
}

void onFocus(Context& context, core::FocusEvent const& event) {
	if (context.script_manager.has(event.observer)) {
		auto& actor = context.script_manager.query(event.observer);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		switch (event.type) {
			case core::FocusEvent::Gained:
				script("onGotFocus", actor.api.get(), event.observed);
				break;

			case core::FocusEvent::Lost:
				script("onLostFocus", actor.api.get(), event.observed);
				break;
		}
	}

	if (context.script_manager.has(event.observed)) {
		auto& actor = context.script_manager.query(event.observed);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		switch (event.type) {
			case core::FocusEvent::Gained:
				script("onWasFocused", actor.api.get(), event.observer);
				break;

			case core::FocusEvent::Lost:
				script("onWasUnfocused", actor.api.get(), event.observer);
				break;
		}
	}
}
void onEffect(Context& context, rpg::EffectEvent const& event) {
	if (context.script_manager.has(event.actor)) {
		auto& actor = context.script_manager.query(event.actor);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		switch (event.type) {
			case rpg::EffectEvent::Add:
				script("onEffectReceived", actor.api.get(), event.effect,
					event.causer);
				break;

			case rpg::EffectEvent::Remove:
				script("onEffectFaded", actor.api.get(), event.effect);
				break;
		}
	}

	if (event.causer > 0u && context.script_manager.has(event.causer)) {
		auto& actor = context.script_manager.query(event.causer);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		switch (event.type) {
			case rpg::EffectEvent::Add:
				script("onEffectInflicted", actor.api.get(), event.effect,
					event.actor);
				break;

			default:
				// not handled here
				break;
		}
	}
}

void onStats(Context& context, rpg::StatsEvent const& event) {
	if (context.script_manager.has(event.actor)) {
		auto& actor = context.script_manager.query(event.actor);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		script("onStatsReceived", actor.api.get(), event.delta[rpg::Stat::Life],
			event.delta[rpg::Stat::Mana], event.delta[rpg::Stat::Stamina],
			event.causer);
	}

	if (event.causer > 0u && context.script_manager.has(event.causer)) {
		auto& actor = context.script_manager.query(event.causer);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		script("onStatsInflicted", actor.api.get(),
			event.delta[rpg::Stat::Life], event.delta[rpg::Stat::Mana],
			event.delta[rpg::Stat::Stamina], event.actor);
	}
}

void onDeath(Context& context, rpg::DeathEvent const& event) {
	if (context.script_manager.has(event.actor)) {
		auto& actor = context.script_manager.query(event.actor);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		script("onDeath", actor.api.get(), event.causer);
	}

	if (event.causer > 0u && context.script_manager.has(event.causer)) {
		auto& actor = context.script_manager.query(event.causer);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		script("onEnemyKilled", actor.api.get(), event.actor);
	}
}

void onSpawn(Context& context, rpg::SpawnEvent const& event) {
	if (context.script_manager.has(event.actor)) {
		auto& actor = context.script_manager.query(event.actor);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		script("onSpawned", actor.api.get(), event.causer);
	}

	if (event.causer > 0u && context.script_manager.has(event.causer)) {
		auto& actor = context.script_manager.query(event.causer);
		if (!actor.is_active) {
			// ai is currently disabled
			return;
		}
		ASSERT(actor.api != nullptr);
		ASSERT(actor.script != nullptr);
		auto& script = *actor.script;

		script("onCausedSpawn", actor.api.get(), event.actor);
	}
}

void onFeedback(Context& context, rpg::FeedbackEvent const& event) {
	if (!context.script_manager.has(event.actor)) {
		// actor's ai was dropped
		return;
	}
	
	auto& actor = context.script_manager.query(event.actor);
	if (!actor.is_active) {
		// ai is currently disabled
		return;
	}
	ASSERT(actor.api != nullptr);
	ASSERT(actor.script != nullptr);
	auto& script = *actor.script;
	
	script("onFeedback", actor.api.get(), event.type);
}

void onPathFailed(Context& context, PathFailedEvent const& event) {
	if (!context.script_manager.has(event.actor)) {
		// actor's ai was dropped
		return;
	}

	auto& actor = context.script_manager.query(event.actor);
	if (!actor.is_active) {
		// ai is currently disabled
		return;
	}
	ASSERT(actor.api != nullptr);
	ASSERT(actor.script != nullptr);
	auto& script = *actor.script;

	script("onPathFailed", actor.api.get(), event.pos);
}

void onUpdate(Context& context, ScriptData& actor) {
	if (!actor.is_active) {
		// ai is currently disabled
		return;
	}
	ASSERT(actor.api != nullptr);
	ASSERT(actor.script != nullptr);
	auto& script = *actor.script;

	script("onUpdate", actor.api.get());
}

// ---------------------------------------------------------------------------

void update(Context& context, sf::Time const& elapsed) {
	auto max = sf::milliseconds(script_impl::UPDATE_DELAY);
	context.update_delay += elapsed;
	if (context.update_delay >= max) {
		context.update_delay -= max;
		for (auto& data : context.script_manager) {
			onUpdate(context, data);
		}
	}
}

}  // ::script_impl

// ---------------------------------------------------------------------------

ScriptSystem::ScriptSystem(core::LogContext& log, std::size_t max_objects)
	: utils::EventListener<core::CollisionEvent, core::TeleportEvent,
		core::AnimationEvent, core::MoveEvent, core::FocusEvent,
		rpg::EffectEvent, rpg::StatsEvent, rpg::DeathEvent,
		rpg::SpawnEvent, rpg::FeedbackEvent, PathFailedEvent>{}
	, ScriptManager{max_objects}
	, context{log, *this} {
}

void ScriptSystem::handle(core::CollisionEvent const& event) {
	script_impl::onCollision(context, event);
}

void ScriptSystem::handle(core::TeleportEvent const& event) {
	script_impl::onTeleport(context, event);
}

void ScriptSystem::handle(core::AnimationEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}

	switch (event.type) {
		case core::AnimationEvent::Action:
			switch (event.action) {
				case core::AnimationAction::Idle:
					script_impl::onIdle(context, query(event.actor));
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}
}

void ScriptSystem::handle(core::MoveEvent const& event) {
	script_impl::onMove(context, event);
}

void ScriptSystem::handle(rpg::EffectEvent const& event) {
	script_impl::onEffect(context, event);
}

void ScriptSystem::handle(rpg::StatsEvent const& event) {
	script_impl::onStats(context, event);
}

void ScriptSystem::handle(rpg::DeathEvent const& event) {
	script_impl::onDeath(context, event);
}

void ScriptSystem::handle(rpg::SpawnEvent const& event) {
	script_impl::onSpawn(context, event);
}

void ScriptSystem::handle(rpg::FeedbackEvent const& event) {
	script_impl::onFeedback(context, event);
}

void ScriptSystem::handle(PathFailedEvent const& event) {
	script_impl::onPathFailed(context, event);
}

void ScriptSystem::update(sf::Time const& elapsed) {
	dispatch<core::CollisionEvent>(*this);
	dispatch<core::TeleportEvent>(*this);
	dispatch<core::AnimationEvent>(*this);
	dispatch<core::MoveEvent>(*this);
	dispatch<rpg::EffectEvent>(*this);
	dispatch<rpg::StatsEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<rpg::FeedbackEvent>(*this);
	dispatch<PathFailedEvent>(*this);

	for (auto& data : *this) {
		if (data.is_active) {
			auto failure = data.api->update(elapsed);
			if (failure) {
				handle(*failure);
			}
		}
	}
	script_impl::update(context, elapsed);
}
*/

}  // ::game
