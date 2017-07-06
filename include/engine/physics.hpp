#pragma once
#include <core/dungeon.hpp>
#include <core/entity.hpp>
#include <core/event.hpp>
#include <core/collision.hpp>
#include <core/movement.hpp>
#include <core/focus.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>
#include <rpg/projectile.hpp>
#include <engine/event.hpp>

namespace engine {

struct PhysicsSystem : utils::EventListener<core::InputEvent> {

	core::MovementSystem movement;
	core::CollisionSystem collision;
	core::FocusSystem focus;
	rpg::ProjectileSystem projectile;

	PhysicsSystem(core::LogContext& log, std::size_t max_objects, core::DungeonSystem& dungeon);
	
	void connect(MultiEventListener& listener);
	void disconnect(MultiEventListener& listener);

	template <typename T>
	void bind(utils::SingleEventListener<T>& listener);
	
	template <typename T>
	void unbind(utils::SingleEventListener<T> const & listener);

	void handle(core::InputEvent const& event);

	sf::Time update(sf::Time const& elapsed);
	
	void clear();
};

}  // ::state
