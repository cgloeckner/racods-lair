#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/quickslot.hpp>

struct QuickslotFixture {
	core::LogContext log;
	rpg::ItemSender item_sender;
	rpg::PerkSender perk_sender;
	rpg::FeedbackSender feedback_sender;

	rpg::quickslot_impl::Context context;
	rpg::ItemTemplate item;
	rpg::PerkTemplate perk;

	rpg::QuickslotData actor;

	QuickslotFixture()
		: log{}
		, item_sender{}
		, perk_sender{}
		, context{log, item_sender, perk_sender, feedback_sender}
		, item{}
		, perk{}
		, actor{} {
		actor.id = 0u;
		actor.slots[0u] = {item};
		actor.slots[2u] = {perk};
	}

	void reset() {
		actor.slot_id = 0u;
		for (auto& slot : actor.slots) {
			slot = rpg::Shortcut{};
		}
		actor.slots[0u] = {item};
		actor.slots[2u] = {perk};
		actor.cooldown = sf::Time::Zero;

		item_sender.clear();
		perk_sender.clear();
		feedback_sender.clear();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}
};

BOOST_AUTO_TEST_SUITE(quickslot_test)

BOOST_AUTO_TEST_CASE(cannot_switch_to_next_slot_if_cooldown_is_active) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.cooldown = sf::milliseconds(150);
	rpg::quickslot_impl::onSwitchSlot(fix.context, fix.actor, true);

	BOOST_CHECK_EQUAL(fix.actor.slot_id, 0u);
}

BOOST_AUTO_TEST_CASE(can_switch_to_next_slot) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::onSwitchSlot(fix.context, fix.actor, true);
	BOOST_CHECK_EQUAL(fix.actor.slot_id, 1u);
}

BOOST_AUTO_TEST_CASE(can_switch_to_next_slot_via_overflow) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.slot_id = rpg::MAX_QUICKSLOTS - 1u;
	rpg::quickslot_impl::onSwitchSlot(fix.context, fix.actor, true);
	BOOST_CHECK_EQUAL(fix.actor.slot_id, 0u);
}

BOOST_AUTO_TEST_CASE(switch_to_next_slots_sets_cooldown) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::onSwitchSlot(fix.context, fix.actor, true);
	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::milliseconds(250u));
}

BOOST_AUTO_TEST_CASE(cannot_switch_to_prev_slot_if_cooldown_is_active) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.cooldown = sf::milliseconds(150);
	fix.actor.slot_id = 2u;
	rpg::quickslot_impl::onSwitchSlot(fix.context, fix.actor, false);

	BOOST_CHECK_EQUAL(fix.actor.slot_id, 2u);
}

BOOST_AUTO_TEST_CASE(can_switch_to_prev_slot) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.slot_id = 5u;
	rpg::quickslot_impl::onSwitchSlot(fix.context, fix.actor, false);
	BOOST_CHECK_EQUAL(fix.actor.slot_id, 4u);
}

BOOST_AUTO_TEST_CASE(can_switch_to_prev_slot_via_underflow) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::onSwitchSlot(fix.context, fix.actor, false);
	BOOST_CHECK_EQUAL(fix.actor.slot_id, rpg::MAX_QUICKSLOTS - 1u);
}

BOOST_AUTO_TEST_CASE(switch_to_prev_slots_sets_cooldown) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::onSwitchSlot(fix.context, fix.actor, false);
	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::milliseconds(250u));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(can_use_slot_if_cooldown_is_active) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.cooldown = sf::milliseconds(50);
	rpg::quickslot_impl::onUseSlot(fix.context, fix.actor);

	BOOST_CHECK(!fix.item_sender.data().empty() || !fix.perk_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(use_empty_sends_only_feedback) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.slot_id = 1u;
	rpg::quickslot_impl::onUseSlot(fix.context, fix.actor);

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::EmptyShortcut);

	BOOST_CHECK(fix.item_sender.data().empty());
	BOOST_CHECK(fix.perk_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(use_slot_can_trigger_item_event) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::onUseSlot(fix.context, fix.actor);

	BOOST_CHECK(fix.perk_sender.data().empty());
	auto const& events = fix.item_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK_EQUAL(events[0].item, &fix.item);
	BOOST_CHECK(events[0].type == rpg::ItemEvent::Use);
}

