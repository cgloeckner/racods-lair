#include <cstdint>
#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <core/sound.hpp>

struct SoundFixture {
	sf::SoundBuffer dummy_sound, another;

	core::LogContext log;
	core::sound_impl::Context context;

	SoundFixture()
		: dummy_sound{}
		, another{}
		, log{}
		, context{log, 4u} {
		//log.debug.add(std::cout);
		// fill dummy sounds with 1 sec silence
		std::vector<std::int16_t> spam;
		spam.resize(44100, 0u);
		dummy_sound.loadFromSamples(spam.data(), 44100, 1, 44100);
		another.loadFromSamples(spam.data(), 44100, 1, 44100);
	}

	~SoundFixture() {
		reset();
	}

	void reset() {
		context.volume = 50.f;
		context.threshold = sf::milliseconds(250u);
		for (auto& sound: context.pool) {
			sound.stop();
		}
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}

};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(sound_test)

BOOST_AUTO_TEST_CASE(getChannel_returns_first_channel_if_free) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	
	auto channel = core::sound_impl::getChannel(fix.context, event);
	BOOST_REQUIRE(channel != nullptr);
	BOOST_CHECK_EQUAL(channel, &fix.context.pool[0]);
}

BOOST_AUTO_TEST_CASE(getChannel_returns_first_free_channel) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	
	fix.context.pool[0].setBuffer(fix.another);
	fix.context.pool[0].play();
	fix.context.pool[1].setBuffer(fix.another);
	fix.context.pool[1].play();
	
	auto channel = core::sound_impl::getChannel(fix.context, event);
	BOOST_REQUIRE(channel != nullptr);
	BOOST_CHECK_EQUAL(channel, &fix.context.pool[2]);
}

BOOST_AUTO_TEST_CASE(getChannel_returns_most_recent_channel_to_recycle_if_all_are_playing) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	
	fix.context.pool[0].setBuffer(fix.another);
	fix.context.pool[0].setPlayingOffset(sf::seconds(0.2f));
	fix.context.pool[1].setBuffer(fix.another);
	fix.context.pool[1].setPlayingOffset(sf::seconds(0.3f));
	fix.context.pool[2].setBuffer(fix.another);
	fix.context.pool[2].setPlayingOffset(sf::seconds(0.1f));
	fix.context.pool[3].setBuffer(fix.another);
	fix.context.pool[3].setPlayingOffset(sf::seconds(0.7f));
	for (auto& s: fix.context.pool) {
		s.play();
	}
	
	auto channel = core::sound_impl::getChannel(fix.context, event);
	BOOST_REQUIRE(channel != nullptr);
	BOOST_CHECK_EQUAL(channel, &fix.context.pool[3]);
}

BOOST_AUTO_TEST_CASE(getChannel_returns_no_channel_if_buffer_already_played_within_threshold) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	
	fix.context.pool[1].setBuffer(fix.dummy_sound);
	fix.context.pool[1].setPlayingOffset(sf::seconds(0.2f));
	fix.context.pool[1].play();
	
	auto channel = core::sound_impl::getChannel(fix.context, event);
	BOOST_REQUIRE(channel == nullptr);
}

BOOST_AUTO_TEST_CASE(getChannel_also_returns_free_channel_if_buffer_already_played_but_beyond_threshold) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	
	fix.context.pool[1].setBuffer(fix.dummy_sound);
	fix.context.pool[1].setPlayingOffset(sf::seconds(0.3f));
	fix.context.pool[1].play();
	
	auto channel = core::sound_impl::getChannel(fix.context, event);
	BOOST_REQUIRE(channel != nullptr);
	BOOST_CHECK_EQUAL(channel, &fix.context.pool[0]);
}

BOOST_AUTO_TEST_CASE(getChannel_also_returns_recycle_channel_if_buffer_already_played_but_beyond_threshold) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	
	fix.context.pool[0].setBuffer(fix.dummy_sound);
	fix.context.pool[0].setPlayingOffset(sf::seconds(0.6f));
	fix.context.pool[1].setBuffer(fix.dummy_sound);
	fix.context.pool[1].setPlayingOffset(sf::seconds(0.3f));
	fix.context.pool[2].setBuffer(fix.dummy_sound);
	fix.context.pool[2].setPlayingOffset(sf::seconds(0.7f));
	fix.context.pool[3].setBuffer(fix.dummy_sound);
	fix.context.pool[3].setPlayingOffset(sf::seconds(0.4f));
	for (auto& s: fix.context.pool) {
		s.play();
	}
	
	auto channel = core::sound_impl::getChannel(fix.context, event);
	BOOST_REQUIRE(channel != nullptr);
	BOOST_CHECK_EQUAL(channel, &fix.context.pool[2]);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(play_starts_playback_with_proper_volume_pitch_and_buffer) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	event.pitch = 0.345f;
	event.relative_volume = 0.4f;
	
	auto& channel = fix.context.pool[0];
	core::sound_impl::play(fix.context, channel, event);
	
	BOOST_CHECK(channel.getStatus() == sf::SoundSource::Status::Playing);
	BOOST_CHECK_EQUAL(channel.getBuffer(), event.buffer);
	BOOST_CHECK_CLOSE(channel.getPitch(), event.pitch, 0.0001f);
	BOOST_CHECK_CLOSE(channel.getVolume(), fix.context.volume * event.relative_volume, 0.0001f);
}

BOOST_AUTO_TEST_CASE(play_fixes_too_high_volume) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	event.relative_volume = 999.f;
	
	auto& channel = fix.context.pool[0];
	core::sound_impl::play(fix.context, channel, event);
	
	BOOST_CHECK(channel.getStatus() == sf::SoundSource::Status::Playing);
	BOOST_CHECK_EQUAL(channel.getBuffer(), event.buffer);
	BOOST_CHECK_CLOSE(channel.getVolume(), fix.context.volume, 0.0001f);
}

BOOST_AUTO_TEST_CASE(play_fixes_too_low_volume) {
	auto& fix = Singleton<SoundFixture>::get();
	fix.reset();
	
	core::SoundEvent event;
	event.buffer = &fix.dummy_sound;
	event.relative_volume = -0.2f;
	
	auto& channel = fix.context.pool[0];
	core::sound_impl::play(fix.context, channel, event);
	
	BOOST_CHECK(channel.getStatus() == sf::SoundSource::Status::Playing);
	BOOST_CHECK_EQUAL(channel.getBuffer(), event.buffer);
	BOOST_CHECK_CLOSE(channel.getVolume(), 0.f, 0.0001f);
}

BOOST_AUTO_TEST_SUITE_END()
