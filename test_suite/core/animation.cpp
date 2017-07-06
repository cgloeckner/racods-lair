#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <core/animation.hpp>

struct AnimationFixture {
	sf::Texture dummy_tileset;
	core::IdManager id_manager;
	std::vector<core::ObjectID> ids;

	core::LogContext log;
	core::AnimationSender animation_sender;
	core::AnimationManager animation_manager;
	core::animation_impl::Context context;

	struct {
		utils::ActionFrames legs;
		utils::EnumMap<core::AnimationAction, utils::ActionFrames> torso;
	} demo_template;

	AnimationFixture()
		: dummy_tileset{}
		, id_manager{}
		, log{}
		, animation_sender{}
		, animation_manager{}
		, context{log, animation_sender, animation_manager} {
		// create demo animation template
		demo_template.legs.frames.reserve(4u);
		demo_template.legs.append(
			{0, 0, 10, 5}, {1.f, 0.5f}, sf::milliseconds(15));
		demo_template.legs.append(
			{10, 0, 10, 5}, {1.f, 0.5f}, sf::milliseconds(17));
		demo_template.legs.append(
			{20, 0, 10, 5}, {1.f, 0.5f}, sf::milliseconds(23));
		demo_template.legs.append(
			{30, 0, 10, 5}, {1.f, 0.5f}, sf::milliseconds(12));
		demo_template.legs.refresh();
		for (auto& pair : demo_template.torso) {
			pair.second.frames.reserve(4u);
			pair.second.append(
				{0, 5, 10, 5}, {1.f, 0.5f}, sf::milliseconds(15));
			pair.second.append(
				{10, 5, 10, 5}, {1.f, 0.5f}, sf::milliseconds(17));
			pair.second.append(
				{20, 5, 10, 5}, {1.f, 0.5f}, sf::milliseconds(23));
			pair.second.append(
				{30, 5, 10, 5}, {1.f, 0.5f}, sf::milliseconds(12));
			pair.second.refresh();
		}
		demo_template.torso[core::AnimationAction::Melee].frames.clear();
		demo_template.torso[core::AnimationAction::Melee].refresh();
	}

	void reset() {
		// remove components
		for (auto id : ids) {
			animation_manager.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		animation_manager.cleanup();
		// clear event stuff
		animation_sender.clear();
	}

	core::ObjectID add_object() {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& data = animation_manager.acquire(id);
		data.tpl.legs[core::SpriteLegLayer::Base] = &demo_template.legs;
		data.tpl.torso[core::SpriteTorsoLayer::Base] = &demo_template.torso;
		return id;
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(animation_test)

BOOST_AUTO_TEST_CASE(can_trigger_action) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.torso.elapsed = sf::milliseconds(20);
	data.torso.index = 1u;

	// trigger action
	core::animation_impl::trigger(
		fix.context, data, core::AnimationAction::Use);

	BOOST_CHECK(data.current == core::AnimationAction::Use);
	BOOST_CHECK(data.torso.elapsed == sf::Time::Zero);
	BOOST_CHECK_EQUAL(data.torso.index, 0u);
	BOOST_CHECK(!data.is_moving);
}

BOOST_AUTO_TEST_CASE(can_start_movement_without_state_reset) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.legs.elapsed = sf::milliseconds(20);
	data.legs.index = 1u;

	// trigger movement
	core::animation_impl::trigger(fix.context, data, true);

	BOOST_CHECK(data.is_moving);
	BOOST_CHECK_TIME_EQUAL(data.legs.elapsed, sf::milliseconds(20));
	BOOST_CHECK_EQUAL(data.legs.index, 1u);
	BOOST_CHECK(data.current == core::AnimationAction::Idle);
}

BOOST_AUTO_TEST_CASE(can_stop_movement_without_state_reset) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.is_moving = true;
	data.legs.elapsed = sf::milliseconds(20);
	data.legs.index = 1u;

	// trigger movement
	core::animation_impl::trigger(fix.context, data, false);

