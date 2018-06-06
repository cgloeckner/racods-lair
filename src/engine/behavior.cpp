#include <engine/behavior.hpp>

namespace engine {

BehaviorSystem::BehaviorSystem(core::LogContext& log, std::size_t max_objects,
	core::DungeonSystem const& dungeon, core::MovementManager const& movement,
	core::CollisionManager const & collision, core::FocusManager const& focus,
	core::AnimationManager const& animation, rpg::ItemManager const& item,
	rpg::StatsManager const& stats, rpg::PlayerManager const& player)
	: utils::EventListener<core::AnimationEvent, core::MoveEvent,
		  core::CollisionEvent, rpg::ActionEvent, rpg::DeathEvent,
		  rpg::SpawnEvent, rpg::PerkEvent, rpg::FeedbackEvent>{}
	, input{log, max_objects, dungeon, movement, collision, focus}
	, action{log, max_objects}
	, interact{log, max_objects, movement, focus, player}
	, delay{log, dungeon, movement, focus, animation, item, stats, interact,
		  player} {
	// internal events
	input.bind<core::InputEvent>(action);
	input.bind<rpg::ActionEvent>(action);
	action.bind<rpg::ActionEvent>(delay);
	delay.bind<rpg::InteractEvent>(interact);
}

void BehaviorSystem::connect(MultiEventListener& listener) {
	action.bind<rpg::ActionEvent>(listener);
	delay.bind<rpg::InteractEvent>(listener);
}

void BehaviorSystem::disconnect(MultiEventListener& listener) {
	action.unbind<rpg::ActionEvent>(listener);
	delay.unbind<rpg::InteractEvent>(listener);
}

// ---------------------------------------------------------------------------

template <>
void BehaviorSystem::bind(
	utils::SingleEventListener<core::InputEvent>& listener) {
	// to physics
	action.bind(listener);
	interact.bind(listener);
}

template <>
void BehaviorSystem::bind(
	utils::SingleEventListener<core::AnimationEvent>& listener) {
	// to animation
	action.bind(listener);
	delay.bind(listener);
}

template <>
void BehaviorSystem::bind(
	utils::SingleEventListener<rpg::ActionEvent>& listener) {
	// to avatar and game state (handling pause)
	action.bind(listener);
}

template <>
void BehaviorSystem::bind(
	utils::SingleEventListener<rpg::CombatEvent>& listener) {
	// to combat
	delay.bind(listener);
}

template <>
void BehaviorSystem::bind(
	utils::SingleEventListener<rpg::ProjectileEvent>& listener) {
	// to factory
	delay.bind(listener);
}

template <>
void BehaviorSystem::bind(
	utils::SingleEventListener<rpg::ItemEvent>& listener) {
	// to avatar
	interact.bind(listener);
}

// ---------------------------------------------------------------------------

template <>
void BehaviorSystem::unbind(
	utils::SingleEventListener<core::InputEvent> const & listener) {
	// to physics
	action.unbind(listener);
	interact.unbind(listener);
}

template <>
void BehaviorSystem::unbind(
	utils::SingleEventListener<core::AnimationEvent> const & listener) {
	// to animation
	action.unbind(listener);
	delay.unbind(listener);
}

template <>
void BehaviorSystem::unbind(
	utils::SingleEventListener<rpg::ActionEvent> const & listener) {
	// to avatar and game state (handling pause)
	action.unbind(listener);
}

template <>
void BehaviorSystem::unbind(
	utils::SingleEventListener<rpg::CombatEvent> const & listener) {
	// to combat
	delay.unbind(listener);
}

template <>
void BehaviorSystem::unbind(
	utils::SingleEventListener<rpg::ProjectileEvent> const & listener) {
	// to factory
	delay.unbind(listener);
}

template <>
void BehaviorSystem::unbind(
	utils::SingleEventListener<rpg::ItemEvent> const & listener) {
	// to avatar
	interact.unbind(listener);
}

// ---------------------------------------------------------------------------

void BehaviorSystem::handle(sf::Event const& event) {
	// from sfml
	input.handle(event);
}

void BehaviorSystem::handle(core::AnimationEvent const& event) {
	// from animation
	action.receive(event);
}

void BehaviorSystem::handle(core::MoveEvent const& event) {
	// from physics
	interact.receive(event);
}

void BehaviorSystem::handle(core::CollisionEvent const& event) {
	// from physics
	interact.receive(event);
}

void BehaviorSystem::handle(rpg::ActionEvent const& event) {
	// from ai
	action.receive(event);
}

void BehaviorSystem::handle(rpg::DeathEvent const& event) {
	// from avatar
	input.receive(event);
	action.receive(event);
}

void BehaviorSystem::handle(rpg::SpawnEvent const& event) {
	// from combat
	input.receive(event);
	action.receive(event);
}

void BehaviorSystem::handle(rpg::PerkEvent const& event) {
	// from avatar
	delay.receive(event);
}

void BehaviorSystem::handle(rpg::FeedbackEvent const & event) {
	action.receive(event);
}

// ---------------------------------------------------------------------------

sf::Time BehaviorSystem::update(sf::Time const& elapsed) {
	sf::Clock clock;

	dispatch<core::AnimationEvent>(*this);
	dispatch<core::MoveEvent>(*this);
	dispatch<core::CollisionEvent>(*this);
	dispatch<rpg::ActionEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::PerkEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<rpg::FeedbackEvent>(*this);

	input.update(elapsed);
	action.update(elapsed);
	delay.update(elapsed);
	interact.update(elapsed);

	return clock.restart();
}

void BehaviorSystem::clear() {
	
}

}  // ::state
