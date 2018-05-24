#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

#include <core/animation.hpp>

namespace core {

AnimationSystem::AnimationSystem(LogContext& log, std::size_t max_objects, MovementManager const & movement_manager)
	// Event API
	: utils::EventListener<AnimationEvent>{}
	, utils::EventSender<AnimationEvent>{}  // Component API
	, AnimationManager{max_objects}
	, context{log, *this, movement_manager, *this} {}

void AnimationSystem::handle(AnimationEvent const& event) {
	if (!has(event.actor)) {
		// object has no animation component
		return;
	}
	auto& data = query(event.actor);

	switch (event.type) {
		case AnimationEvent::Action:
			animation_impl::trigger(context, data, event.action);
			break;

		case AnimationEvent::Brightness:
			animation_impl::trigger(context, data.brightness, event.interval);
			break;

		case AnimationEvent::Alpha:
			animation_impl::trigger(context, data.alpha, event.interval);
			break;

		case AnimationEvent::MinSaturation:
			animation_impl::trigger(
				context, data.min_saturation, event.interval);
			break;

		case AnimationEvent::MaxSaturation:
			animation_impl::trigger(
				context, data.max_saturation, event.interval);
			break;

		case AnimationEvent::LightIntensity:
			animation_impl::trigger(
				context, data.light_intensity, event.interval);
			break;
			
		case AnimationEvent::LightRadius:
			animation_impl::trigger(context, data.light_radius, event.interval);
			break;

		case AnimationEvent::Legs:
			animation_impl::trigger(context, data, event.leg_layer, event.legs);
			break;

		case AnimationEvent::Torso:
			animation_impl::trigger(
				context, data, event.torso_layer, event.torso);
			break;
	}
}

void AnimationSystem::update(sf::Time const& elapsed) {
	dispatch<AnimationEvent>(*this);

	for (auto& data : *this) {
		animation_impl::update(context, data, elapsed);
	}

	propagate<AnimationEvent>();
}

// ---------------------------------------------------------------------------

namespace animation_impl {

Context::Context(LogContext& log, AnimationSender& animation_sender,
	MovementManager const & movement_manager, AnimationManager& animation_manager)
	: log{log}
	, animation_sender{animation_sender}
	, movement_manager{movement_manager}
	, animation_manager{animation_manager} {}

// ---------------------------------------------------------------------------

void trigger(Context& context, AnimationData& data, AnimationAction action) {
	ASSERT(data.tpl.torso[SpriteTorsoLayer::Base] != nullptr);
	auto const& layer = *data.tpl.torso[SpriteTorsoLayer::Base];
	if (layer[action].frames.empty()) {
		context.log.error << "[Core/Animation] " << "AnimationData #" << data.id
						  << " has no specified animation for '"
						  << to_string(action) << "'; ignored.\n";
		action = AnimationAction::Idle;
	}
	// reset action state
	data.current = action;
	data.torso = utils::ActionState{};
	data.has_changed = true;

	// propagate new action state
	AnimationEvent event;
	event.actor = data.id;
	event.type = AnimationEvent::Action;
	event.action = action;
	context.animation_sender.send(event);
}

void trigger(Context& context, utils::IntervalState& state,
	utils::IntervalState const& args) {
	// apply state
	state = args;
}

void trigger(Context& context, AnimationData& data, SpriteLegLayer layer,
	AnimationEvent::LegAnimation const* ptr) {
	data.tpl.legs[layer] = ptr;
	if (ptr != nullptr) {
		data.legs.index = std::min(data.legs.index, ptr->frames.size() - 1u);
	} else {
		data.legs.index = 0u;
	}
	data.has_changed = true;
}

void trigger(Context& context, AnimationData& data, SpriteTorsoLayer layer,
	AnimationEvent::TorsoAnimation const* ptr) {
	if (layer == SpriteTorsoLayer::Base) {
		ASSERT(ptr != nullptr);
	}
	data.tpl.torso[layer] = ptr;
	if (ptr != nullptr) {
		data.torso.index = std::min(data.torso.index, (*ptr)[data.current].frames.size() - 1u);
	} else {
		data.torso.index = 0u;
	}
	data.has_changed = true;
}

void update(Context& context, AnimationData& data, sf::Time const& elapsed) {
	ASSERT(data.tpl.torso[SpriteTorsoLayer::Base] != nullptr);
	auto const & move_data = context.movement_manager.query(data.id);

	bool legs_updated{false}, torso_updated{false}, brightness_updated{false},
		alpha_updated{false}, min_saturation_updated{false},
		max_saturation_updated{false}, light_intensity_updated{false},
		light_radius_updated{false};

	// update legs
	if (move_data.move != sf::Vector2f{} && data.tpl.legs[SpriteLegLayer::Base] != nullptr) {
		// assuming leg layers to be synchronous
		auto& layer = *data.tpl.legs[SpriteLegLayer::Base];
		utils::updateActionState(data.legs, layer, elapsed, legs_updated);
	}
	// update torso
	// assuming torso layers to be synchronous
	auto& torso = *data.tpl.torso[SpriteTorsoLayer::Base];
	if (utils::updateActionState(
			data.torso, torso[data.current], elapsed, torso_updated)) {
		onActionFinished(context, data);
	}
	// update intervals
	utils::updateInterval(data.brightness, elapsed, brightness_updated);
	utils::updateInterval(data.alpha, elapsed, alpha_updated);
	utils::updateInterval(data.min_saturation, elapsed, min_saturation_updated);
	utils::updateInterval(data.max_saturation, elapsed, max_saturation_updated);
	utils::updateInterval(data.light_intensity, elapsed, light_intensity_updated);
	utils::updateInterval(data.light_radius, elapsed, light_radius_updated);
	// update dirty flag (or keep it if already true)
	data.has_changed = data.has_changed || legs_updated || torso_updated ||
					   brightness_updated || alpha_updated ||
					   min_saturation_updated || max_saturation_updated ||
					   light_intensity_updated || light_radius_updated;
}

void onActionFinished(Context& context, AnimationData& data) {
	if (data.current == AnimationAction::Die) {
		// stop everything and stay at last dying frame
		data.torso.index =
			(*data.tpl.torso[SpriteTorsoLayer::Base])[AnimationAction::Die]
				.frames.size() -
			1u;
		return;
	}

	// reset to idle
	auto prev = data.current;
	data.current = AnimationAction::Idle;
	data.torso = utils::ActionState{};

	if (prev != AnimationAction::Idle) {
		// propagate new state
		AnimationEvent event;
		event.actor = data.id;
		event.type = AnimationEvent::Action;
		event.action = AnimationAction::Idle;
		context.animation_sender.send(event);
	}
}

}  // ::animation_impl

// ---------------------------------------------------------------------------

sf::Time getDuration(AnimationData const& data, AnimationAction action) {
	ASSERT(data.tpl.torso[SpriteTorsoLayer::Base] != nullptr);
	return (*data.tpl.torso[SpriteTorsoLayer::Base])[action].duration;
}

}  // ::core
