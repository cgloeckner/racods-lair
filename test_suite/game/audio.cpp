#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <game/audio.hpp>

struct AudioFixture {
	sf::SoundBuffer dummy;

	core::IdManager id_manager;
	core::LogContext log;
	std::vector<core::ObjectID> ids;
	core::SoundManager sound_manager;
	rpg::ItemManager item_manager;
	rpg::PlayerManager player_manager;
	core::SoundSender sounds;
	core::MusicSender music;
	game::audio_impl::Context context;
	
	rpg::ItemTemplate item;
	rpg::PerkTemplate perk;

	AudioFixture()
		: dummy{}
		, id_manager{}
		, log{}
		, sound_manager{}
		, item_manager{}
		, player_manager{}
		, sounds{}
		, music{}
		, context{log, sound_manager, item_manager, player_manager, sounds, music}
		, item{}
		, perk{} {
		// fill dummy sound
		std::vector<std::int16_t> spam;
		spam.resize(44100, 0u);
		dummy.loadFromSamples(spam.data(), 44100, 1, 44100);
	}

	void reset() {
		for (auto id : ids) {
			player_manager.tryRelease(id);
			sound_manager.release(id);
		}
		ids.clear();
		sound_manager.cleanup();
		player_manager.cleanup();
		sounds.clear();
		music.clear();
		context.levelup.clear();
		context.powerup.clear();
		for (auto& pair: context.feedback) {
			pair.second.clear();
		}
		context.music.clear();
		item.sound = &dummy;
		item.slot = rpg::EquipmentSlot::None;
		perk.sound = &dummy;
	}

	core::ObjectID add_object(bool player=false) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& sound = sound_manager.acquire(id);
		for (auto& pair: sound.sfx) {
			pair.second.push_back(&dummy);
		}
		if (player) {
			player_manager.acquire(id);
		}
		return id;
	}
};

// --------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(audio_test)

BOOST_AUTO_TEST_CASE(random_music_is_played_after_previous_music_stopped) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	fix.context.music.push_back("foo");
	
	game::audio_impl::onMusicStopped(fix.context);
	auto const & data = fix.music.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].filename, "foo");
}

BOOST_AUTO_TEST_CASE(cannot_play_music_if_none_assigned) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	game::audio_impl::onMusicStopped(fix.context);
	auto const & data = fix.music.data();
	BOOST_CHECK(data.empty());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(onAction_triggers_event_with_any_sound_specified_for_the_given_action) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	game::audio_impl::onAction(fix.context, id, core::SoundAction::Death);
	
	auto const & data = fix.sounds.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].buffer, &fix.dummy);
}

BOOST_AUTO_TEST_CASE(onAction_does_not_trigger_an_event_if_no_action_is_specified_for_the_given_action) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	fix.sound_manager.query(id).sfx[core::SoundAction::Death].clear();
	game::audio_impl::onAction(fix.context, id, core::SoundAction::Death);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(onItem_triggers_no_event_for_adding_an_item) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::ItemEvent event;
	event.type = rpg::ItemEvent::Add;
	event.item = &fix.item;
	
	game::audio_impl::onItem(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

BOOST_AUTO_TEST_CASE(onItem_triggers_no_event_for_removing_an_item) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::ItemEvent event;
	event.type = rpg::ItemEvent::Remove;
	event.item = &fix.item;
	
	game::audio_impl::onItem(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

BOOST_AUTO_TEST_CASE(onItem_triggers_no_event_for_equipping_or_unequipping_an_item) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::ItemEvent event;
	event.type = rpg::ItemEvent::Use;
	event.item = &fix.item;
	event.slot = rpg::EquipmentSlot::Weapon;
	
	game::audio_impl::onItem(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

BOOST_AUTO_TEST_CASE(onItem_triggers_event_for_use_non_equipment_item) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::ItemEvent event;
	event.type = rpg::ItemEvent::Use;
	event.item = &fix.item;
	
	game::audio_impl::onItem(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].buffer, &fix.dummy);
}

BOOST_AUTO_TEST_CASE(onItem_triggers_no_event_if_no_sound_is_specified) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::ItemEvent event;
	event.type = rpg::ItemEvent::Use;
	event.item = &fix.item;
	fix.item.sound = nullptr;
	
	game::audio_impl::onItem(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(onPerk_triggers_no_event_if_perk_level_is_set) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::PerkEvent event;
	event.type = rpg::PerkEvent::Set;
	event.perk = &fix.perk;
	
	game::audio_impl::onPerk(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

BOOST_AUTO_TEST_CASE(onPerk_triggers_event_if_sound_is_used) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::PerkEvent event;
	event.type = rpg::PerkEvent::Use;
	event.perk = &fix.perk;
	
	game::audio_impl::onPerk(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].buffer, &fix.dummy);
}

BOOST_AUTO_TEST_CASE(onPerk_triggers_no_event_if_no_sound_is_specified) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::PerkEvent event;
	event.type = rpg::PerkEvent::Use;
	event.perk = &fix.perk;
	fix.perk.sound = nullptr;
	
	game::audio_impl::onPerk(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(onFeedback_triggers_sound_if_bound_to_the_given_feedback_type) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::FeedbackEvent event;
	event.actor = fix.add_object(true);
	event.type = rpg::FeedbackType::ItemNotFound;
	fix.context.feedback[event.type].push_back(&fix.dummy);
	
	game::audio_impl::onFeedback(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].buffer, &fix.dummy);
}

BOOST_AUTO_TEST_CASE(onFeedback_triggers_no_sound_if_none_is_bound_to_the_given_feedback_type) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::FeedbackEvent event;
	event.actor = fix.add_object(true);
	event.type = rpg::FeedbackType::ItemNotFound;
	
	game::audio_impl::onFeedback(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

BOOST_AUTO_TEST_CASE(onFeedback_triggers_sound_if_actor_is_not_a_player) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::FeedbackEvent event;
	event.actor = fix.add_object(false);
	event.type = rpg::FeedbackType::ItemNotFound;
	fix.context.feedback[event.type].push_back(&fix.dummy);
	
	game::audio_impl::onFeedback(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}
// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(onExp_triggers_levelup_sound_if_bound_and_levelup_occured) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	fix.context.levelup.push_back(&fix.dummy);
	rpg::ExpEvent event;
	event.levelup = 1u;
	game::audio_impl::onExp(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].buffer, &fix.dummy);
}

BOOST_AUTO_TEST_CASE(onExp_does_not_trigger_levelup_sound_no_levelup_occured) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	fix.context.levelup.push_back(&fix.dummy);
	rpg::ExpEvent event;
	event.levelup = 0u;
	game::audio_impl::onExp(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

BOOST_AUTO_TEST_CASE(onExp_trigger_levelup_sound_if_none_is_bound) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	rpg::ExpEvent event;
	event.levelup = 1u;
	game::audio_impl::onExp(fix.context, event);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(onPowerup_triggers_powerup_sound_if_bound) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	fix.context.powerup.push_back(&fix.dummy);
	game::audio_impl::onPowerup(fix.context);
	
	auto const & data = fix.sounds.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].buffer, &fix.dummy);
}

BOOST_AUTO_TEST_CASE(onPowerup_does_not_trigger_powerup_sound_if_none_is_bound) {
	auto& fix = Singleton<AudioFixture>::get();
	fix.reset();
	
	game::audio_impl::onPowerup(fix.context);
	
	auto const & data = fix.sounds.data();
	BOOST_CHECK(data.empty());
}


BOOST_AUTO_TEST_SUITE_END()
