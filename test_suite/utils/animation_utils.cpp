#include <boost/test/unit_test.hpp>

#include <utils/assert.hpp>
#include <utils/animation_utils.hpp>

void build_demo_frames(utils::ActionFrames& frames) {
	frames.append({0, 0, 10, 5}, {1.f, 0.5f}, sf::milliseconds(15));
	frames.append({10, 0, 10, 5}, {1.f, 0.5f}, sf::milliseconds(17));
	frames.append({20, 0, 10, 5}, {1.f, 0.5f}, sf::milliseconds(23));
	frames.append({30, 0, 10, 5}, {1.f, 0.5f}, sf::milliseconds(12));
	frames.refresh();
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(AnimationUtils_test)

BOOST_AUTO_TEST_CASE(
	AnimationUtils_refresh_ActionFrames_will_sum_up_durations) {
	utils::ActionFrames frames;
	build_demo_frames(frames);

	BOOST_CHECK_EQUAL(67, frames.duration.asMilliseconds());
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_too_short_updateActionState_will_not_change_index_but_time) {
	utils::ActionState state;
	utils::ActionFrames frames;
	build_demo_frames(frames);

	BOOST_CHECK_EQUAL(0u, state.index);
	BOOST_CHECK_EQUAL(0, state.elapsed.asMilliseconds());

	bool updated;
	bool finished =
		utils::updateActionState(state, frames, sf::milliseconds(5), updated);
	BOOST_CHECK(!finished);
	BOOST_CHECK(!updated);
	BOOST_CHECK_EQUAL(0u, state.index);
	BOOST_CHECK_EQUAL(5, state.elapsed.asMilliseconds());
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_normal_updateActionState_will_change_index_and_time) {
	utils::ActionState state;
	utils::ActionFrames frames;
	build_demo_frames(frames);

	BOOST_CHECK_EQUAL(0u, state.index);
	BOOST_CHECK_EQUAL(0, state.elapsed.asMilliseconds());

	bool updated;
	bool finished =
		utils::updateActionState(state, frames, sf::milliseconds(20), updated);
	BOOST_CHECK(!finished);
	BOOST_CHECK(updated);
	BOOST_CHECK_EQUAL(1u, state.index);
	BOOST_CHECK_EQUAL(5, state.elapsed.asMilliseconds());
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_large_updateActionState_will_change_index_and_time_multiple_times_but_at_once) {
	utils::ActionState state;
	utils::ActionFrames frames;
	build_demo_frames(frames);

	BOOST_CHECK_EQUAL(0u, state.index);
	BOOST_CHECK_EQUAL(0, state.elapsed.asMilliseconds());

	bool updated;
	bool finished =
		utils::updateActionState(state, frames, sf::milliseconds(40), updated);
	BOOST_CHECK(!finished);
	BOOST_CHECK(updated);
	BOOST_CHECK_EQUAL(2u, state.index);
	BOOST_CHECK_EQUAL(8, state.elapsed.asMilliseconds());
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_too_large_updateActionState_will_change_index_and_time_and_restart) {
	utils::ActionState state;
	utils::ActionFrames frames;
	build_demo_frames(frames);

	BOOST_CHECK_EQUAL(0u, state.index);
	BOOST_CHECK_EQUAL(0, state.elapsed.asMilliseconds());

	bool updated;
	bool finished =
		utils::updateActionState(state, frames, sf::milliseconds(77), updated);
	BOOST_CHECK(finished);
	BOOST_CHECK(updated);
	BOOST_CHECK_EQUAL(0u, state.index);
	BOOST_CHECK_EQUAL(10, state.elapsed.asMilliseconds());
}

BOOST_AUTO_TEST_CASE(AnimationUtils_interval_can_rise_a_little) {
	utils::IntervalState state{14.f};
	state.min = 14.f;
	state.max = 66.5f;
	state.speed = 0.1f;
	state.rise = true;
	state.repeat = -1;

	bool updated;
	bool finished = utils::updateInterval(state, sf::milliseconds(55), updated);
	BOOST_CHECK(!finished);
	BOOST_CHECK(updated);
	BOOST_CHECK_CLOSE(19.5f, state.current, 0.0001f);
	BOOST_CHECK(state.rise);
}

BOOST_AUTO_TEST_CASE(AnimationUtils_interval_can_rise_to_max) {
	utils::IntervalState state{14.f};
	state.min = 14.f;
	state.max = 66.5f;
	state.speed = 0.1f;
	state.rise = true;
	state.repeat = -1;

	bool updated;
	bool finished =
		utils::updateInterval(state, sf::milliseconds(30000), updated);
	BOOST_CHECK(finished);
	BOOST_CHECK(updated);
	BOOST_CHECK_CLOSE(state.max, state.current, 0.0001f);
	BOOST_CHECK(!state.rise);
}

BOOST_AUTO_TEST_CASE(AnimationUtils_interval_can_fall_a_little) {
	utils::IntervalState state{66.5f};
	state.min = 14.f;
	state.max = 66.5f;
	state.speed = 0.1f;
	state.rise = false;
	state.repeat = -1;

	bool updated;
	bool finished = utils::updateInterval(state, sf::milliseconds(73), updated);
	BOOST_CHECK(!finished);
	BOOST_CHECK(updated);
	BOOST_CHECK_CLOSE(59.2f, state.current, 0.0001f);
	BOOST_CHECK(!state.rise);
}

BOOST_AUTO_TEST_CASE(AnimationUtils_interval_can_fall_a_lot) {
	utils::IntervalState state{66.5f};
	state.min = 14.f;
	state.max = 66.5f;
	state.speed = 0.1f;
	state.rise = false;
	state.repeat = -1;

	bool updated;
	bool finished =
		utils::updateInterval(state, sf::milliseconds(30000), updated);
	BOOST_CHECK(finished);
	BOOST_CHECK(updated);
	BOOST_CHECK_CLOSE(state.min, state.current, 0.0001f);
	BOOST_CHECK(state.rise);
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_interval_cannot_be_changed_with_non_positive_speed) {
	utils::IntervalState state{66.5f};
	state.min = 14.f;
	state.max = 66.5f;
	state.speed = 0.f;
	state.repeat = -1;

	bool updated;
	BOOST_CHECK_ASSERT(
		utils::updateInterval(state, sf::milliseconds(5), updated));
	state.speed = -1.f;
	BOOST_CHECK_ASSERT(
		utils::updateInterval(state, sf::milliseconds(5), updated));
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_interval_cannot_be_changed_with_invalid_min_max) {
	utils::IntervalState state{14.f};
	state.min = 14.f;
	state.max = state.min;
	state.speed = 1.f;
	state.repeat = -1;

	bool updated;
	BOOST_CHECK_ASSERT(
		utils::updateInterval(state, sf::milliseconds(5), updated));
}

BOOST_AUTO_TEST_CASE(AnimationUtils_interval_cannot_be_updated_if_disabled) {
	utils::IntervalState state{66.5f};
	state.min = 14.f;
	state.max = 66.5f;
	state.speed = 1.f;
	state.repeat = 0;

	bool updated;
	utils::updateInterval(state, sf::milliseconds(5), updated);
	BOOST_CHECK(!updated);
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_interval_can_only_be_updated_multple_times) {
	utils::IntervalState state{66.5f};
	state.min = 14.f;
	state.max = 66.5f;
	state.speed = 1.f;
	state.repeat = 2;

	bool updated;
	utils::updateInterval(state, sf::seconds(1.f), updated);
	BOOST_CHECK(updated);
	utils::updateInterval(state, sf::seconds(1.f), updated);
	BOOST_CHECK(updated);
	utils::updateInterval(state, sf::seconds(1.f), updated);
	BOOST_CHECK(!updated);
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_interval_update_can_be_splitted_into_chunks) {
	utils::IntervalState state{10.f};
	state.min = 10.f;
	state.max = 20.f;
	state.rise = true;
	state.speed = 2.5f;
	state.repeat = 1;

	bool updated;
	utils::updateInterval(state, sf::milliseconds(2), updated);
	BOOST_CHECK(updated);
	BOOST_CHECK_CLOSE(state.current, 15.f, 0.0001f);
	utils::updateInterval(state, sf::milliseconds(3), updated);
	BOOST_CHECK(updated);
	BOOST_CHECK_CLOSE(state.current, 20.f, 0.0001f);
	utils::updateInterval(state, sf::milliseconds(3), updated);
	BOOST_CHECK(!updated);
}

BOOST_AUTO_TEST_CASE(
	AnimationUtils_interval_update_changes_rise_flag_if_value_exceeded) {
	utils::IntervalState state{10.f};
	state.min = 10.f;
	state.max = 20.f;
	state.rise = true;
	state.speed = 1.f;
	state.repeat = 2;

	bool updated;
	utils::updateInterval(state, sf::milliseconds(10), updated);
	BOOST_CHECK(updated);
	BOOST_CHECK(!state.rise);
	utils::updateInterval(state, sf::milliseconds(10), updated);
	BOOST_CHECK(updated);
	BOOST_CHECK(state.rise);
	utils::updateInterval(state, sf::milliseconds(1), updated);
	BOOST_CHECK(!updated);

	state.repeat = -1;
	utils::updateInterval(state, sf::milliseconds(10), updated);
	BOOST_CHECK(updated);
	BOOST_CHECK(!state.rise);
	utils::updateInterval(state, sf::milliseconds(10), updated);
	BOOST_CHECK(updated);
	BOOST_CHECK(state.rise);
	utils::updateInterval(state, sf::milliseconds(1), updated);
	BOOST_CHECK(updated);
}

BOOST_AUTO_TEST_SUITE_END()
