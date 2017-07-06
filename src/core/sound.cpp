#include <Thor/Math.hpp>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

#include <core/sound.hpp>

namespace core {

namespace sound_impl {

Context::Context(LogContext& log, std::size_t pool_size)
	: log{log}
	, pool{}
	, volume{100.f}
	, threshold{sf::milliseconds(250u)} {
	ASSERT(pool_size <= 256u);
	pool.resize(pool_size);
}

// ---------------------------------------------------------------------------

void onSound(Context& context, SoundEvent const & event) {
	auto channel = getChannel(context, event);
	if (channel != nullptr) {
		play(context, *channel, event);
	}
}

sf::Sound* getChannel(Context& context, SoundEvent const & event) {
	sf::Sound *free{nullptr}, *recycle{nullptr};
	float progress{0.f};
	for (auto& channel: context.pool) {
		auto status = channel.getStatus();
		
		if (free == nullptr) {
			// find most recent channel
			if (status == sf::SoundSource::Status::Stopped) {
				// channel is currently not used
				free = &channel;
				
			} else {
				// consider channel for being recycled
				auto buffer = channel.getBuffer();
				ASSERT(buffer != nullptr);
				float tmp = channel.getPlayingOffset() / buffer->getDuration();
				if (tmp > progress || recycle == nullptr) {
					recycle = &channel;
					progress = tmp;
				}
			}
		}
		
		// check for too fast playback
		if (status == sf::SoundSource::Status::Playing && channel.getBuffer() == event.buffer) {
			auto offset = channel.getPlayingOffset();
			if (offset <= context.threshold) {
				// completly ignore this event
				context.log.debug << "[Core/Sound] Ignored too fast playback: "
					<< offset << " / " << context.threshold << "\n";
				return nullptr;
			}
		}
	}
	
	if (free == nullptr) {
		context.log.debug << "[Core/Sound] Recycling playback channel\n";
		free = recycle;
	}
	ASSERT(free != nullptr);
	return free;
}

void play(Context& context, sf::Sound& sound, SoundEvent const & event) {
	auto volume = context.volume * event.relative_volume;
	if (volume < 0.f) {
		volume = 0.f;
		context.log.debug << "[Core/Sound] Fixed too low playback volume\n";
		
	} else if (volume > context.volume) {
		volume = context.volume;
		context.log.debug << "[Core/Sound] Fixed too high playback volume\n";
		
	}
	
	sound.setBuffer(*event.buffer);
	sound.setVolume(volume);
	sound.setPitch(event.pitch);
	sound.play();
}

}  // ::sound_impl

// --------------------------------------------------------------------

SoundSystem::SoundSystem(LogContext& log, std::size_t pool_size)
	// Event API
	: utils::EventListener<SoundEvent>{}
	, context{log, pool_size} {
	context.log.debug << "[Core/Sound] Initialized with poolsize "
		<< pool_size << "\n";
}

void SoundSystem::setVolume(float volume) {
	context.volume = volume;
}

void SoundSystem::setThreshold(sf::Time threshold) {
	context.threshold = threshold;
}

void SoundSystem::handle(SoundEvent const& event) {
	if (event.buffer == nullptr) {
		return;
	}
	sound_impl::onSound(context, event);
}

void SoundSystem::update(sf::Time const& elapsed) {
	dispatch<SoundEvent>(*this);
}

}  // ::core
