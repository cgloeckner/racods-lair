#pragma once
#include <SFML/Graphics.hpp>

#include <core/event.hpp>
#include <core/animation.hpp>
#include <core/music.hpp>
#include <core/render.hpp>
#include <core/sound.hpp>
#include <rpg/event.hpp>
#include <game/audio.hpp>
#include <game/autocam.hpp>
#include <game/event.hpp>
#include <game/hud.hpp>
#include <game/visuals.hpp>
#include <engine/event.hpp>

namespace engine {

struct UiSystem
	: utils::EventListener<core::AnimationEvent, core::TeleportEvent,
		core::SpriteEvent, core::SoundEvent,
		core::MusicEvent, core::MoveEvent, rpg::StatsEvent, rpg::DeathEvent,
		rpg::SpawnEvent, rpg::ProjectileEvent, rpg::ExpEvent,
		rpg::FeedbackEvent, rpg::ItemEvent, rpg::PerkEvent,
		rpg::CombatEvent, rpg::ActionEvent, game::PowerupEvent>
	, sf::Drawable {

	utils::LightingSystem lighting;
	core::CameraSystem camera;
	core::AnimationSystem animation;
	mutable core::RenderSystem render;
	core::SoundSystem sound;
	core::MusicSystem music;
	game::AudioSystem audio;
	game::AutoCamSystem autocam;
	game::HudSystem hud;
	game::VisualsSystem visuals;
	
	bool used_autocam;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	UiSystem(core::LogContext& log, std::size_t max_objects, sf::Vector2u const& screen_size,
		sf::Texture const& lightmap, float zoom, unsigned int audio_poolsize,
		core::MovementManager const& movement, core::FocusManager const & focus,
		core::CollisionManager const & collision,
		core::DungeonSystem& dungeon, rpg::StatsManager const& stats,
		rpg::ItemManager const & item, rpg::PlayerManager const & player,
		game::Localization& locale, std::string const & music_base,
		std::string const & music_ext);
	
	void connect(MultiEventListener& listener);
	void disconnect(MultiEventListener& listener);

	void updateHuds();

	template <typename T>
	void bind(utils::SingleEventListener<T>& listener);
	
	template <typename T>
	void unbind(utils::SingleEventListener<T> const & listener);

	void handle(sf::Event const& event);
	void handle(core::AnimationEvent const& event);
	void handle(core::TeleportEvent const& event);
	void handle(core::SpriteEvent const& event);
	void handle(core::SoundEvent const& event);
	void handle(core::MusicEvent const& event);
	void handle(core::MoveEvent const& event);
	void handle(rpg::StatsEvent const& event);
	void handle(rpg::DeathEvent const& event);
	void handle(rpg::SpawnEvent const& event);
	void handle(rpg::ProjectileEvent const& event);
	void handle(rpg::ExpEvent const& event);
	void handle(rpg::FeedbackEvent const& event);
	void handle(rpg::ItemEvent const& event);
	void handle(rpg::PerkEvent const& event);
	void handle(rpg::CombatEvent const& event);
	void handle(rpg::ActionEvent const& event);
	void handle(game::PowerupEvent const& event);

	sf::Time update(sf::Time const& elapsed, bool use_autocam);
	
	void clear();
};

}  // ::state
