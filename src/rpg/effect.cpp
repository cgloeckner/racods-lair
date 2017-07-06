#include <utils/algorithm.hpp>
#include <rpg/effect.hpp>

namespace rpg {

namespace effect_impl {

unsigned int const MIN_ELAPSED_TIME = 2000;

Context::Context(core::LogContext& log, BoniSender& boni_sender,
	CombatSender& combat_sender, EffectSender& effect_sender)
	: log{log}
	, boni_sender{boni_sender}
	, combat_sender{combat_sender}
	, effect_sender{effect_sender} {}

void addEffect(
	Context& context, EffectData& actor, EffectTemplate const& effect) {
	// search existing effect
	auto i = utils::find_if(actor.effects,
		[&](Effect const& node) { return node.effect == &effect; });
	if (i == actor.effects.end()) {
		// create new node
		actor.effects.emplace_back(effect);
		// propagate add boni
		BoniEvent event;
		event.actor = actor.id;
		event.type = BoniEvent::Add;
		event.boni = &effect.boni;
		context.boni_sender.send(event);

	} else {
		// update remaining time
		i->remain = effect.duration;
	}
}

void removeEffect(
	Context& context, EffectData& actor, EffectTemplate const& effect) {
	// search existing effect
	auto i = utils::find_if(actor.effects,
		[&](Effect const& node) { return node.effect == &effect; });
	if (i != actor.effects.end()) {
		// propagate remove boni
		BoniEvent event;
		event.actor = actor.id;
		event.type = BoniEvent::Remove;
		event.boni = &effect.boni;
		context.boni_sender.send(event);

		// stably remove from active effects
		utils::pop(actor.effects, i, true);
	}
}

void onDeath(Context& context, EffectData& actor) {
	actor.effects.clear();
	actor.cooldown = sf::Time::Zero;
}

// ---------------------------------------------------------------------------

void handleEffects(Context& context, EffectData& actor, sf::Time const& step) {
	// handle each effect once
	for (auto& node : actor.effects) {
		ASSERT(node.effect != nullptr);
		// trigger combat
		CombatEvent event;
		event.target = actor.id;
		event.meta_data.emitter = EmitterType::Effect;
		event.meta_data.effect = node.effect;
		context.combat_sender.send(event);
		// decrease remaining time
		node.remain -= step;
		if (node.remain < sf::Time::Zero) {
			node.remain = sf::Time::Zero;
		}
	}
	// remove finished effects (if non-perpetual)
	utils::remove_if(actor.effects, [&](Effect const& node) {
		if (node.remain == sf::Time::Zero &&
			node.effect->duration != sf::Time::Zero) {
			// propagate remove boni
			BoniEvent event;
			event.actor = actor.id;
			event.type = BoniEvent::Remove;
			event.boni = &node.effect->boni;
			context.boni_sender.send(event);
			// propagate effect remove
			EffectEvent ev;
			ev.actor = actor.id;
			ev.type = EffectEvent::Remove;
			ev.effect = node.effect;
			context.effect_sender.send(ev);
			// note: return true will remove this effect
			return true;
		}
		return false;
	});
}

void onUpdate(Context& context, EffectData& actor, sf::Time const& elapsed) {
	auto const step = sf::milliseconds(MIN_ELAPSED_TIME);
	// "charge" until next cycle
	actor.cooldown += elapsed;
	if (actor.cooldown >= step) {
		// apply cycle
		handleEffects(context, actor, step);
		actor.cooldown -= step;
	}
}

}  // ::effect_impl

// ---------------------------------------------------------------------------

EffectSystem::EffectSystem(core::LogContext& log, std::size_t max_objects)
	: utils::EventListener<EffectEvent, DeathEvent>{}
	, utils::EventSender<BoniEvent, CombatEvent, EffectEvent>{}
	, EffectManager{max_objects}
	, context{log, *this, *this, *this} {}

void EffectSystem::handle(EffectEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	ASSERT(event.effect != nullptr);
	auto& actor = query(event.actor);

	switch (event.type) {
		case EffectEvent::Add:
			effect_impl::addEffect(context, actor, *event.effect);
			break;

		case EffectEvent::Remove:
			effect_impl::removeEffect(context, actor, *event.effect);
			break;
	}
}

void EffectSystem::handle(DeathEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	effect_impl::onDeath(context, actor);
}

void EffectSystem::update(sf::Time const& elapsed) {
	dispatch<EffectEvent>(*this);
	dispatch<DeathEvent>(*this);

	for (auto& data : *this) {
		effect_impl::onUpdate(context, data, elapsed);
	}

	propagate<BoniEvent>();
	propagate<CombatEvent>();
	propagate<EffectEvent>();
}

}  // ::game
