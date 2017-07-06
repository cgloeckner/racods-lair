#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/balance.hpp>
#include <rpg/player.hpp>

struct PlayerFixture {
	core::LogContext log;
	core::IdManager ids;
	std::vector<core::ObjectID> objects;

	rpg::ExpSender exp_sender;
	rpg::TrainingSender training_sender;
	rpg::FeedbackSender feedback_sender;
	rpg::PlayerManager player;
	rpg::StatsManager stats;

	rpg::player_impl::Context context;
	rpg::PerkTemplate perk;

	PlayerFixture()
		: log{}
		, ids{}
		, objects{}
		, exp_sender{}
		, training_sender{}
		, player{}
		, stats{}
		, context{log, exp_sender, training_sender, feedback_sender, player,
			  stats}
		, perk{} {
		//log.debug.add(std::cout);
	}

	std::pair<rpg::PlayerData*, rpg::StatsData*> addPlayer() {
		auto id = ids.acquire();
		objects.push_back(id);
		auto& p = player.acquire(id);
		p.player_id = id;
		auto& s = stats.acquire(id);
		s.stats[rpg::Stat::Life] = 100;
		return {&p, &s};
	}

	void reset() {
		for (auto id : objects) {
			player.release(id);
			stats.release(id);
		}
		objects.clear();
		ids.reset();
		player.cleanup();
		stats.cleanup();

		exp_sender.clear();
		training_sender.clear();
		feedback_sender.clear();
	}
};

BOOST_AUTO_TEST_SUITE(player_test)

BOOST_AUTO_TEST_CASE(gainExp_increases_experience) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->exp = 60;
	rpg::player_impl::gainExp(fix.context, *pair.first, 50u);

	BOOST_CHECK_EQUAL(pair.first->exp, 110u);
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

BOOST_AUTO_TEST_CASE(gainExp_unstacks_some_experience) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->stacked_exp = 30u;
	rpg::player_impl::gainExp(fix.context, *pair.first, 40u);

	BOOST_CHECK_EQUAL(pair.first->exp, 70u);
	BOOST_CHECK_EQUAL(pair.first->stacked_exp, 0u);
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

BOOST_AUTO_TEST_CASE(gainExp_unstacks_not_more_exp_than_gained) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->stacked_exp = 100u;
	rpg::player_impl::gainExp(fix.context, *pair.first, 40u);

	BOOST_CHECK_EQUAL(pair.first->exp, 80u);
	BOOST_CHECK_EQUAL(pair.first->stacked_exp, 60u);
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

BOOST_AUTO_TEST_CASE(gainExp_can_cause_levelup) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	rpg::player_impl::gainExp(fix.context, *pair.first, 100u);

	BOOST_CHECK_EQUAL(pair.first->exp, 100u);
	BOOST_CHECK_EQUAL(pair.first->perk_points, 1u);
	BOOST_CHECK_EQUAL(pair.first->attrib_points, 5u);
	
	auto const& events = fix.exp_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].levelup, 1);
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

BOOST_AUTO_TEST_CASE(gainExp_can_cause_multiple_levelups) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	rpg::player_impl::gainExp(fix.context, *pair.first, 1000u);

	auto const& events = fix.exp_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, pair.first->id);
	BOOST_CHECK_GE(events[0].levelup, 1);
	
	BOOST_CHECK_EQUAL(pair.first->exp, 1000u);
	BOOST_CHECK_EQUAL(pair.first->perk_points, 1u * events[0].levelup);
	BOOST_CHECK_EQUAL(pair.first->attrib_points, 5u * events[0].levelup);
	
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

BOOST_AUTO_TEST_CASE(gainExp_can_cause_levelup_by_unstacking) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->stacked_exp = 50u;
	rpg::player_impl::gainExp(fix.context, *pair.first, 90u);

	BOOST_CHECK_EQUAL(pair.first->exp, 140u);
	BOOST_CHECK_EQUAL(pair.first->perk_points, 1u);
	BOOST_CHECK_EQUAL(pair.first->attrib_points, 5u);
	
	auto const& events = fix.exp_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].levelup, 1);
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

BOOST_AUTO_TEST_CASE(gain_exp_forwards_exp_event_with_total_exp_gain) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->stacked_exp = 20u;
	rpg::player_impl::gainExp(fix.context, *pair.first, 50u);

	auto const& events = fix.exp_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, pair.first->id);
	BOOST_CHECK_EQUAL(events[0].exp, 70u);
	BOOST_CHECK_EQUAL(events[0].levelup, 0u);
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

