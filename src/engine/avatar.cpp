#include <engine/avatar.hpp>

namespace engine {

AvatarSystem::AvatarSystem(core::LogContext& log, std::size_t max_objects)
	: utils::EventListener<rpg::ItemEvent, rpg::PerkEvent, rpg::TrainingEvent,
		  rpg::StatsEvent, rpg::QuickslotEvent, rpg::ActionEvent, rpg::ExpEvent,
		  rpg::EffectEvent, rpg::SpawnEvent>{}
	, stats{log, max_objects}
	, effect{log, max_objects}
	, item{log, max_objects, stats}
	, perk{log, max_objects, stats}
	, quickslot{log, max_objects}
	, player{log, max_objects, stats} {
	// internal events
	stats.bind<rpg::DeathEvent>(effect);
	item.bind<rpg::BoniEvent>(stats);
	item.bind<rpg::StatsEvent>(stats);
	item.bind<rpg::QuickslotEvent>(quickslot);
	effect.bind<rpg::BoniEvent>(stats);
	player.bind<rpg::TrainingEvent>(stats);
	player.bind<rpg::TrainingEvent>(perk);
	player.bind<rpg::ExpEvent>(stats);
	quickslot.bind<rpg::ItemEvent>(item);
	quickslot.bind<rpg::PerkEvent>(perk);
	perk.bind<rpg::StatsEvent>(stats);
}

void AvatarSystem::connect(MultiEventListener& listener) {
	item.bind<rpg::BoniEvent>(listener);
	item.bind<rpg::StatsEvent>(listener);
	item.bind<rpg::QuickslotEvent>(listener);
	effect.bind<rpg::BoniEvent>(listener);
	player.bind<rpg::TrainingEvent>(listener);
	quickslot.bind<rpg::ItemEvent>(listener);
	quickslot.bind<rpg::PerkEvent>(listener);
	perk.bind<rpg::StatsEvent>(listener);
}

void AvatarSystem::disconnect(MultiEventListener& listener) {
	item.unbind<rpg::BoniEvent>(listener);
	item.unbind<rpg::StatsEvent>(listener);
	item.unbind<rpg::QuickslotEvent>(listener);
	effect.unbind<rpg::BoniEvent>(listener);
	player.unbind<rpg::TrainingEvent>(listener);
	player.unbind<rpg::ExpEvent>(listener);
	quickslot.unbind<rpg::ItemEvent>(listener);
	quickslot.unbind<rpg::PerkEvent>(listener);
	perk.unbind<rpg::StatsEvent>(listener);
}

// ---------------------------------------------------------------------------

template <>
void AvatarSystem::unbind(utils::SingleEventListener<core::AnimationEvent> const & listener) {
	// to animation
	item.unbind(listener);
	perk.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<core::SpriteEvent> const & listener) {
	// to render
	item.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<rpg::StatsEvent> const & listener) {
	// to ai, hud
	stats.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<rpg::ExpEvent> const & listener) {
	// to hud
	player.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<rpg::FeedbackEvent> const & listener) {
	// to hud
	player.unbind(listener);
	quickslot.unbind(listener);
	item.unbind(listener);
	perk.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<rpg::DeathEvent> const & listener) {
	// to hud
	stats.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<rpg::EffectEvent> const & listener) {
	// to hud, ai
	effect.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<rpg::CombatEvent> const & listener) {
	// to combat
	effect.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<rpg::PerkEvent> const & listener) {
	// to ui
	perk.unbind(listener);
}

template <>
void AvatarSystem::unbind(utils::SingleEventListener<rpg::ItemEvent> const & listener) {
	// to ui
	item.unbind(listener);
}

// ---------------------------------------------------------------------------

template <>
void AvatarSystem::bind(
	utils::SingleEventListener<core::AnimationEvent>& listener) {
	// to animation
	item.bind(listener);
	perk.bind(listener);
}

template <>
void AvatarSystem::bind(
	utils::SingleEventListener<core::SpriteEvent>& listener) {
	// to render
	item.bind(listener);
}

template <>
void AvatarSystem::bind(utils::SingleEventListener<rpg::StatsEvent>& listener) {
	// to ai, hud
	stats.bind(listener);
}

template <>
void AvatarSystem::bind(utils::SingleEventListener<rpg::ExpEvent>& listener) {
	// to hud
	player.bind(listener);
}

template <>
void AvatarSystem::bind(
	utils::SingleEventListener<rpg::FeedbackEvent>& listener) {
	// to hud
	player.bind(listener);
	quickslot.bind(listener);
	item.bind(listener);
	perk.bind(listener);
}

template <>
void AvatarSystem::bind(utils::SingleEventListener<rpg::DeathEvent>& listener) {
	// to hud
	stats.bind(listener);
}

template <>
void AvatarSystem::bind(
	utils::SingleEventListener<rpg::EffectEvent>& listener) {
	// to hud, ai
	effect.bind(listener);
}

template <>
void AvatarSystem::bind(
	utils::SingleEventListener<rpg::CombatEvent>& listener) {
	// to combat
	effect.bind(listener);
}

template <>
void AvatarSystem::bind(utils::SingleEventListener<rpg::PerkEvent>& listener) {
	// to ui
	perk.bind(listener);
}

template <>
void AvatarSystem::bind(utils::SingleEventListener<rpg::ItemEvent>& listener) {
	// to ui
	item.bind(listener);
}

// ---------------------------------------------------------------------------

void AvatarSystem::handle(rpg::ItemEvent const& event) {
	// from behavior, ui
	item.receive(event);
}

void AvatarSystem::handle(rpg::PerkEvent const& event) {
	// from behavior
	perk.receive(event);
}

void AvatarSystem::handle(rpg::TrainingEvent const& event) {
	// from input
	player.receive(event);
}

void AvatarSystem::handle(rpg::StatsEvent const& event) {
	// from combat
	stats.receive(event);
}

void AvatarSystem::handle(rpg::QuickslotEvent const& event) {
	// from ui
	quickslot.receive(event);
}

void AvatarSystem::handle(rpg::ActionEvent const& event) {
	// from behavior
	quickslot.receive(event);
}

void AvatarSystem::handle(rpg::ExpEvent const& event) {
	// from combat
	player.receive(event);
}

void AvatarSystem::handle(rpg::EffectEvent const& event) {
	// from combat
	effect.receive(event);
}

void AvatarSystem::handle(rpg::SpawnEvent const& event) {
	// not used yet
}

// ---------------------------------------------------------------------------

sf::Time AvatarSystem::update(sf::Time const& elapsed) {
	sf::Clock clock;

	dispatch<rpg::ItemEvent>(*this);
	dispatch<rpg::PerkEvent>(*this);
	dispatch<rpg::TrainingEvent>(*this);
	dispatch<rpg::StatsEvent>(*this);
	dispatch<rpg::QuickslotEvent>(*this);
	dispatch<rpg::ActionEvent>(*this);
	dispatch<rpg::ExpEvent>(*this);
	dispatch<rpg::EffectEvent>(*this);

	player.update(elapsed);
	stats.update(elapsed);
	effect.update(elapsed);
	item.update(elapsed);
	perk.update(elapsed);
	quickslot.update(elapsed);

	return clock.restart();
}

void AvatarSystem::clear() {
	
}

}  // ::state