	BOOST_CHECK(!data.is_moving);
	BOOST_CHECK_TIME_EQUAL(data.legs.elapsed, sf::milliseconds(20));
	BOOST_CHECK_EQUAL(data.legs.index, 1u);
	BOOST_CHECK(data.current == core::AnimationAction::Idle);
}

BOOST_AUTO_TEST_CASE(flying_object_never_stops_moving) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.is_moving = true;
	data.flying = true;
	data.legs.elapsed = sf::milliseconds(20);
	data.legs.index = 1u;

	// trigger movement
	core::animation_impl::trigger(fix.context, data, false);

	BOOST_CHECK(data.is_moving);
}

BOOST_AUTO_TEST_CASE(flying_object_stops_moving_if_forced) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.is_moving = true;
	data.flying = true;
	data.legs.elapsed = sf::milliseconds(20);
	data.legs.index = 1u;

	// trigger movement (forced, e.g. on death)
	core::animation_impl::trigger(fix.context, data, false, true);

	BOOST_CHECK(!data.is_moving);
}

BOOST_AUTO_TEST_CASE(can_start_interval_animation) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	utils::IntervalState args{15.f};
	args.min = 1.f;
	args.max = 200.f;
	args.speed = 10.f;
	args.rise = true;
	args.repeat = -1;

	// trigger interval
	core::animation_impl::trigger(fix.context, data.brightness, args);
	BOOST_CHECK_CLOSE(data.brightness.current, args.current, 0.0001f);
	BOOST_CHECK_CLOSE(data.brightness.min, args.min, 0.0001f);
	BOOST_CHECK_CLOSE(data.brightness.max, args.max, 0.0001f);
	BOOST_CHECK_CLOSE(data.brightness.speed, args.speed, 0.0001f);
	BOOST_CHECK_EQUAL(data.brightness.repeat, args.repeat);
}

BOOST_AUTO_TEST_CASE(can_stop_interval_animation) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	utils::IntervalState args{0.f};
	args.repeat = 0;

	// trigger interval
	core::animation_impl::trigger(fix.context, data.brightness, args);
	BOOST_CHECK_EQUAL(data.brightness.repeat, 0);
}

BOOST_AUTO_TEST_CASE(can_animate_interval) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	utils::IntervalState args{50.f};
	args.min = 1.f;
	args.max = 200.f;
	args.speed = 5.f;
	args.rise = true;
	args.repeat = -1;

	// trigger interval and update
	core::animation_impl::trigger(fix.context, data.min_saturation, args);
	BOOST_REQUIRE_CLOSE(data.min_saturation.current, 50.f, 0.0001f);
	core::animation_impl::update(fix.context, data, sf::milliseconds(10));

	// assert current interval to be changed
	BOOST_REQUIRE_CLOSE(data.min_saturation.current, 100.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(too_little_update_duration_doesnt_change_dirtyflag) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);

	// trigger action
	core::animation_impl::trigger(
		fix.context, data, core::AnimationAction::Range);
	core::animation_impl::trigger(fix.context, data, true);
	BOOST_REQUIRE(data.current == core::AnimationAction::Range);
	BOOST_REQUIRE(data.is_moving);

	// update too little
	data.has_changed = false;
	core::animation_impl::update(fix.context, data, sf::milliseconds(5));
	BOOST_CHECK(!data.has_changed);
	BOOST_CHECK(data.current == core::AnimationAction::Range);
	BOOST_REQUIRE(data.is_moving);
}

BOOST_AUTO_TEST_CASE(too_little_update_duration_doesnt_reset_dirtyflag) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.has_changed = true;

	// trigger action
	core::animation_impl::trigger(
		fix.context, data, core::AnimationAction::Range);
	core::animation_impl::trigger(fix.context, data, true);
	BOOST_REQUIRE(data.current == core::AnimationAction::Range);
	BOOST_REQUIRE(data.is_moving);

	// update too little
	core::animation_impl::update(fix.context, data, sf::milliseconds(5));
	BOOST_CHECK(data.has_changed);
	BOOST_CHECK(data.current == core::AnimationAction::Range);
	BOOST_REQUIRE(data.is_moving);
}

