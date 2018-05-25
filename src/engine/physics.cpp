#include <engine/physics.hpp>

namespace engine {

PhysicsSystem::PhysicsSystem(core::LogContext& log, std::size_t max_objects,
	core::DungeonSystem& dungeon)
	: utils::EventListener<core::InputEvent>{}
	, movement{log, max_objects, dungeon}
	, collision{log, max_objects, dungeon, movement}
	, focus{log, max_objects}
	, projectile{log, max_objects, movement, collision, dungeon} {
	// internal events
	collision.bind<core::CollisionEvent>(movement);
	collision.bind<core::CollisionEvent>(projectile);
}

void PhysicsSystem::connect(MultiEventListener& listener) {
	collision.bind<core::CollisionEvent>(listener);
}

void PhysicsSystem::disconnect(MultiEventListener& listener) {
	collision.unbind<core::CollisionEvent>(listener);
}

// ---------------------------------------------------------------------------

template <>
void PhysicsSystem::bind(
	utils::SingleEventListener<core::MoveEvent>& listener) {
	// to various
	movement.bind(listener);
}

template <>
void PhysicsSystem::bind(
	utils::SingleEventListener<core::CollisionEvent>& listener) {
	// to various
	collision.bind(listener);
}

template <>
void PhysicsSystem::bind(
	utils::SingleEventListener<core::TeleportEvent>& listener) {
	// to various
	collision.bind(listener);
}

template <>
void PhysicsSystem::bind(
	utils::SingleEventListener<rpg::CombatEvent>& listener) {
	// to combat
	projectile.bind(listener);
}

template <>
void PhysicsSystem::bind(
	utils::SingleEventListener<rpg::ProjectileEvent>& listener) {
	// to factory
	projectile.bind(listener);
}

// ---------------------------------------------------------------------------

template <>
void PhysicsSystem::unbind(
	utils::SingleEventListener<core::MoveEvent> const & listener) {
	// to various
	movement.unbind(listener);
}

template <>
void PhysicsSystem::unbind(
	utils::SingleEventListener<core::CollisionEvent> const & listener) {
	// to various
	collision.unbind(listener);
}

template <>
void PhysicsSystem::unbind(
	utils::SingleEventListener<core::TeleportEvent> const & listener) {
	// to various
	collision.unbind(listener);
}

template <>
void PhysicsSystem::unbind(
	utils::SingleEventListener<rpg::CombatEvent> const & listener) {
	// to combat
	projectile.unbind(listener);
}

template <>
void PhysicsSystem::unbind(
	utils::SingleEventListener<rpg::ProjectileEvent> const & listener) {
	// to factory
	projectile.unbind(listener);
}

// ---------------------------------------------------------------------------

void PhysicsSystem::handle(core::InputEvent const& event) {
	// from behavior
	movement.receive(event);
}

// ---------------------------------------------------------------------------

sf::Time PhysicsSystem::update(sf::Time const& elapsed) {
	sf::Clock clock;

	dispatch<core::InputEvent>(*this);

	movement.update(elapsed);
	collision.update(elapsed);
	focus.update(elapsed);
	projectile.update(elapsed);

	return clock.restart();
}

void PhysicsSystem::clear() {
	
}

}  // ::state
