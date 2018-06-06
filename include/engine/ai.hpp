#pragma once
#include <core/event.hpp>
#include <rpg/event.hpp>
#include <game/path.hpp>
#include <game/script.hpp>
#include <game/tracer.hpp>
#include <engine/event.hpp>

namespace engine {

struct AiSystem
	: utils::EventListener<core::CollisionEvent, core::TeleportEvent,
		core::AnimationEvent, core::MoveEvent,
		rpg::EffectEvent, rpg::StatsEvent, rpg::DeathEvent,
		rpg::SpawnEvent, rpg::FeedbackEvent> {

	core::LogContext& log;
	
	game::ScriptSystem script;
	game::PathSystem path;
	game::NavigationSystem navigation;
	game::TracerSystem tracer; /// @note not only for AI-based entities

	AiSystem(core::LogContext& log, std::size_t max_objects, core::MovementManager const & movement);
	~AiSystem();
	
	void connect(MultiEventListener& listener);
	void disconnect(MultiEventListener& listener);
	
	template <typename T>
	void bind(utils::SingleEventListener<T>& listener);
	
	template <typename T>
	void unbind(utils::SingleEventListener<T> const & listener);

	void handle(core::CollisionEvent const& event);
	void handle(core::TeleportEvent const& event);
	void handle(core::AnimationEvent const& event);
	void handle(core::MoveEvent const& event);
	void handle(rpg::EffectEvent const& event);
	void handle(rpg::StatsEvent const& event);
	void handle(rpg::DeathEvent const& event);
	void handle(rpg::SpawnEvent const& event);
	void handle(rpg::FeedbackEvent const& event);

	sf::Time update(sf::Time const& elapsed);
	
	void clear();
};

}  // ::state
