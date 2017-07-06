#pragma once
#include <core/event.hpp>
#include <rpg/event.hpp>
#include <game/path.hpp>
#include <game/script.hpp>
#include <engine/event.hpp>

namespace engine {

struct AiSystem
	: utils::EventListener<core::CollisionEvent, core::TeleportEvent,
		core::AnimationEvent, core::MoveEvent, core::FocusEvent,
		rpg::EffectEvent, rpg::StatsEvent, rpg::DeathEvent,
		rpg::SpawnEvent, rpg::FeedbackEvent> {

	game::ScriptSystem script;
	game::PathSystem path;
	game::NavigationSystem navigation;

	AiSystem(core::LogContext& log, std::size_t max_objects);
	~AiSystem();
	
	void connect(MultiEventListener& listener);
	void disconnect(MultiEventListener& listener);
	
	void handle(core::CollisionEvent const& event);
	void handle(core::TeleportEvent const& event);
	void handle(core::AnimationEvent const& event);
	void handle(core::MoveEvent const& event);
	void handle(core::FocusEvent const& event);
	void handle(rpg::EffectEvent const& event);
	void handle(rpg::StatsEvent const& event);
	void handle(rpg::DeathEvent const& event);
	void handle(rpg::SpawnEvent const& event);
	void handle(rpg::FeedbackEvent const& event);

	sf::Time update(sf::Time const& elapsed);
	
	void clear();
};

}  // ::state