BOOST_AUTO_TEST_CASE(gain_exp_forwards_exp_event_with_levelup_flag) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->stacked_exp = 50u;
	rpg::player_impl::gainExp(fix.context, *pair.first, 100u);

	auto const& events = fix.exp_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, pair.first->id);
	BOOST_CHECK_EQUAL(events[0].exp, 150u);
	BOOST_CHECK_EQUAL(events[0].levelup, 1u);
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(levelup_refreshs_required_exp_for_next_levelup) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	rpg::player_impl::gainExp(fix.context, *pair.first, 100u);

	BOOST_CHECK_EQUAL(pair.first->base_exp, 100u);
	BOOST_CHECK_EQUAL(pair.first->next_exp, 400u);
	
	auto const& events = fix.exp_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, pair.first->id);
	BOOST_CHECK_EQUAL(events[0].exp, 100u);
	BOOST_CHECK_EQUAL(events[0].levelup, 1u);
	
	BOOST_CHECK_LE(pair.first->base_exp, pair.first->exp);
	BOOST_CHECK_LE(pair.first->exp, pair.first->next_exp);
}

BOOST_AUTO_TEST_CASE(levelup_adds_new_attribute_points) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	rpg::player_impl::gainExp(fix.context, *pair.first, 100u);

	BOOST_CHECK_EQUAL(pair.first->attrib_points, rpg::ATTRIB_POINTS_PER_LEVEL);
}

BOOST_AUTO_TEST_CASE(levelup_adds_new_perk_points) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	rpg::player_impl::gainExp(fix.context, *pair.first, 100u);

	BOOST_CHECK_EQUAL(pair.first->perk_points, rpg::PERK_POINTS_PER_LEVEL);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(stackExp_stacks_exp) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	auto other = fix.addPlayer();
	rpg::player_impl::stackExp(fix.context, *pair.second, *other.first, 120u);

	BOOST_CHECK_EQUAL(other.first->exp, 0u);
	BOOST_CHECK_EQUAL(other.first->stacked_exp, 120u);
}

BOOST_AUTO_TEST_CASE(stackExp_never_causes_levelup) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	auto other = fix.addPlayer();
	rpg::player_impl::stackExp(fix.context, *pair.second, *other.first, 9999u);

	BOOST_CHECK_EQUAL(other.first->exp, 0u);
	BOOST_CHECK_EQUAL(other.first->stacked_exp, 9999u);
}

BOOST_AUTO_TEST_CASE(stackExp_doesnt_work_if_target_dead) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	auto other = fix.addPlayer();
	other.second->stats[rpg::Stat::Life] = 0;
	rpg::player_impl::stackExp(fix.context, *pair.second, *other.first, 120u);

	BOOST_CHECK_EQUAL(other.first->exp, 0u);
	BOOST_CHECK_EQUAL(other.first->stacked_exp, 0u);
}

BOOST_AUTO_TEST_CASE(stackExp_reduces_exp_if_level_distance_is_too_large) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.second->level = 15u;
	pair.first->next_exp = rpg::getNextExp(pair.second->level+1u);
	auto other = fix.addPlayer();
	rpg::player_impl::stackExp(fix.context, *pair.second, *other.first, 150u);

	BOOST_CHECK_EQUAL(other.first->exp, 0u);
	BOOST_CHECK_EQUAL(other.first->stacked_exp, 67u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(onExp_gains_and_shares_exp) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.second->level = 15u;
	pair.first->next_exp = rpg::getNextExp(pair.second->level+1u);
	auto other = fix.addPlayer();
	rpg::player_impl::onExp(fix.context, *pair.first, 150u);

	BOOST_CHECK_EQUAL(pair.first->exp, 150u);
	BOOST_CHECK_EQUAL(pair.first->stacked_exp, 0u);
	BOOST_CHECK_EQUAL(other.first->exp, 0u);
	BOOST_CHECK_EQUAL(other.first->stacked_exp, 67u);
}

BOOST_AUTO_TEST_CASE(onExp_does_nothing_if_actor_is_dead) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.second->level = 15u;
	pair.first->next_exp = rpg::getNextExp(pair.second->level+1u);
	pair.second->stats[rpg::Stat::Life] = 0;
	auto other = fix.addPlayer();
	rpg::player_impl::onExp(fix.context, *pair.first, 150u);

	BOOST_CHECK_EQUAL(pair.first->exp, 0u);
	BOOST_CHECK_EQUAL(pair.first->stacked_exp, 0u);
	BOOST_CHECK_EQUAL(other.first->exp, 0u);
	BOOST_CHECK_EQUAL(other.first->stacked_exp, 0u);
}

