#include <engine/ui.hpp>

namespace engine {

UiSystem::UiSystem(core::LogContext& log, std::size_t max_objects, sf::Vector2u const& screen_size,
	sf::Texture const& lightmap, float zoom, unsigned int audio_poolsize,
	core::MovementManager const& movement, core::FocusManager const & focus,
	core::DungeonSystem& dungeon, rpg::StatsManager const& stats,
	rpg::ItemManager const & item, rpg::PlayerManager const & player,
	game::Localization& locale, std::string const & music_base, std::string const & music_ext)
	: utils::EventListener<core::AnimationEvent, core::TeleportEvent,
		core::FocusEvent, core::SpriteEvent, core::SoundEvent, core::MusicEvent,
		core::MoveEvent, rpg::StatsEvent, rpg::DeathEvent, rpg::SpawnEvent,
		rpg::ProjectileEvent, rpg::ExpEvent, rpg::FeedbackEvent,
		rpg::ItemEvent, rpg::PerkEvent, rpg::CombatEvent, rpg::ActionEvent,
		game::PowerupEvent>{}
	, sf::Drawable{}
	, lighting{screen_size, lightmap}
	, camera{screen_size, zoom}
	, animation{log, max_objects, movement}
	, render{log, max_objects, animation, movement, focus, dungeon, camera, lighting}
	, sound{log, audio_poolsize}
	, music{log, music_base, music_ext}
	, audio{log, max_objects, item, player}
	, autocam{log, movement, dungeon, camera}
	, hud{log, max_objects, camera, movement, focus, dungeon, stats, player, locale}
	, visuals{log, render}
	, used_autocam{false} {
	// internal events
	visuals.bind<core::AnimationEvent>(animation);
	audio.bind<core::SoundEvent>(sound);
	audio.bind<core::MusicEvent>(music); // trigger about music playback
	music.bind<core::MusicEvent>(audio); // notify about music stopped
}

void UiSystem::connect(MultiEventListener& listener) {
	visuals.bind<core::AnimationEvent>(listener);
}

void UiSystem::disconnect(MultiEventListener& listener) {
	visuals.unbind<core::AnimationEvent>(listener);
}

void UiSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	render.cull();
	target.draw(render, states);
	target.draw(hud, states);
}

void UiSystem::updateHuds() {
	// realign all ui elements
	for (auto& cam : camera) {
		auto i = 0u;
		for (auto id : cam->objects) {
			if (!hud.has(id)) {
				continue;
			}
			auto& data = hud.query(id);
			ASSERT(data.hud != nullptr);
			data.hud->resize(sf::Vector2u{cam->hud.getSize()}, i++);
		}
	}
}

// ---------------------------------------------------------------------------

template <>
void UiSystem::bind(
	utils::SingleEventListener<core::AnimationEvent>& listener) {
	// to behavior
	animation.bind(listener);
}

template <>
void UiSystem::bind(
	utils::SingleEventListener<core::SoundEvent>& listener) {
	audio.bind(listener);
}

// ---------------------------------------------------------------------------

template <>
void UiSystem::unbind(
	utils::SingleEventListener<core::AnimationEvent> const & listener) {
	// to behavior
	animation.unbind(listener);
}

template <>
void UiSystem::unbind(
	utils::SingleEventListener<core::SoundEvent> const & listener) {
	audio.unbind(listener);
}

// ---------------------------------------------------------------------------

void UiSystem::handle(sf::Event const& event) {
	if (event.type == sf::Event::Resized) {
		sf::Vector2u size{event.size.width, event.size.height};
		camera.resize(size);
		lighting.resize(size);
		
		updateHuds();
	}
}

void UiSystem::handle(core::AnimationEvent const& event) {
	animation.receive(event);
}

void UiSystem::handle(core::TeleportEvent const& event) {
	hud.receive(event);
	
	// update autocam if necessary
	if (!used_autocam && autocam.onTeleport(event.actor)) {
		updateHuds();
	}
}

void UiSystem::handle(core::FocusEvent const& event) {
	hud.receive(event);
}

void UiSystem::handle(core::SpriteEvent const& event) {
	render.receive(event);
}

void UiSystem::handle(core::SoundEvent const& event) {
	sound.receive(event);
}

void UiSystem::handle(core::MusicEvent const& event) {
	music.receive(event);
}

void UiSystem::handle(core::MoveEvent const& event) {
	audio.receive(event);
}

void UiSystem::handle(rpg::StatsEvent const& event) {
	hud.receive(event);
	visuals.receive(event);
	audio.receive(event);
}

void UiSystem::handle(rpg::DeathEvent const& event) {
	hud.receive(event);
	visuals.receive(event);
	audio.receive(event);
}

void UiSystem::handle(rpg::SpawnEvent const& event) {
	hud.receive(event);
	visuals.receive(event);
	audio.receive(event);
}

void UiSystem::handle(rpg::ProjectileEvent const& event) {
	visuals.receive(event);
	audio.receive(event);
}

void UiSystem::handle(rpg::ExpEvent const& event) {
	hud.receive(event);
	audio.receive(event);
}

void UiSystem::handle(rpg::FeedbackEvent const& event) {
	hud.receive(event);
	audio.receive(event);
}

void UiSystem::handle(rpg::ItemEvent const& event) {
	audio.receive(event);
}

void UiSystem::handle(rpg::PerkEvent const& event) {
	audio.receive(event);
}

void UiSystem::handle(rpg::ActionEvent const& event) {
	audio.receive(event);
}

void UiSystem::handle(rpg::CombatEvent const& event) {
	//audio.receive(event);
}

void UiSystem::handle(game::PowerupEvent const& event) {
	hud.receive(event);
	audio.receive(event);
}

// ---------------------------------------------------------------------------

sf::Time UiSystem::update(sf::Time const& elapsed, bool use_autocam) {
	sf::Clock clock;

	dispatch<core::AnimationEvent>(*this);
	dispatch<core::TeleportEvent>(*this);
	dispatch<core::FocusEvent>(*this);
	dispatch<core::SpriteEvent>(*this);
	dispatch<core::SoundEvent>(*this);
	dispatch<core::MusicEvent>(*this);
	dispatch<core::MoveEvent>(*this);
	dispatch<rpg::StatsEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::ProjectileEvent>(*this);
	dispatch<rpg::ExpEvent>(*this);
	dispatch<rpg::FeedbackEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<rpg::ItemEvent>(*this);
	dispatch<rpg::PerkEvent>(*this);
	dispatch<rpg::ProjectileEvent>(*this);
	dispatch<rpg::CombatEvent>(*this);
	dispatch<rpg::ActionEvent>(*this);
	dispatch<game::PowerupEvent>(*this);

	animation.update(elapsed);
	render.update(elapsed);
	sound.update(elapsed);
	music.update(elapsed);
	audio.update(elapsed);
	hud.update(elapsed);
	visuals.update(elapsed);
	
	used_autocam = use_autocam;
	if (use_autocam && autocam.update(elapsed)) {
		updateHuds();
	}
	
	return clock.restart();
}

void UiSystem::clear() {
	camera.clear();
}

}  // ::state
