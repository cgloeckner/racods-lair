#pragma once
#include <core/dungeon.hpp>
#include <core/entity.hpp>
#include <core/event.hpp>
#include <rpg/input.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>
#include <rpg/action.hpp>
#include <rpg/delay.hpp>
#include <rpg/interact.hpp>
#include <engine/event.hpp>

namespace engine {

struct BehaviorSystem
	: utils::EventListener<core::AnimationEvent, core::MoveEvent,
		  core::CollisionEvent, rpg::ActionEvent, rpg::DeathEvent,
		  rpg::SpawnEvent, rpg::PerkEvent, rpg::FeedbackEvent> {

	rpg::InputSystem input;
	rpg::ActionSystem action;
	rpg::InteractSystem interact;
	rpg::DelaySystem delay;

	BehaviorSystem(core::LogContext& log, std::size_t max_objects, core::DungeonSystem const& dungeon,
		core::MovementManager const& movement, core::FocusManager const& focus,
		core::AnimationManager const& animation, rpg::ItemManager const& item,
		rpg::StatsManager const& stats, rpg::PlayerManager const& player);
	
	void connect(MultiEventListener& listener);
	void disconnect(MultiEventListener& listener);
	
	template <typename T>
	void bind(utils::SingleEventListener<T>& listener);
	
	template <typename T>
	void unbind(utils::SingleEventListener<T> const & listener);

	void handle(sf::Event const& event);
	void handle(core::AnimationEvent const& event);
	void handle(core::MoveEvent const& event);
	void handle(core::CollisionEvent const& event);
	void handle(rpg::ActionEvent const& event);
	void handle(rpg::DeathEvent const& event);
	void handle(rpg::SpawnEvent const& event);
	void handle(rpg::PerkEvent const& event);
	void handle(rpg::FeedbackEvent const& event);

	sf::Time update(sf::Time const& elapsed);
	
	void clear();
};

/*
BehaviorSystems : rage/behavior
	IN:
		Input -- Anfrage von Spieler oder KI
		Action -- Anfrage von Spieler oder KI
		Animation -- Action-Notify
		Move -- Movement-Nofity
		Collision -- Collision-Notify
		Death -- Death-Notify
		Perk -- Anfrage zur Perk-Verwendung (durch Quickslot oder KI)
	OUT:
		Input -- auszuführende Bewegung (durch Spieler, KI oder Barrier)
		Animation -- abzuspielende FrameAnimation (Angriff etc.)
		Combat -- durchzuführender Kampf (zB Angriff)
		Projectile -- zu erzeugendes Projektil (zb Bogenkampf)
		Item -- gelootetes Zeug

	ctor(PhysicsSystem const & physics, Animation const & animation, Item const
& item, core::DungeonSystem const & dungeon)
	sf::Time update(sf::Time const & elapsed)

	rpg::ActionSystem action;
	rpg::DelaySystem delay;
	rpg::InteractSystem interact;

*/

}  // ::state
