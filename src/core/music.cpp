#include <cstdlib>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

#include <core/music.hpp>

namespace core {

MusicSystem::MusicSystem(LogContext& log, std::string const& base_path,
	std::string const & ext)
	// Event API
	: utils::EventListener<core::MusicEvent>{}
	, utils::EventSender<core::MusicEvent>{}
	, log{log}
	, base_path{base_path}
	, ext{ext}
	, music{} {
}

void MusicSystem::setVolume(float volume) {
	music.setMaxVolume(volume);
}

void MusicSystem::pause() {
	music.pause();
	log.debug << "[Core/Music] Paused\n";
}

void MusicSystem::resume() {
	music.resume();
	log.debug << "[Core/Music] Resumed\n";
}

void MusicSystem::handle(MusicEvent const& event) {
	auto filename = base_path + "/" + event.filename + ext;
	music.play(filename);
	log.debug << "[Core/Music] Now playing " << event.filename << "\n";
}

void MusicSystem::update(sf::Time const& elapsed) {
	dispatchAll(*this);
	
	music.update(elapsed);
	if (!music.isPlaying()) {
		// notify about inactive music
		core::MusicEvent event;
		send(event);
	}
	
	propagateAll();
}

}  // ::core