BOOST_AUTO_TEST_CASE(suitable_update_duration_does_change_dirtyflag) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);

	// trigger action
	core::animation_impl::trigger(
		fix.context, data, core::AnimationAction::Range);
	core::animation_impl::trigger(fix.context, data, true);
	BOOST_REQUIRE(data.current == core::AnimationAction::Range);
	BOOST_REQUIRE(data.is_moving);

	data.legs.elapsed = sf::milliseconds(5);
	data.legs.index = 1u;

	// update with suitable duration
	core::animation_impl::update(fix.context, data, sf::milliseconds(20));
	BOOST_REQUIRE(data.has_changed);
	BOOST_REQUIRE(data.current == core::AnimationAction::Range);
	BOOST_REQUIRE(data.is_moving);
	BOOST_CHECK(data.torso.elapsed == sf::milliseconds(5));
	BOOST_CHECK_EQUAL(data.torso.index, 1u);
	BOOST_CHECK(data.legs.elapsed == sf::milliseconds(8));
	BOOST_CHECK_EQUAL(data.legs.index, 2u);
}

BOOST_AUTO_TEST_CASE(very_long_update_duration_can_reset_action) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);

	// trigger action
	core::animation_impl::trigger(
		fix.context, data, core::AnimationAction::Range);
	core::animation_impl::trigger(fix.context, data, true);
	BOOST_REQUIRE(data.current == core::AnimationAction::Range);
	BOOST_REQUIRE(data.is_moving);

	data.legs.elapsed = sf::milliseconds(5);
	data.legs.index = 1u;

	// update with suitable duration
	core::animation_impl::update(fix.context, data, sf::milliseconds(2000));
	BOOST_REQUIRE(data.has_changed);
	BOOST_REQUIRE(data.current == core::AnimationAction::Idle);
	BOOST_REQUIRE(data.is_moving);
}

BOOST_AUTO_TEST_CASE(cannot_update_without_torsoBase_template) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.tpl.torso[core::SpriteTorsoLayer::Base] = nullptr;
	BOOST_REQUIRE_ASSERT(
		core::animation_impl::update(fix.context, data, sf::milliseconds(20)));
}

BOOST_AUTO_TEST_CASE(cannot_query_action_duration_without_torsoBase) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto action = core::AnimationAction::Range;
	auto& data = fix.animation_manager.query(id);
	data.tpl.torso[core::SpriteTorsoLayer::Base] = nullptr;
	BOOST_REQUIRE_ASSERT(core::getDuration(data, action));
}

BOOST_AUTO_TEST_CASE(action_duration_equals_duration_of_torsoBases_frames) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto action = core::AnimationAction::Range;
	auto& data = fix.animation_manager.query(id);
	BOOST_CHECK_TIME_EQUAL(core::getDuration(data, action),
		(*data.tpl.torso[core::SpriteTorsoLayer::Base])[action].duration);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(trigger_action_forwards_animation_event) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	core::animation_impl::trigger(
		fix.context, data, core::AnimationAction::Range);
	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Range);
}

BOOST_AUTO_TEST_CASE(animation_event_is_send_on_action_finished) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.current = core::AnimationAction::Range;
	core::animation_impl::onActionFinished(fix.context, data);
	BOOST_CHECK(data.current == core::AnimationAction::Idle);
	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Idle);
}

