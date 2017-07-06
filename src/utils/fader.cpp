#include <utils/fader.hpp>

namespace utils {

float default_fade_in(sf::Time const & elapsed, float value) {
	return value + elapsed.asMilliseconds() / 10.f;
}

float default_fade_out(sf::Time const & elapsed, float value) {
	return value - elapsed.asMilliseconds() / 10.f;
}

float dummy_fade_in(sf::Time const &, float) {
	return 100.f;
}

float dummy_fade_out(sf::Time const &, float) {
	return 0.f;
}

Fader::Fader()
	: in{default_fade_in}
	, out{default_fade_out} {}

bool Fader::operator()(sf::Time const& elapsed, sf::SoundSource& sound,
	FadeMode mode, float max_volume) {
	float volume = sound.getVolume();
	bool done{false};

	switch (mode) {
		case FadeMode::In:
			volume = std::min(max_volume, in(elapsed, volume));
			done = (volume == max_volume);
			break;

		case FadeMode::Out:
			volume = std::max(0.f, out(elapsed, volume));
			done = (volume == 0.f);
			break;

		case FadeMode::None:
			done = (volume > max_volume);
			volume = std::max(volume, max_volume);
			break;
	}

	sound.setVolume(volume);
	return done;
}

// ---------------------------------------------------------------------------

Music::Channel::Channel()
	: music{}
	, mode{utils::FadeMode::None} {
}

bool Music::Channel::isPlaying() const {
	if (music.getStatus() == sf::SoundSource::Status::Playing) {
		// is playing if not fading out
		return mode != utils::FadeMode::Out;
	}
	
	// not playing
	return false;
}

Music::Music()
	: primary{}, secondary{}, fader{}, max_volume{100.f} {}

void Music::play(std::string const& filename) {
	if (primary.music.getStatus() == sf::SoundSource::Status::Stopped) {
		// play at primary channel
		primary.music.openFromFile(filename);
		primary.music.play();
		primary.music.setVolume(0.f);
		primary.mode = utils::FadeMode::In;
		secondary.mode = utils::FadeMode::Out;

	} else if (secondary.music.getStatus() ==
			   sf::SoundSource::Status::Stopped) {
		// play at secondary channel
		secondary.music.openFromFile(filename);
		secondary.music.play();
		secondary.music.setVolume(0.f);
		secondary.mode = utils::FadeMode::In;
		primary.mode = utils::FadeMode::Out;

	} else {
		// play at channel which is quieter
		if (primary.music.getVolume() < secondary.music.getVolume()) {
			primary.music.openFromFile(filename);
			primary.music.play();
			primary.music.setVolume(0.f);
			primary.mode = utils::FadeMode::In;
			secondary.mode = utils::FadeMode::Out;

		} else {
			secondary.music.openFromFile(filename);
			secondary.music.play();
			secondary.music.setVolume(0.f);
			secondary.mode = utils::FadeMode::In;
			primary.mode = utils::FadeMode::Out;
		}
	}
}

void Music::stop() {
	if (primary.music.getStatus() == sf::SoundSource::Status::Playing) {
		// fade-out primary channel
		primary.mode = utils::FadeMode::Out;
	}

	if (secondary.music.getStatus() == sf::SoundSource::Status::Playing) {
		// fade-out secondary channel
		secondary.mode = utils::FadeMode::Out;
	}
}

void Music::pause() {
	if (primary.music.getStatus() == sf::SoundSource::Status::Playing) {
		primary.music.pause();
		primary.mode = utils::FadeMode::None;
	}

	if (secondary.music.getStatus() == sf::SoundSource::Status::Playing) {
		secondary.music.pause();
		secondary.mode = utils::FadeMode::None;
	}
}

void Music::resume() {
	if (primary.music.getStatus() == sf::SoundSource::Status::Paused) {
		primary.music.play();
		primary.mode = utils::FadeMode::In;
	}

	if (secondary.music.getStatus() == sf::SoundSource::Status::Paused) {
		secondary.music.play();
		secondary.mode = utils::FadeMode::In;
	}
}

void Music::update(sf::Time const& elapsed) {
	bool done1 = fader(elapsed, primary.music, primary.mode, max_volume);
	bool done2 = fader(elapsed, secondary.music, secondary.mode, max_volume);
	if (done1 && done2) {
		// stop playback of fade-out channel
		if (primary.mode == utils::FadeMode::Out) {
			primary.music.stop();
		}
		if (secondary.mode == utils::FadeMode::Out) {
			secondary.music.stop();
		}
		// stop fading
		primary.mode = utils::FadeMode::None;
		secondary.mode = utils::FadeMode::None;
	}
}

void Music::setMaxVolume(float volume) {
	max_volume = volume;

	// cap channels' volume
	if (primary.music.getVolume() > volume) {
		primary.music.setVolume(volume);
	}
	if (secondary.music.getVolume() > volume) {
		secondary.music.setVolume(volume);
	}

	// make channels fade to max volume (if they're not fading, yet)
	if (primary.mode == utils::FadeMode::None) {
		primary.mode = utils::FadeMode::In;
	}
	if (secondary.mode == utils::FadeMode::None) {
		secondary.mode = utils::FadeMode::In;
	}
}

float Music::getMaxVolume() const { return max_volume; }

utils::Fader& Music::getFader() { return fader; }

sf::Music& Music::getMusic() {
	bool prim_play =
		(primary.music.getStatus() == sf::SoundSource::Status::Playing);
	bool sec_play =
		(secondary.music.getStatus() == sf::SoundSource::Status::Playing);

	if (prim_play && !sec_play) {
		return primary.music;

	} else if (!prim_play && sec_play) {
		return secondary.music;

	} else if (!prim_play && !sec_play) {
		return primary.music;

	} else {
		// both are playing - pick which is fading in
		bool prim_in = (primary.mode == utils::FadeMode::In);
		bool sec_in = (secondary.mode == utils::FadeMode::In);
		if (prim_in && !sec_in) {
			return primary.music;

		} else if (!prim_in && sec_in) {
			return secondary.music;

		} else if (!prim_in && !sec_in) {
			return primary.music;

		} else {
			// both are fading in - pick loudest
			if (primary.music.getVolume() > secondary.music.getVolume()) {
				return primary.music;
			} else {
				return secondary.music;
			}
		}
	}
}

bool Music::isPlaying() const {
	return primary.isPlaying() || secondary.isPlaying();
}

} // ::utils
