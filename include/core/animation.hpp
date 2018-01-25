#pragma once
#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>
#include <core/entity.hpp>

namespace core {

namespace animation_impl {

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	AnimationSender& animation_sender;
	MovementManager const & movement_manager;
	AnimationManager& animation_manager;

	Context(LogContext& log, AnimationSender& animation_sender,
		MovementManager const & movement_manager, AnimationManager& animation_manager);
};

}  // ::animation_impl

// ---------------------------------------------------------------------------
// Animation System

/// AnimationSystem handles sprite animations
/**
 *	Each animation component holds a non-owning reference to the actual
 *	animation template data. Those data specify the animation itself, the
 *	component specifies the actual state within the animation definition.
 *	Furthermore, each component has two distinct kinds of animations:
 *	Animation actions (that can e.g. be displayed by changing the player torso)
 *	and Movement animations (that could be displayed by changing the player's
 *	feets). Both kinds of animations are handled seperately but within the
 *	same call.
 *	An animation (despite its actual kind) is triggered by instances of
 *	AnimationEvent, that could be created by an avatar system. Those events
 *	are handled here. When setting a new animation, it is applied immediately.
 *	There is no delay such as at the movement system or elsewhere.
 *	Once an animation was finished, the successor animation state is up to
 *	the kind of animation. All move animations are repeated until they are
 *	changed by an AnimationEvent. All actio-based animations (including idle)
 *	have idle as their successor, which is played afterwards.
 *	There is no notification if an animation stopped or finished a loop,
 *	because this system is very graphics-related. In order to let the render
 *	system know about whether the animation processed since last frame is
 *	done by a dirty flag. This flag is set by the animation system after an
 *	animation state was changed. If not, the flag isn't set at all. It is reset
 *	by the render system after it processed the new animation state (e.g.
 *	by changing the rectangle to the used subtexture).
 *	Additionally, each object's brightness and saturation can also be animated.
 *	AnimationEvents about actions and movement are forwarded one an animation
 *	stopped or was changed.
 */
class AnimationSystem
	// Event API
	: public utils::EventListener<AnimationEvent>,
	  public utils::EventSender<AnimationEvent>
	  // Component API
	  ,
	  public AnimationManager {

  protected:
	animation_impl::Context context;

  public:
	AnimationSystem(LogContext& log, std::size_t max_objects, MovementManager const & movement_manager);

	void handle(AnimationEvent const& event);

	void update(sf::Time const& elapsed);
};

namespace animation_impl {

// ---------------------------------------------------------------------------
// Internal Animation API

/// This triggers a new action-based animation
/**
 *	This function is used to trigger an action-based animation. The new
 *	animation is set directly without any delay.
 *
 *	@param context Animation context to work with
 *	@param data Component data to trigger for
 *	@param action Animation action to trigger
 */
void trigger(Context& context, AnimationData& data, AnimationAction action);

/// This triggers a new interval animation
/**
 *	This function is used to trigger a interval animation (including
 *	stopping it).
 *
 *	@param context Animation context to work with
 *	@param data Component data to trigger for
 *	@param interval Arguments to specify the behavior
 */
void trigger(Context& context, utils::IntervalState& state,
	utils::IntervalState const& args);

/// This triggers changing the leg layer animation
/**
 *	This will apply the given legs animation to the specified leg layer.
 *
 *	@param context Animation context to work with
 *	@param data Component data to trigger for
 *	@param layer SpriteLegLayer to update
 *	@param Ptr Pointer to leg animation to apply
 */
void trigger(Context& context, AnimationData& data, SpriteLegLayer layer,
	AnimationEvent::LegAnimation const* ptr);

/// This triggers changing the torso layer animation
/**
 *	This will apply the given torso animation to the specified torso layer.
 *	The torso base's animation cannot be changed to nullptr.
 *
 *	@param context Animation context to work with
 *	@param data Component data to trigger for
 *	@param layer SpriteTorsoLayer to update
 *	@param ptr Pointer to torso animation to apply
 */
void trigger(Context& context, AnimationData& data, SpriteTorsoLayer layer,
	AnimationEvent::TorsoAnimation const* ptr);

/// Triggers the entire animation handling
/**
 *	This function triggers the entire animation handling process. The current
 *	action-based animation (including idle) is always processed. Additionally,
 *	if the object is moving, the move animation is also processed. Once any of
 *	those processings changed the actual state (in the terms of which
 *	rectangle should be used), the component's dirty flag is set.
 *
 *	@pre data.tpl.torsoBase != nullptr
 *	@param context Animation context to work with
 *	@param data Component data to update
 *	@param elapsed Duration to use for processing the animation(s)
 */
void update(Context& context, AnimationData& data, sf::Time const& elapsed);

/// Trigger new action animation if the previous was finished
/**
 *	This will trigger an idle animation if the previous animation was neither
 *	idle nor die. Otherwise nothing is done. If the animation is changed,
 *	an animation event is propagated.
 *
 *	@param context Animation context to work with
 *	@param data Component data to update
 */
void onActionFinished(Context& context, AnimationData& data);

}  // ::animation_impl

// ---------------------------------------------------------------------------
// Public Animation API

/// This returns the duration of the given animation action
/**
 *	Because all non-leg-based animations run synchronously, the duration is
 *	queried by the torsoBase part of the animation's template.
 *	If no torsoBase is assigned, the duration cannot be calculated.
 *
 *	@pre data.torsoBase[action] != nullptr
 *	@param data Component data to query at
 *	@param action Animation action to query for
 *	@return total duration of the animation
 */
sf::Time getDuration(AnimationData const& data, AnimationAction action);

}  // ::core
