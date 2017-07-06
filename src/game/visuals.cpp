#include <game/visuals.hpp>

namespace game {

namespace visuals_impl {

float const BRIGHTNESS_ON_DEATH = 0.5f;

Context::Context(core::LogContext& log, core::RenderManager const & render_manager,
	core::AnimationSender& animation_sender)
	: log{log}
	, render_manager{render_manager}
	, animation_sender{animation_sender} {
}

// ---------------------------------------------------------------------------

void onDamaged(Context& context, rpg::StatsEvent const& event) {
	// nothing implemented yet
}

void onKilled(Context& context, rpg::DeathEvent const& event) {
	// note: there could also be an effect done to the causer here

	// fade out target's light intensity
	core::AnimationEvent ev;
	ev.actor = event.actor;
	ev.type = core::AnimationEvent::LightIntensity;
	ev.interval.min = 0.f;
	ev.interval.current = 1.f;
	ev.interval.max = 1.f;
	ev.interval.speed = 1.f / FADE_DELAY;
	ev.interval.rise = false;
	ev.interval.repeat = 1;
	context.animation_sender.send(ev);
	
	// fade out target's color
	ev.type = core::AnimationEvent::Brightness;
	ev.interval.min = BRIGHTNESS_ON_DEATH;
	ev.interval.current = 1.f;
	ev.interval.max = 1.f;
	ev.interval.speed = 0.7f / FADE_DELAY;
	context.animation_sender.send(ev);
}

void onSpawn(Context& context, rpg::SpawnEvent const& event) {
	// note: there could also be an effect done to the causer here

	// fade in target's light intensity
	core::AnimationEvent ev;
	ev.actor = event.actor;
	ev.type = core::AnimationEvent::LightIntensity;
	ev.interval.min = 0.f;
	ev.interval.current = 0.f;
	ev.interval.max = 1.f;
	ev.interval.speed = 1.f / FADE_DELAY;
	ev.interval.rise = true;
	ev.interval.repeat = 1;
	context.animation_sender.send(ev);
	
	// fade in target's color
	ev.type = core::AnimationEvent::Brightness;
	ev.interval.min = BRIGHTNESS_ON_DEATH;
	ev.interval.current = BRIGHTNESS_ON_DEATH;
	ev.interval.max = 1.f;
	ev.interval.speed = 10.f / FADE_DELAY; // very fast!
	context.animation_sender.send(ev);
	
	auto const & render = context.render_manager.query(event.actor);
	if (render.light != nullptr) {
		ev.type = core::AnimationEvent::LightRadius;
		ev.interval = utils::IntervalState{1.f};
		ev.interval.min = 0.95f * render.light->radius;
		ev.interval.max = 1.05f * render.light->radius;
		ev.interval.speed = thor::random(0.005f, 0.010f);
		ev.interval.rise = false;
		ev.interval.repeat = -1;
		context.animation_sender.send(ev);
	}
}

void onExploded(Context& context, core::ObjectID id) {
	// start alpha fading animation
	core::AnimationEvent event;
	event.actor = id;
	event.type = core::AnimationEvent::Alpha;
	event.interval = utils::IntervalState{1.f};
	event.interval.min = 0.f;
	event.interval.max = 1.f;
	event.interval.speed = 2.f / FADE_DELAY; // fast!
	event.interval.rise = false;
	event.interval.repeat = 1;
	context.animation_sender.send(event);
	
	auto const & render = context.render_manager.query(id);
	if (render.light != nullptr) {
		// start light fading animation
		auto radius = render.light->radius;
		event.type = core::AnimationEvent::LightRadius;
		event.interval.current = radius;
		event.interval.max = radius;
		event.interval.speed = radius / FADE_DELAY;
		event.interval.rise = false;
		context.animation_sender.send(event);
	}
}

}  // ::visuals_impl

// ---------------------------------------------------------------------------

VisualsSystem::VisualsSystem(core::LogContext& log, core::RenderManager const & render_manager)
	: utils::EventListener<rpg::StatsEvent, rpg::DeathEvent, rpg::SpawnEvent,
		  rpg::ProjectileEvent>{}
	, utils::EventSender<core::AnimationEvent>{}
	, context{log, render_manager, *this} {}

void VisualsSystem::handle(rpg::StatsEvent const& event) {
	if (event.delta[rpg::Stat::Life] < 0) {
		visuals_impl::onDamaged(context, event);
	}
}

void VisualsSystem::handle(rpg::DeathEvent const& event) {
	visuals_impl::onKilled(context, event);
}

void VisualsSystem::handle(rpg::SpawnEvent const& event) {
	visuals_impl::onSpawn(context, event);
}

void VisualsSystem::handle(rpg::ProjectileEvent const& event) {
	switch (event.type) {
		case rpg::ProjectileEvent::Destroy:
			visuals_impl::onExploded(context, event.id);
			break;

		default:
			// others are not handled here
			break;
	}
}

sf::Time VisualsSystem::update(sf::Time const& elapsed) {
	sf::Clock clock;

	dispatch<rpg::StatsEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<rpg::ProjectileEvent>(*this);

	propagate<core::AnimationEvent>();

	return clock.restart();
}

}  // ::rage
