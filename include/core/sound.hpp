#pragma once
#include <SFML/Audio/Sound.hpp>

#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>

namespace core {

namespace sound_impl {

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	std::vector<sf::Sound> pool;
	float volume;
	sf::Time threshold;

	/// @pre pool_size <= 256
	Context(LogContext& log, std::size_t pool_size);
};

// ---------------------------------------------------------------------

void onSound(Context& context, SoundEvent const & event);

sf::Sound* getChannel(Context& context, SoundEvent const & event);
void play(Context& context, sf::Sound& sound, SoundEvent const & event);

}  // ::sound_impl

// ---------------------------------------------------------------------------
// Sound System

/// Handles playback of sound effects
/// The system provides a pool of audio channels that can be used for
/// playback. If playback is triggered, one non-playing channel is
/// selected. If all channels are busy, the most-recent channel
/// (referring to the largest playing offset) is chosen.
/// If a sound is played too fast, it will be ignored for a specified
/// time span.
class SoundSystem
	// Event API
	: public utils::EventListener<SoundEvent> {

  protected:
	sound_impl::Context context;

  public:
	SoundSystem(LogContext& log, std::size_t pool_size);

	/// Sets volume for all upcomming sounds. No currently played sound
	/// is affected.
	void setVolume(float volume);
	
	void setThreshold(sf::Time threshold);
	
	void handle(SoundEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::core
