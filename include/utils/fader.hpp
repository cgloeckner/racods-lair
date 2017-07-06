#pragma once
#include <functional>
#include <SFML/Audio.hpp>

namespace utils {

// lambda to control fading "speed"
using FadeFunc = std::function<float(sf::Time const&, float)>;

float default_fade_in(sf::Time const & delta, float value);
float default_fade_out(sf::Time const & delta, float value);

float dummy_fade_in(sf::Time const & delta, float value);
float dummy_fade_out(sf::Time const & delta, float value);

/// Fading mode
enum class FadeMode { None, In, Out };

/// Fader that applies fading to a `sf::SoundSource` by changing its volume
struct Fader {
	/// Default constructs fader
	/**
	 * This will set fader functions for fade-in and fade-out to default
	 * lambdas.
	 */
	Fader();

	/// Fading operation
	/**
	 * Applies fading to a given `sf::SoundSource`. This method should be
	 * called once a frame with each fading sound source in order to fade it
	 * in or out.
	 * If `FadeMode::None` is given, the volume is not changed unless it
	 * exceeds `max_volume`. In this case the volume is adjusted.
	 * @param elapsed Time since last frame
	 * @param sound Reference to sound source whose volume is changed
	 * @param mode `FadeMode` determining whether fade-in, fade-out or none.
	 * @param max_volume Maximum volume for fade-in
	 * @return true if volume was changed
	 */
	bool operator()(sf::Time const& elapsed, sf::SoundSource& sound,
		FadeMode mode, float max_volume = 100.f);

	/// Fading function for fade-in. Modify it to change fade-in behavior.
	FadeFunc in;

	/// Fading function for fade-ou. Modify it to change fade-out behavior.
	FadeFunc out;
};

// ---------------------------------------------------------------------------

/// A simple music manager using a fader
/**
 * It provides playing `sf::Music` with fading between two tracks. Also
 * starting/stopping playback is faded. When fading between two tracks, a
 * simple crossfade is applied.
 */
class Music {
  private:
	/// Determines fading-state of the manager
	enum FadeState { Idle, AtoB, BtoA };

	/// A music channel
	struct Channel {
		sf::Music music;	   // music instance used for playback
		utils::FadeMode mode;  // fading mode

		/// Default constructs Channel with `FadeMode::None`
		Channel();
		
		bool isPlaying() const;
	};

	Channel primary, secondary;  // primary and secondary music channel
	utils::Fader fader;			 // fader instance
	float max_volume;			 // maximum volume for music playback

  public:
	/// Default construct Music with empty channels and max volume
	Music();

	/// Trigger a new track to be played
	/**
	 * The most recent channel is chosen in order to start the playback.
	 * @param filename Determines source of music
	 */
	void play(std::string const& filename);

	/// Trigger the current track to stop
	void stop();
	
	void pause();
	void resume();

	/// Update manager
	/**
	 * This will handle (cross-)fading for primary and secondary channel.
	 * @param elapsed Time since last frame.
	 */
	void update(sf::Time const& elapsed);

	/// Set maximum music volume
	/**
	 * Sets the maximum volume for music playback. According to SFML, a
	 * valid value must be chosen from [0.f, 100.f].
	 * The volume of all channels is adjusted not to exceed the given
	 * maximum.
	 * @param volume Maximum volume to set for playback
	 */
	void setMaxVolume(float volume);

	/// Get maximum music volume
	/**
	 * @see `setMaxVolume(float)`.
	 * @return maximum volume which was set to the manager
	 */
	float getMaxVolume() const;

	/// Return a reference to the fader
	/**
	 * This can be used do modify the fader's fade-in and -out lambdas in
	 * order to modify fading behavior.
	 * @return Reference to the underlying fader
	 */
	utils::Fader& getFader();

	/// Return a reference to the most recent music
	/**
	 * This can be used to modify the music instance which is most recent.
	 * It will always return the instance which is fading in. If no one
	 * is fading in, the loudest channel is chosen.
	 */
	sf::Music& getMusic();
	
	bool isPlaying() const;
};

} // ::utils
