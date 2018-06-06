#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/action.hpp>

struct ActionFixture {
	core::LogContext log;
	core::InputSender input_sender;
	core::AnimationSender animation_sender;
	rpg::ActionSender action_sender;

	rpg::ActionManager action;

	rpg::action_impl::Context context;

	rpg::ActionData& actor;

	ActionFixture()
		: log{}
		, input_sender{}
		, animation_sender{}
		, action{}
		, context{log, input_sender, animation_sender, action_sender}
		, actor{action.acquire(1u)} {}

	void reset() {
		actor.idle = true;
		actor.dead = false;
		// reset event senders
		input_sender.clear();
		animation_sender.clear();
		action_sender.clear();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}
};

BOOST_AUTO_TEST_SUITE(action_test)

BOOST_AUTO_TEST_CASE(look_is_forwarded_if_when_idling) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.idle = true;
	core::InputEvent event;
	event.actor = 1u;
	event.move = {-1, 0};
	event.look = {1, 0};
	rpg::action_impl::onInput(fix.context, fix.actor, event);

	auto const& events = fix.context.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_VECTOR_EQUAL(events[0].move, sf::Vector2i(-1, 0));
	BOOST_CHECK_VECTOR_EQUAL(events[0].look, sf::Vector2i(1, 0));
}

BOOST_AUTO_TEST_CASE(cannot_spam_gameplay_action) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	rpg::ActionEvent event;
	event.action = rpg::PlayerAction::Attack;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	rpg::action_impl::onAction(fix.context, fix.actor, event);

	auto const& events = fix.context.action_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
}

BOOST_AUTO_TEST_CASE(can_spam_quickslot_switch_actions) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	rpg::ActionEvent event;
	event.actor = 1u;
	event.action = rpg::PlayerAction::NextSlot;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	event.action = rpg::PlayerAction::PrevSlot;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	event.action = rpg::PlayerAction::PrevSlot;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	event.action = rpg::PlayerAction::NextSlot;
	rpg::action_impl::onAction(fix.context, fix.actor, event);

	auto const& events = fix.context.action_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 4u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(actor_plays_dying_animation_on_death) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	rpg::DeathEvent event;
	rpg::action_impl::onDeath(fix.context, fix.actor, event);

	BOOST_CHECK(fix.actor.dead);
	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Die);
}

BOOST_AUTO_TEST_CASE(actor_stops_movement_on_death) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	rpg::DeathEvent event;
	rpg::action_impl::onDeath(fix.context, fix.actor, event);

	BOOST_CHECK(fix.actor.dead);
	auto const& events = fix.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_VECTOR_CLOSE(events[0].move, sf::Vector2f(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(actor_can_respawn) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	rpg::SpawnEvent event;
	fix.actor.dead = true;
	rpg::action_impl::onSpawn(fix.context, fix.actor, event);

	BOOST_CHECK(!fix.actor.dead);
}

BOOST_AUTO_TEST_CASE(respawn_triggers_idle_animation) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	rpg::SpawnEvent event;
	fix.actor.dead = true;
	rpg::action_impl::onSpawn(fix.context, fix.actor, event);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Idle);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cannot_propagate_move_if_dead) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.dead = true;
	core::InputEvent event;
	event.actor = fix.actor.id;
	event.move.x = 1;
	rpg::action_impl::onInput(fix.context, fix.actor, event);
	BOOST_CHECK(fix.context.input_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(cannot_propagate_looking_if_dead) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.dead = true;
	core::InputEvent event;
	event.actor = fix.actor.id;
	event.look.x = 1;
	rpg::action_impl::onInput(fix.context, fix.actor, event);
	BOOST_CHECK(fix.context.input_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(can_propagate_stop_if_dead) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.dead = true;
	core::InputEvent event;
	event.actor = fix.actor.id;
	rpg::action_impl::onInput(fix.context, fix.actor, event);
	auto const& events = fix.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK_VECTOR_EQUAL(events[0].move, sf::Vector2i());
	BOOST_CHECK_VECTOR_EQUAL(events[0].look, sf::Vector2i());
}

BOOST_AUTO_TEST_CASE(cannot_propagate_action_if_dead) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.dead = true;
	rpg::ActionEvent event;
	event.actor = fix.actor.id;
	event.action = rpg::PlayerAction::UseSlot;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	BOOST_CHECK(fix.context.action_sender.data().empty());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(can_pause_if_dead) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.dead = true;
	rpg::ActionEvent event;
	event.actor = fix.actor.id;
	event.action = rpg::PlayerAction::Pause;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	auto const & events = fix.context.action_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK(events[0].action == rpg::PlayerAction::Pause);
}

BOOST_AUTO_TEST_CASE(can_pause_if_alive) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.dead = false;
	rpg::ActionEvent event;
	event.actor = fix.actor.id;
	event.action = rpg::PlayerAction::Pause;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	BOOST_CHECK_EQUAL(fix.context.action_sender.data().size(), 1u);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cannot_interact_if_already_acting) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.idle = false;
	rpg::ActionEvent event;
	event.actor = fix.actor.id;
	event.action = rpg::PlayerAction::Interact;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	BOOST_CHECK(fix.context.action_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(cannot_attack_if_already_acting) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.idle = false;
	rpg::ActionEvent event;
	event.actor = fix.actor.id;
	event.action = rpg::PlayerAction::Attack;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	BOOST_CHECK(fix.context.action_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(cannot_quickuse_if_already_acting) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.idle = false;
	rpg::ActionEvent event;
	event.actor = fix.actor.id;
	event.action = rpg::PlayerAction::UseSlot;
	rpg::action_impl::onAction(fix.context, fix.actor, event);
	BOOST_CHECK(fix.context.action_sender.data().empty());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(feedback_resets_action_idle_if_alive) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.idle = false;
	rpg::FeedbackEvent event;
	event.actor = fix.actor.id;
	event.type = rpg::FeedbackType::NotEnoughMana;
	rpg::action_impl::onFeedback(fix.context, fix.actor, event);
	BOOST_CHECK(fix.actor.idle);
}

BOOST_AUTO_TEST_CASE(feedback_does_not_reset_action_idle_if_dead) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.idle = false;
	fix.actor.dead = true;
	rpg::FeedbackEvent event;
	event.actor = fix.actor.id;
	event.type = rpg::FeedbackType::NotEnoughMana;
	rpg::action_impl::onFeedback(fix.context, fix.actor, event);
	BOOST_CHECK(!fix.actor.idle);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(animation_event_can_start_idle) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.idle = false;
	fix.actor.dead = false;
	
	core::AnimationEvent event;
	event.type   = core::AnimationEvent::Action;
	event.action = core::AnimationAction::Idle;
	rpg::action_impl::onAnimation(fix.context, fix.actor, event);
	
	BOOST_CHECK(fix.actor.idle);
}

BOOST_AUTO_TEST_CASE(animation_event_can_stop_idle) {
	auto& fix = Singleton<ActionFixture>::get();
	fix.reset();

	fix.actor.idle = true;
	fix.actor.dead = false;
	
	core::AnimationEvent event;
	event.type   = core::AnimationEvent::Action;
	event.action = core::AnimationAction::Melee;
	rpg::action_impl::onAnimation(fix.context, fix.actor, event);
	
	BOOST_CHECK(!fix.actor.idle);
}

BOOST_AUTO_TEST_SUITE_END()
