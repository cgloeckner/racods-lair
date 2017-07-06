#pragma once
#include <SFML/Audio/Music.hpp>

#include <utils/enum_map.hpp>
#include <utils/fader.hpp>

#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>
#include <core/entity.hpp>

namespace core {

/// Handles stream-based audio playback for ambience, music and speed
class MusicSystem
	// Event API
	: public utils::EventListener<core::MusicEvent>
	, public utils::EventSender<core::MusicEvent> {

  protected:
	LogContext& log;
	std::string const base_path, ext;
	utils::Music music;

  public:
	MusicSystem(LogContext& log, std::string const& base_path, std::string const & ext);

	void setVolume(float volume);
	void pause();
	void resume();

	void handle(MusicEvent const& event);
	void update(sf::Time const& elapsed);
};

}  // ::core