BOOST_AUTO_TEST_CASE(animation_event_is_not_send_on_idle_finished) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.current = core::AnimationAction::Idle;
	core::animation_impl::onActionFinished(fix.context, data);
	BOOST_CHECK(data.current == core::AnimationAction::Idle);
	BOOST_REQUIRE(fix.context.animation_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(animation_is_reset_on_idle_finished) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.current = core::AnimationAction::Idle;
	data.torso.index = 5u;
	core::animation_impl::onActionFinished(fix.context, data);
	BOOST_CHECK(data.current == core::AnimationAction::Idle);
	BOOST_CHECK_EQUAL(data.torso.index, 0u);
	BOOST_REQUIRE(fix.context.animation_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(animation_event_is_not_send_on_death_finished) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.current = core::AnimationAction::Die;
	core::animation_impl::onActionFinished(fix.context, data);
	BOOST_CHECK(data.current == core::AnimationAction::Die);
	BOOST_REQUIRE(fix.context.animation_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(animation_event_not_reset_on_death_finished) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.current = core::AnimationAction::Die;
	data.torso.index = 3u;  // last frame
	core::animation_impl::onActionFinished(fix.context, data);
	BOOST_CHECK(data.current == core::AnimationAction::Die);
	BOOST_CHECK_EQUAL(data.torso.index, 3u);
	BOOST_REQUIRE(fix.context.animation_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(movement_is_reset_on_death_finished) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.current = core::AnimationAction::Die;
	data.is_moving = true;
	core::animation_impl::onActionFinished(fix.context, data);
	BOOST_CHECK(data.current == core::AnimationAction::Die);
	BOOST_CHECK(!data.is_moving);
	BOOST_REQUIRE(fix.context.animation_sender.data().empty());
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(event_can_change_legs_animation) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.legs.index = 100u;
	BOOST_REQUIRE(data.tpl.legs[core::SpriteLegLayer::Armor] == nullptr);
	core::animation_impl::trigger(fix.context, data,
		core::SpriteLegLayer::Armor, &fix.demo_template.legs);
	BOOST_CHECK_EQUAL(data.tpl.legs[core::SpriteLegLayer::Armor], &fix.demo_template.legs);
	BOOST_CHECK(data.legs.index < fix.demo_template.legs.frames.size());
}

BOOST_AUTO_TEST_CASE(event_can_change_torso_animation) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.torso.index = 100u;
	BOOST_REQUIRE(data.tpl.torso[core::SpriteTorsoLayer::Armor] == nullptr);
	core::animation_impl::trigger(fix.context, data,
		core::SpriteTorsoLayer::Armor, &fix.demo_template.torso);
	BOOST_CHECK_EQUAL(data.tpl.torso[core::SpriteTorsoLayer::Armor],
		&fix.demo_template.torso);
	BOOST_CHECK(data.torso.index < fix.demo_template.torso[data.current].frames.size());
}

BOOST_AUTO_TEST_CASE(event_can_change_legs_animation_to_null) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.legs.index = 100u;
	BOOST_REQUIRE(data.tpl.legs[core::SpriteLegLayer::Armor] == nullptr);
	core::animation_impl::trigger(fix.context, data, core::SpriteLegLayer::Armor, nullptr);
	BOOST_CHECK(data.tpl.legs[core::SpriteLegLayer::Armor] == nullptr);
	BOOST_CHECK_EQUAL(data.legs.index, 0u);
}

BOOST_AUTO_TEST_CASE(event_can_change_torso_animation_to_null) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	data.torso.index = 100u;
	BOOST_REQUIRE(data.tpl.torso[core::SpriteTorsoLayer::Armor] == nullptr);
	core::animation_impl::trigger(fix.context, data, core::SpriteTorsoLayer::Armor, nullptr);
	BOOST_CHECK(data.tpl.torso[core::SpriteTorsoLayer::Armor] == nullptr);
	BOOST_CHECK_EQUAL(data.torso.index, 0u);
}

BOOST_AUTO_TEST_CASE(event_cannot_change_torso_base_to_null) {
	auto& fix = Singleton<AnimationFixture>::get();
	fix.reset();

	auto id = fix.add_object();
	auto& data = fix.animation_manager.query(id);
	BOOST_CHECK_ASSERT(core::animation_impl::trigger(
		fix.context, data, core::SpriteTorsoLayer::Base, nullptr));
	BOOST_CHECK_EQUAL(
		data.tpl.torso[core::SpriteTorsoLayer::Base], &fix.demo_template.torso);
}

BOOST_AUTO_TEST_SUITE_END()
