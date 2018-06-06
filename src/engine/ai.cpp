#include <engine/ai.hpp>

namespace engine {

AiSystem::AiSystem(core::LogContext& log, std::size_t max_objects, core::MovementManager const & movement)
	: utils::EventListener<core::CollisionEvent, core::TeleportEvent,
		core::AnimationEvent, core::MoveEvent,
		rpg::EffectEvent, rpg::StatsEvent, rpg::DeathEvent,
		rpg::SpawnEvent, rpg::FeedbackEvent>{}
	, log{log}
	, script{log, max_objects}
	, path{log}
	, navigation{}
	, tracer{log, max_objects, movement, path} {
	// path.start();
}

AiSystem::~AiSystem() {
	// path.stop();
}

void AiSystem::connect(MultiEventListener& listener) {
	tracer.bind<core::InputEvent>(listener);
}

void AiSystem::disconnect(MultiEventListener& listener) {
	tracer.unbind<core::InputEvent>(listener);
}

// ---------------------------------------------------------------------------

template <>
void AiSystem::bind(
	utils::SingleEventListener<core::InputEvent>& listener) {
	// to action
	tracer.bind<core::InputEvent>(listener);
}

// ---------------------------------------------------------------------------

template <>
void AiSystem::unbind(
	utils::SingleEventListener<core::InputEvent> const & listener) {
	// to action
	tracer.unbind<core::InputEvent>(listener);
}

// ---------------------------------------------------------------------------

void AiSystem::handle(core::CollisionEvent const& event) {
	script.receive(event);
	tracer.receive(event);
}

void AiSystem::handle(core::TeleportEvent const& event) {
	script.receive(event);
	tracer.receive(event);
}

void AiSystem::handle(core::AnimationEvent const& event) {
	script.receive(event);
}

void AiSystem::handle(core::MoveEvent const& event) {
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
	tracer.receive(event);
}

void AiSystem::handle(rpg::SpawnEvent const& event) {
	script.receive(event);
	tracer.receive(event);
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
	dispatch<rpg::EffectEvent>(*this);
	dispatch<rpg::StatsEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<rpg::FeedbackEvent>(*this);

	script.update(elapsed);
	path.calculate(elapsed);
	tracer.update(elapsed);

	return clock.restart();
}

void AiSystem::clear() {
	navigation.clear();
}

}  // ::state
