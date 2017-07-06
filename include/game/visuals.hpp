#pragma once
#include <core/event.hpp>
#include <rpg/event.hpp>

#include <game/common.hpp>

namespace game {

namespace visuals_impl {

extern float const BRIGHTNESS_ON_DEATH;

/// Visualization context
struct Context {
	core::LogContext& log;
	core::RenderManager const & render_manager;
	core::AnimationSender& animation_sender;

	Context(core::LogContext& log, core::RenderManager const & render_manager,
		core::AnimationSender& animation_sender);
};

// ---------------------------------------------------------------------------

/// Trigger damage visualization
/**
 *	Let the damage target blink.
 *
 *	@param context Reference to visualization context
 *	@param event StatsEvent which contains damage information
 */
void onDamaged(Context& context, rpg::StatsEvent const& event);

/// Trigger death visualization
/**
 *	Let dying object's light intensity fade out.
 *
 *	@param context Reference to visualization context
 *	@param event DeathEvent which contains death information
 */
void onKilled(Context& context, rpg::DeathEvent const& event);

/// Trigger (re)spawn visualization
/**
 *	Let object's light intensity fade in.
 *
 *	@param context Reference to visualization context
 *	@param event SpawnEvent which contains (re)spawn information
 */
void onSpawn(Context& context, rpg::SpawnEvent const& event);

/// Trigger explosion visualization
/**
 *	Let exploding object's alpha fade out.
 *
 *	@param context Reference to visualization context
 *	@param id Actor's object ID
 */
void onExploded(Context& context, core::ObjectID id);

}  // ::visuals_impl

// ---------------------------------------------------------------------------

/// Triggers visualization on several events
class VisualsSystem
	// Event API
	: public utils::EventListener<rpg::StatsEvent, rpg::DeathEvent,
		  rpg::SpawnEvent, rpg::ProjectileEvent>,
	  public utils::EventSender<core::AnimationEvent> {

  private:
	visuals_impl::Context context;

  public:
	VisualsSystem(core::LogContext& log, core::RenderManager const & render_manager);

	void handle(rpg::StatsEvent const& event);
	void handle(rpg::DeathEvent const& event);
	void handle(rpg::SpawnEvent const& event);
	void handle(rpg::ProjectileEvent const& event);

	sf::Time update(sf::Time const& elapsed);
};

}  // ::rage
