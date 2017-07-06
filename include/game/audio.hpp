#pragma once
#include <utils/enum_map.hpp>

#include <core/entity.hpp>
#include <core/event.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>
#include <game/event.hpp>

namespace game {

namespace audio_impl {

struct Context {
	core::LogContext& log;
	core::SoundManager const & sounds;
	rpg::ItemManager const & items;
	rpg::PlayerManager const & players;
	core::SoundSender& sound_sender;
	core::MusicSender& music_sender;
	
	utils::EnumMap<rpg::FeedbackType, std::vector<sf::SoundBuffer const *>> feedback;
	std::vector<std::string> music;
	std::vector<sf::SoundBuffer const *> levelup, powerup;
	
	Context(core::LogContext& log, core::SoundManager const & sounds,
		rpg::ItemManager const & items, rpg::PlayerManager const & players,
		core::SoundSender& sound_sender, core::MusicSender& music_sender);
};

void onMusicStopped(Context& context);

void onAction(Context& context, core::ObjectID actor, core::SoundAction action);
void onItem(Context& context, rpg::ItemEvent const & event);
void onPerk(Context& context, rpg::PerkEvent const & event);
void onFeedback(Context& context, rpg::FeedbackEvent const & event);

void onExp(Context& context, rpg::ExpEvent const & event);
void onPowerup(Context& context);

} // ::audio_impl

// --------------------------------------------------------------------

class AudioSystem
	// Event API
	: public utils::EventListener<core::MusicEvent, core::MoveEvent,
		rpg::ItemEvent, rpg::PerkEvent, rpg::StatsEvent, rpg::DeathEvent,
		rpg::SpawnEvent, rpg::FeedbackEvent, rpg::ExpEvent, rpg::ProjectileEvent,
		rpg::ActionEvent, game::PowerupEvent>
	, public utils::EventSender<core::SoundEvent, core::MusicEvent>
	// Component API
	, public core::SoundManager {
	
  private:
	audio_impl::Context context;
	
  public:
	AudioSystem(core::LogContext& log, std::size_t max_objects,
		rpg::ItemManager const & items, rpg::PlayerManager const & players);
	
	void assign(rpg::FeedbackType type, sf::SoundBuffer const & buffer);
	void addMusic(std::string const & filename);
	void addLevelup(sf::SoundBuffer const & buffer);
	void addPowerup(sf::SoundBuffer const & buffer);
	
	void handle(core::MusicEvent const & event);
	void handle(core::MoveEvent const & event);
	void handle(rpg::ItemEvent const & event);
	void handle(rpg::PerkEvent const & event);
	void handle(rpg::StatsEvent const & event);
	void handle(rpg::DeathEvent const & event);
	void handle(rpg::SpawnEvent const & event);
	void handle(rpg::FeedbackEvent const & event);
	void handle(rpg::ExpEvent const & event);
	void handle(rpg::ProjectileEvent const & event);
	void handle(rpg::ActionEvent const & event);
	void handle(game::PowerupEvent const & event);
	
	void update(sf::Time const& elapsed);
};

} // ::game