BOOST_AUTO_TEST_CASE(onExp_shares_only_with_living_allies) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.second->level = 15u;
	pair.first->next_exp = rpg::getNextExp(pair.second->level+1u);
	auto bob = fix.addPlayer();
	auto eddy = fix.addPlayer();
	eddy.second->stats[rpg::Stat::Life] = 0;
	auto carl = fix.addPlayer();
	rpg::player_impl::onExp(fix.context, *pair.first, 150u);

	BOOST_CHECK_EQUAL(pair.first->exp, 150u);
	BOOST_CHECK_EQUAL(bob.first->stacked_exp, 67u);
	BOOST_CHECK_EQUAL(eddy.first->stacked_exp, 0u);
	BOOST_CHECK_EQUAL(carl.first->stacked_exp, 67u);
}

BOOST_AUTO_TEST_CASE(onExp_shares_considers_everybodys_level) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.second->level = 15u;
	pair.first->next_exp = rpg::getNextExp(pair.second->level+1u);
	auto bob = fix.addPlayer();
	auto eddy = fix.addPlayer();
	eddy.second->level = 30u;
	eddy.first->next_exp = rpg::getNextExp(eddy.second->level+1u);
	auto carl = fix.addPlayer();
	carl.second->level = 13u;
	carl.first->next_exp = rpg::getNextExp(carl.second->level+1u);
	rpg::player_impl::onExp(fix.context, *pair.first, 150u);

	BOOST_CHECK_EQUAL(pair.first->exp, 150u);
	BOOST_CHECK_EQUAL(bob.first->stacked_exp, 67u);
	BOOST_CHECK_EQUAL(eddy.first->stacked_exp, 67u);
	BOOST_CHECK_EQUAL(carl.first->stacked_exp, 150u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(can_train_attribute_if_player_has_attrib_points) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->attrib_points = 1u;

	rpg::TrainingEvent event;
	event.actor = pair.first->id;
	event.type = rpg::TrainingEvent::Attrib;
	event.attrib = rpg::Attribute::Strength;
	rpg::player_impl::onTraining(fix.context, *pair.first, event);

	BOOST_CHECK_EQUAL(pair.first->attrib_points, 0u);
	auto const& events = fix.training_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, pair.first->id);
	BOOST_CHECK(events[0].type == rpg::TrainingEvent::Attrib);
	BOOST_CHECK(events[0].attrib == rpg::Attribute::Strength);
}

BOOST_AUTO_TEST_CASE(cannot_train_attribute_if_player_has_no_attrib_points) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->attrib_points = 0u;

	rpg::TrainingEvent event;
	event.actor = pair.first->id;
	event.type = rpg::TrainingEvent::Attrib;
	event.attrib = rpg::Attribute::Strength;
	rpg::player_impl::onTraining(fix.context, *pair.first, event);

	BOOST_CHECK_EQUAL(pair.first->attrib_points, 0u);
	BOOST_CHECK_EQUAL(fix.training_sender.data().size(), 0u);

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, pair.first->id);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::NotEnoughAttribPoints);
}

BOOST_AUTO_TEST_CASE(can_train_perk_if_player_has_perk_points) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->perk_points = 1u;

	rpg::TrainingEvent event;
	event.actor = pair.first->id;
	event.type = rpg::TrainingEvent::Perk;
	event.perk = &fix.perk;
	rpg::player_impl::onTraining(fix.context, *pair.first, event);

	BOOST_CHECK_EQUAL(pair.first->perk_points, 0u);
	auto const& events = fix.training_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, pair.first->id);
	BOOST_CHECK(events[0].type == rpg::TrainingEvent::Perk);
	BOOST_CHECK_EQUAL(events[0].perk, &fix.perk);
}

BOOST_AUTO_TEST_CASE(cannot_train_perk_if_player_has_no_attrib_points) {
	auto& fix = Singleton<PlayerFixture>::get();
	fix.reset();

	auto pair = fix.addPlayer();
	pair.first->perk_points = 0u;

	rpg::TrainingEvent event;
	event.actor = pair.first->id;
	event.type = rpg::TrainingEvent::Perk;
	event.perk = &fix.perk;
	rpg::player_impl::onTraining(fix.context, *pair.first, event);

	BOOST_CHECK_EQUAL(pair.first->perk_points, 0u);
	BOOST_CHECK_EQUAL(fix.training_sender.data().size(), 0u);

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, pair.first->id);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::NotEnoughPerkPoints);
}

BOOST_AUTO_TEST_SUITE_END()
