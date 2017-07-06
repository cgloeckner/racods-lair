#include <engine/ai.hpp>

namespace engine {

AiSystem::AiSystem(core::LogContext& log, std::size_t max_objects)
	: utils::EventListener<core::CollisionEvent, core::TeleportEvent,
		core::AnimationEvent, core::MoveEvent, core::FocusEvent,
		rpg::EffectEvent, rpg::StatsEvent, rpg::DeathEvent,
		rpg::SpawnEvent, rpg::FeedbackEvent>{}
	, script{log, max_objects}
	, path{log}
	, navigation{} {
	// path.start();
}

AiSystem::~AiSystem() {
	// path.stop();
}

void AiSystem::connect(MultiEventListener& listener) {
}

void AiSystem::disconnect(MultiEventListener& listener) {
}

// ---------------------------------------------------------------------------

void AiSystem::handle(core::CollisionEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(core::TeleportEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(core::AnimationEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(core::MoveEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(core::FocusEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(rpg::EffectEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(rpg::StatsEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(rpg::DeathEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(rpg::SpawnEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(rpg::FeedbackEvent const& event) {
	script.receive(event);
}

// ---------------------------------------------------------------------------

sf::Time AiSystem::update(sf::Time const& elapsed) {
	sf::Clock clock;

	dispatch<core::CollisionEvent>(*this);
	dispatch<core::TeleportEvent>(*this);
	dispatch<core::AnimationEvent>(*this);
	dispatch<core::MoveEvent>(*this);
	dispatch<core::FocusEvent>(*this);
	dispatch<rpg::EffectEvent>(*this);
	dispatch<rpg::StatsEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<rpg::FeedbackEvent>(*this);

	script.update(elapsed);

	return clock.restart();
}

void AiSystem::clear() {
	navigation.clear();
}

}  // ::state