BOOST_AUTO_TEST_CASE(use_slot_can_trigger_perk_event) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.slot_id = 2u;
	rpg::quickslot_impl::onUseSlot(fix.context, fix.actor);

	BOOST_CHECK(fix.item_sender.data().empty());
	auto const& events = fix.perk_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK_EQUAL(events[0].perk, &fix.perk);
	BOOST_CHECK(events[0].type == rpg::PerkEvent::Use);
}

BOOST_AUTO_TEST_CASE(use_empty_slot_does_not_starts_cooldown) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.slot_id = 1u;
	rpg::quickslot_impl::onUseSlot(fix.context, fix.actor);
	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::Time::Zero);
}

BOOST_AUTO_TEST_CASE(quickuse_item_does_not_start_cooldown) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::onUseSlot(fix.context, fix.actor);
	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::Time::Zero);
}

BOOST_AUTO_TEST_CASE(quickuse_perk_does_not_start_cooldown) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.slot_id = 2u;
	rpg::quickslot_impl::onUseSlot(fix.context, fix.actor);
	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::Time::Zero);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(can_assign_item_to_slot) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::assignSlot(fix.actor, fix.item, 5u);
	BOOST_CHECK_EQUAL(fix.actor.slots[5u].item, &fix.item);
}

BOOST_AUTO_TEST_CASE(can_assign_perk_to_slot) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::assignSlot(fix.actor, fix.perk, 5u);
	BOOST_CHECK_EQUAL(fix.actor.slots[5u].perk, &fix.perk);
}

BOOST_AUTO_TEST_CASE(can_release_item_from_slots) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::assignSlot(fix.actor, fix.item, 2u);
	rpg::quickslot_impl::assignSlot(fix.actor, fix.item, 3u);
	rpg::quickslot_impl::assignSlot(fix.actor, fix.item, 5u);
	rpg::quickslot_impl::assignSlot(fix.actor, fix.item, 8u);

	rpg::quickslot_impl::releaseSlot(fix.actor, fix.item);
	BOOST_CHECK(fix.actor.slots[2u].item == nullptr);
	BOOST_CHECK(fix.actor.slots[3u].item == nullptr);
	BOOST_CHECK(fix.actor.slots[5u].item == nullptr);
	BOOST_CHECK(fix.actor.slots[8u].item == nullptr);
}

BOOST_AUTO_TEST_CASE(can_release_perk_from_slots) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	rpg::quickslot_impl::assignSlot(fix.actor, fix.perk, 1u);
	rpg::quickslot_impl::assignSlot(fix.actor, fix.perk, 3u);
	rpg::quickslot_impl::assignSlot(fix.actor, fix.perk, 4u);
	rpg::quickslot_impl::assignSlot(fix.actor, fix.perk, 7u);

	rpg::quickslot_impl::releaseSlot(fix.actor, fix.perk);
	BOOST_CHECK(fix.actor.slots[1u].perk == nullptr);
	BOOST_CHECK(fix.actor.slots[3u].perk == nullptr);
	BOOST_CHECK(fix.actor.slots[4u].perk == nullptr);
	BOOST_CHECK(fix.actor.slots[7u].perk == nullptr);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cooldown_is_reduced_on_update) {
	auto& fix = Singleton<QuickslotFixture>::get();
	fix.reset();

	fix.actor.cooldown = sf::milliseconds(100);
	rpg::quickslot_impl::onUpdate(
		fix.context, fix.actor, sf::milliseconds(120));
	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::Time::Zero);
}

BOOST_AUTO_TEST_SUITE_END()
