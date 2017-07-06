#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/item.hpp>

struct ItemFixture {
	core::LogContext log;
	core::AnimationSender animation_sender;
	core::SpriteSender sprite_sender;
	rpg::QuickslotSender quickslot_sender;
	rpg::BoniSender boni_sender;
	rpg::StatsSender stats_sender;
	rpg::FeedbackSender feedback_sender;
	rpg::StatsManager stats;

	rpg::item_impl::Context context;
	rpg::ItemData actor;

	sf::Texture weapon_texture, armor_texture, shield_texture, helmet_texture;
	rpg::SpriteTemplate weapon_sprite, armor_sprite, shield_sprite, helmet_sprite;
	rpg::ItemTemplate sword, bow, shield, helmet, armor, potion;

	ItemFixture()
		: log{}
		, animation_sender{}
		, sprite_sender{}
		, quickslot_sender{}
		, boni_sender{}
		, stats_sender{}
		, stats{}
		, context{log, animation_sender, sprite_sender, boni_sender,
			  quickslot_sender, stats_sender, feedback_sender, stats}
		, actor{}
		, weapon_texture{}
		, armor_texture{}
		, shield_texture{}
		, helmet_texture{}
		, weapon_sprite{}
		, armor_sprite{}
		, shield_sprite{}
		, helmet_sprite{}
		, sword{}
		, bow{}
		, shield{}
		, helmet{}
		, armor{}
		, potion{} {
		actor.id = 1u;
		stats.acquire(1u);

		weapon_sprite.frameset = &weapon_texture;
		armor_sprite.frameset = &armor_texture;
		shield_sprite.frameset = &shield_texture;
		helmet_sprite.frameset = &helmet_texture;

		sword.type = rpg::ItemType::Weapon;
		sword.slot = rpg::EquipmentSlot::Weapon;
		sword.sprite = &weapon_sprite;
		bow.type = rpg::ItemType::Weapon;
		bow.slot = rpg::EquipmentSlot::Weapon;
		bow.two_handed = true;
		bow.sprite = &weapon_sprite;
		shield.type = rpg::ItemType::Armor;
		shield.slot = rpg::EquipmentSlot::Extension;
		shield.sprite = &shield_sprite;
		helmet.type = rpg::ItemType::Armor;
		helmet.slot = rpg::EquipmentSlot::Head;
		helmet.sprite = &helmet_sprite;
		helmet.require[rpg::Attribute::Strength] = 10;
		armor.type = rpg::ItemType::Armor;
		armor.slot = rpg::EquipmentSlot::Body;
		armor.sprite = &armor_sprite;
		potion.type = rpg::ItemType::Potion;
		potion.recover[rpg::Stat::Mana] = 20;
	}

	void reset() {
		auto& s = stats.query(actor.id);
		s.attributes[rpg::Attribute::Strength] = 5;
		for (auto& slot : actor.inventory) {
			slot.second.clear();
		}
		for (auto& slot : actor.equipment) {
			slot.second = nullptr;
		}
		animation_sender.clear();
		sprite_sender.clear();
		quickslot_sender.clear();
		boni_sender.clear();
		stats_sender.clear();
		feedback_sender.clear();
	}
};

BOOST_AUTO_TEST_SUITE(item_test)

BOOST_AUTO_TEST_CASE(addItem_creates_new_node_for_new_items) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	auto success = rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_REQUIRE(success);

	auto const& slot = fix.actor.inventory[fix.sword.type];
	BOOST_REQUIRE_EQUAL(slot.size(), 1u);
	BOOST_CHECK(slot[0].item == &fix.sword);
	BOOST_CHECK_EQUAL(slot[0].quantity, 1u);
}

BOOST_AUTO_TEST_CASE(addItem_modifies_existing_node_for_another) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 2u);
	auto success = rpg::item_impl::addItem(fix.actor, fix.sword, 3u);
	BOOST_REQUIRE(success);

	auto const& slot = fix.actor.inventory[fix.sword.type];
	BOOST_REQUIRE_EQUAL(slot.size(), 1u);
	BOOST_CHECK(slot[0].item == &fix.sword);
	BOOST_CHECK_EQUAL(slot[0].quantity, 5u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(removeItem_reduces_quantity_of_item) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.potion, 10u);
	BOOST_CHECK(rpg::item_impl::removeItem(fix.context, fix.actor, fix.potion, 2u));

	auto const& slot = fix.actor.inventory[fix.potion.type];
	BOOST_REQUIRE_EQUAL(slot.size(), 1u);
	BOOST_CHECK(slot[0].item == &fix.potion);
	BOOST_CHECK_EQUAL(slot[0].quantity, 8u);
}

BOOST_AUTO_TEST_CASE(removeItem_can_remove_entire_node) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.potion, 2u);
	BOOST_CHECK(rpg::item_impl::removeItem(fix.context, fix.actor, fix.potion, 2u));

	auto const& slot = fix.actor.inventory[fix.potion.type];
	BOOST_CHECK(slot.empty());
}

BOOST_AUTO_TEST_CASE(cannot_remove_more_items_than_existing) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.potion, 3u);
	BOOST_CHECK(!rpg::item_impl::removeItem(fix.context, fix.actor, fix.potion, 4u));

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::ItemNotFound);
}

BOOST_AUTO_TEST_CASE(remove_equipped_item_resets_equipment_slot) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	fix.actor.equipment[rpg::EquipmentSlot::Weapon] = &fix.sword;
	rpg::item_impl::removeItem(fix.context, fix.actor, fix.sword, 1u);

	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Weapon] == nullptr);
}

BOOST_AUTO_TEST_CASE(remove_doesnt_unequip_if_last_item_is_not_removed) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 2u);
	fix.actor.equipment[rpg::EquipmentSlot::Weapon] = &fix.sword;
	rpg::item_impl::removeItem(fix.context, fix.actor, fix.sword, 1u);

	BOOST_CHECK_EQUAL(
		fix.actor.equipment[rpg::EquipmentSlot::Weapon], &fix.sword);
}

BOOST_AUTO_TEST_CASE(remove_item_creates_quickslot_release_event) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	rpg::item_impl::removeItem(fix.context, fix.actor, fix.sword, 1u);

	auto const& events = fix.quickslot_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].item, &fix.sword);
	BOOST_CHECK(events[0].type == rpg::QuickslotEvent::Release);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cannot_use_missing_potion) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	BOOST_CHECK(!rpg::item_impl::useItem(fix.context, fix.actor, fix.potion));
	BOOST_CHECK(fix.stats_sender.data().empty());

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::ItemNotFound);
}

BOOST_AUTO_TEST_CASE(use_potion_creates_item_event) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.potion, 1u);
	BOOST_CHECK(rpg::item_impl::useItem(fix.context, fix.actor, fix.potion));

	auto const& events = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 20);
}

BOOST_AUTO_TEST_CASE(using_one_of_many_items_doesnt_drop_slot) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.potion, 2u);
	BOOST_CHECK(rpg::item_impl::useItem(fix.context, fix.actor, fix.potion));
	auto const& slot = fix.actor.inventory[fix.potion.type];
	BOOST_CHECK(!slot.empty());
}

BOOST_AUTO_TEST_CASE(using_last_item_will_drop_entire_node) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.potion, 1u);
	BOOST_CHECK(rpg::item_impl::useItem(fix.context, fix.actor, fix.potion));
	auto const& slot = fix.actor.inventory[fix.potion.type];
	BOOST_CHECK(slot.empty());
}

BOOST_AUTO_TEST_CASE(using_last_item_will_create_quickslot_release) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.potion, 1u);
	BOOST_CHECK(rpg::item_impl::useItem(fix.context, fix.actor, fix.potion));
	auto const& events = fix.quickslot_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].item, &fix.potion);
	BOOST_CHECK(events[0].type == rpg::QuickslotEvent::Release);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cannot_equip_with_unsatisfied_requirements) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.helmet, 1u);
	BOOST_CHECK(!rpg::item_impl::canEquip(fix.context, fix.actor, fix.helmet));
}

BOOST_AUTO_TEST_CASE(can_equip_item_with_satisfied_requirements) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(rpg::item_impl::canEquip(fix.context, fix.actor, fix.sword));
}

BOOST_AUTO_TEST_CASE(cannot_equip_non_existing_item) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	BOOST_CHECK(!rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::ItemNotFound);
}

BOOST_AUTO_TEST_CASE(can_equip_existing_item) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));
	BOOST_CHECK_EQUAL(
		fix.actor.equipment[rpg::EquipmentSlot::Weapon], &fix.sword);
}

BOOST_AUTO_TEST_CASE(equip_item_without_requirements_fails) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.helmet, 1u);
	BOOST_CHECK(!rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.helmet, rpg::EquipmentSlot::Weapon));

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::CannotUseThis);
}

BOOST_AUTO_TEST_CASE(cannot_equip_item_to_stupid_slot) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(!rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Head));

	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Weapon] == nullptr);
	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Head] == nullptr);

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::CannotUseThis);
}

BOOST_AUTO_TEST_CASE(equip_a_second_one_handed_weapon_will_use_extension_slot) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 2u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Extension));
	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Weapon] == &fix.sword);
	BOOST_CHECK(
		fix.actor.equipment[rpg::EquipmentSlot::Extension] == &fix.sword);
}

BOOST_AUTO_TEST_CASE(
	equip_a_second_one_handed_weapon_of_the_same_kind_requires_two_instances) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));
	BOOST_CHECK(!rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Extension));
}

BOOST_AUTO_TEST_CASE(equip_a_two_handed_weapon_resets_an_equipped_extension) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 2u);
	rpg::item_impl::addItem(fix.actor, fix.bow, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Extension));
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.bow, rpg::EquipmentSlot::Weapon));
	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Weapon] == &fix.bow);
	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Extension] == nullptr);
}

BOOST_AUTO_TEST_CASE(equip_an_extension_resets_two_handed_weapon) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.shield, 1u);
	rpg::item_impl::addItem(fix.actor, fix.bow, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.bow, rpg::EquipmentSlot::Weapon));
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.shield, rpg::EquipmentSlot::Extension));
	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Weapon] == nullptr);
	BOOST_CHECK(
		fix.actor.equipment[rpg::EquipmentSlot::Extension] == &fix.shield);
}

BOOST_AUTO_TEST_CASE(equip_slot_propagates_added_boni) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.bow, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.bow, rpg::EquipmentSlot::Weapon));

	auto const& events = fix.boni_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].boni, &fix.bow.boni);
	BOOST_CHECK(events[0].type == rpg::BoniEvent::Add);
}

BOOST_AUTO_TEST_CASE(reequip_slot_propagates_removed_and_added_boni) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.bow, 1u);
	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.bow, rpg::EquipmentSlot::Weapon));
	fix.boni_sender.clear();
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));

	auto const& events = fix.boni_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].boni, &fix.bow.boni);
	BOOST_CHECK(events[0].type == rpg::BoniEvent::Remove);
	BOOST_CHECK_EQUAL(events[1].actor, 1u);
	BOOST_CHECK_EQUAL(events[1].boni, &fix.sword.boni);
	BOOST_CHECK(events[1].type == rpg::BoniEvent::Add);
}

BOOST_AUTO_TEST_CASE(
	equip_one_handed_weapon_as_pure_extension_uses_weapon_slot_first) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Extension));
	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Weapon] == &fix.sword);
	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Extension] == nullptr);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(unequip_clears_slot) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));
	bool success = rpg::item_impl::onUnequip(fix.context, fix.actor, rpg::EquipmentSlot::Weapon);
	BOOST_REQUIRE(success);
	BOOST_CHECK(fix.actor.equipment[rpg::EquipmentSlot::Weapon] == nullptr);
}

BOOST_AUTO_TEST_CASE(cannot_unequip_none_slot) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	BOOST_CHECK_ASSERT(rpg::item_impl::onUnequip(
		fix.context, fix.actor, rpg::EquipmentSlot::None));
}

BOOST_AUTO_TEST_CASE(unequipping_empty_slot_is_harmless) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	bool success = rpg::item_impl::onUnequip(fix.context, fix.actor, rpg::EquipmentSlot::Weapon);
	BOOST_CHECK(!success);
}

BOOST_AUTO_TEST_CASE(unequipping_propagates_remove_boni) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.bow, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.bow, rpg::EquipmentSlot::Weapon));
	rpg::item_impl::onUnequip(
		fix.context, fix.actor, rpg::EquipmentSlot::Weapon);

	auto const& events = fix.boni_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[1].actor, 1u);
	BOOST_CHECK_EQUAL(events[1].boni, &fix.bow.boni);
	BOOST_CHECK(events[1].type == rpg::BoniEvent::Remove);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(hasItem_can_find_existing_item) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 2u);
	BOOST_CHECK(rpg::hasItem(fix.actor, fix.sword));
	BOOST_CHECK(rpg::hasItem(fix.actor, fix.sword, 2u));
	BOOST_CHECK(!rpg::hasItem(fix.actor, fix.sword, 3u));
}

BOOST_AUTO_TEST_CASE(hasItem_works_for_unexisting_items) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 2u);
	BOOST_CHECK(!rpg::hasItem(fix.actor, fix.potion));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(equip_weapon_triggers_animation_update) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Torso);
	BOOST_CHECK(events[0].torso_layer == core::SpriteTorsoLayer::Weapon);
	BOOST_CHECK(events[0].torso == &fix.weapon_sprite.torso);
}

BOOST_AUTO_TEST_CASE(unequip_weapon_triggers_animation_reset) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon);
	fix.animation_sender.clear();
	rpg::item_impl::onUnequip(
		fix.context, fix.actor, rpg::EquipmentSlot::Weapon);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Torso);
	BOOST_CHECK(events[0].torso_layer == core::SpriteTorsoLayer::Weapon);
	BOOST_CHECK(events[0].torso == nullptr);
}

BOOST_AUTO_TEST_CASE(equip_armor_triggers_animation_update) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.armor, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.armor, rpg::EquipmentSlot::Body));

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Legs);
	BOOST_CHECK(events[0].leg_layer == core::SpriteLegLayer::Armor);
	BOOST_CHECK(events[0].legs == &fix.armor_sprite.legs);
	BOOST_CHECK_EQUAL(events[1].actor, 1u);
	BOOST_CHECK(events[1].type == core::AnimationEvent::Torso);
	BOOST_CHECK(events[1].torso_layer == core::SpriteTorsoLayer::Armor);
	BOOST_CHECK(events[1].torso == &fix.armor_sprite.torso);
}

BOOST_AUTO_TEST_CASE(unequip_armor_triggers_animation_reset) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.armor, 1u);
	rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.armor, rpg::EquipmentSlot::Body);
	fix.animation_sender.clear();
	rpg::item_impl::onUnequip(fix.context, fix.actor, rpg::EquipmentSlot::Body);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Torso);
	BOOST_CHECK(events[0].torso_layer == core::SpriteTorsoLayer::Armor);
	BOOST_CHECK(events[0].torso == nullptr);
	BOOST_CHECK_EQUAL(events[1].actor, 1u);
	BOOST_CHECK(events[1].type == core::AnimationEvent::Legs);
	BOOST_CHECK(events[1].leg_layer == core::SpriteLegLayer::Armor);
	BOOST_CHECK(events[1].legs == nullptr);
}

BOOST_AUTO_TEST_CASE(equip_helmet_triggers_animation_update) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	auto& s = fix.stats.query(fix.actor.id);
	s.attributes[rpg::Attribute::Strength] = 10;
	rpg::item_impl::addItem(fix.actor, fix.helmet, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(fix.context, fix.actor, fix.helmet, rpg::EquipmentSlot::Head));

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Torso);
	BOOST_CHECK(events[0].torso_layer == core::SpriteTorsoLayer::Helmet);
	BOOST_CHECK(events[0].torso == &fix.helmet_sprite.torso);
}

BOOST_AUTO_TEST_CASE(unequip_helmet_triggers_animation_reset) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	auto& s = fix.stats.query(fix.actor.id);
	s.attributes[rpg::Attribute::Strength] = 10;
	rpg::item_impl::addItem(fix.actor, fix.helmet, 1u);
	rpg::item_impl::onEquip(fix.context, fix.actor, fix.helmet, rpg::EquipmentSlot::Head);
	auto const & events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	fix.animation_sender.clear();
	rpg::item_impl::onUnequip(fix.context, fix.actor, rpg::EquipmentSlot::Head);

	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Torso);
	BOOST_CHECK(events[0].torso_layer == core::SpriteTorsoLayer::Helmet);
	BOOST_CHECK(events[0].torso == nullptr);
}

BOOST_AUTO_TEST_CASE(equip_weapon_triggers_sprite_update) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.sword, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.sword, rpg::EquipmentSlot::Weapon));

	auto const& events = fix.sprite_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::SpriteEvent::Torso);
	BOOST_CHECK(events[0].torso_layer == core::SpriteTorsoLayer::Weapon);
	BOOST_CHECK(events[0].texture == &fix.weapon_texture);
}

BOOST_AUTO_TEST_CASE(equip_armor_triggers_sprite_update) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	rpg::item_impl::addItem(fix.actor, fix.armor, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(
		fix.context, fix.actor, fix.armor, rpg::EquipmentSlot::Body));

	auto const& events = fix.sprite_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::SpriteEvent::Legs);
	BOOST_CHECK(events[0].leg_layer == core::SpriteLegLayer::Armor);
	BOOST_CHECK(events[0].texture == &fix.armor_texture);
	BOOST_CHECK_EQUAL(events[1].actor, 1u);
	BOOST_CHECK(events[1].type == core::SpriteEvent::Torso);
	BOOST_CHECK(events[1].torso_layer == core::SpriteTorsoLayer::Armor);
	BOOST_CHECK(events[1].texture == &fix.armor_texture);
}

BOOST_AUTO_TEST_CASE(equip_helmet_triggers_sprite_update) {
	auto& fix = Singleton<ItemFixture>::get();
	fix.reset();

	auto& s = fix.stats.query(fix.actor.id);
	s.attributes[rpg::Attribute::Strength] = 10;
	rpg::item_impl::addItem(fix.actor, fix.helmet, 1u);
	BOOST_CHECK(rpg::item_impl::onEquip(fix.context, fix.actor, fix.helmet, rpg::EquipmentSlot::Head));

	auto const& events = fix.sprite_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::SpriteEvent::Torso);
	BOOST_CHECK(events[0].torso_layer == core::SpriteTorsoLayer::Helmet);
	BOOST_CHECK(events[0].texture == &fix.helmet_texture);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cannot_drop_if_ratio_is_negative) {
	rpg::ItemData actor;
	rpg::InteractData corpse;

	BOOST_CHECK_ASSERT(
		rpg::dropItems(actor, corpse, 1u, -0.1f, rpg::drop::byQuantity));
}

BOOST_AUTO_TEST_CASE(cannot_drop_if_ratio_is_larger_than_1) {
	rpg::ItemData actor;
	rpg::InteractData corpse;

	BOOST_CHECK_ASSERT(
		rpg::dropItems(actor, corpse, 1u, 1.1f, rpg::drop::byQuantity));
}

BOOST_AUTO_TEST_CASE(can_drop_if_preconditions_satisfied) {
	rpg::ItemData actor;
	rpg::InteractData corpse;

	BOOST_CHECK_NO_ASSERT(
		rpg::dropItems(actor, corpse, 1u, 0.7f, rpg::drop::byQuantity));
}

BOOST_AUTO_TEST_CASE(all_items_can_be_dropped) {
	rpg::ItemData actor;
	rpg::InteractData corpse;
	rpg::ItemTemplate foo, bar;

	// prepare loot
	actor.inventory[rpg::ItemType::Weapon].emplace_back(foo, 2u);
	actor.inventory[rpg::ItemType::Potion].emplace_back(bar, 11u);

	// drop items
	rpg::dropItems(actor, corpse, 1u, 1.f, rpg::drop::byQuantity);

	// expect all items
	BOOST_REQUIRE_EQUAL(corpse.loot[0].size(), 2u);
	if (corpse.loot[0][0].item == &foo) {
		BOOST_CHECK_EQUAL(corpse.loot[0][0].quantity, 2u);
		BOOST_CHECK_EQUAL(corpse.loot[0][1].item, &bar);
		BOOST_CHECK_EQUAL(corpse.loot[0][1].quantity, 11u);
	} else {
		BOOST_CHECK_EQUAL(corpse.loot[0][0].item, &bar);
		BOOST_CHECK_EQUAL(corpse.loot[0][0].quantity, 11u);
		BOOST_CHECK_EQUAL(corpse.loot[0][1].item, &foo);
		BOOST_CHECK_EQUAL(corpse.loot[0][1].quantity, 2u);
	}
}

BOOST_AUTO_TEST_CASE(only_some_items_can_be_dropped) {
	rpg::ItemData actor;
	rpg::InteractData corpse;
	rpg::ItemTemplate foo, bar;

	// prepare loot
	actor.inventory[rpg::ItemType::Weapon].emplace_back(foo, 1u);
	actor.inventory[rpg::ItemType::Potion].emplace_back(bar, 5u);

	// drop items
	rpg::dropItems(actor, corpse, 1u, 0.65f, rpg::drop::byQuantity);

	// count items
	std::size_t total = 0u;
	for (auto const& pair : corpse.loot[0]) {
		total += pair.quantity;
	}
	BOOST_CHECK_EQUAL(total, 3u);
}

BOOST_AUTO_TEST_CASE(nothing_is_dropped_if_no_items_given) {
	rpg::ItemData actor;
	rpg::InteractData corpse;

	// drop items
	rpg::dropItems(actor, corpse, 1u, 1.f, rpg::drop::byQuantity);

	BOOST_CHECK(corpse.loot[0].empty());
}

BOOST_AUTO_TEST_CASE(can_drop_equally) {
	rpg::ItemData actor;
	rpg::InteractData corpse;
	rpg::ItemTemplate foo, bar;

	// prepare loot
	actor.inventory[rpg::ItemType::Weapon].emplace_back(foo, 1u);
	actor.inventory[rpg::ItemType::Potion].emplace_back(bar, 5u);

	// drop items
	rpg::dropItems(actor, corpse, 3u, 1.f, rpg::drop::byQuantity);

	// count items
	std::array<std::size_t, 3u> num_items;
	num_items.fill(0u);
	for (auto i = 0u; i < 3u; ++i) {
		for (auto const& pair : corpse.loot[i]) {
			num_items[i] += pair.quantity;
		}
	}
	BOOST_CHECK_EQUAL(num_items[0], 2u);
	BOOST_CHECK_EQUAL(num_items[1], 2u);
	BOOST_CHECK_EQUAL(num_items[2], 2u);
}

BOOST_AUTO_TEST_CASE(can_drop_nearly_equal) {
	rpg::ItemData actor;
	rpg::InteractData corpse;
	rpg::ItemTemplate foo, bar;

	// prepare loot
	actor.inventory[rpg::ItemType::Weapon].emplace_back(foo, 1u);
	actor.inventory[rpg::ItemType::Potion].emplace_back(bar, 5u);

	// drop items
	rpg::dropItems(actor, corpse, 4u, 1.f, rpg::drop::byQuantity);

	// count items
	std::array<std::size_t, 4u> num_items;
	num_items.fill(0u);
	for (auto i = 0u; i < 4u; ++i) {
		for (auto const& pair : corpse.loot[i]) {
			num_items[i] += pair.quantity;
		}
	}
	// create histogram
	std::array<std::size_t, 7u> histo;  // number of slots with 0..6 items
	histo.fill(0u);
	for (auto i = 0u; i < 4u; ++i) {
		++histo[num_items[i]];
	}
	BOOST_CHECK_EQUAL(histo[0], 0u);  // 0x zero items
	BOOST_CHECK_EQUAL(histo[1], 2u);  // 2x one item
	BOOST_CHECK_EQUAL(histo[2], 2u);  // 2x two items
	BOOST_CHECK_EQUAL(histo[3], 0u);  // 0x three items
	BOOST_CHECK_EQUAL(histo[4], 0u);  // 0x four items
	BOOST_CHECK_EQUAL(histo[5], 0u);  // 0x five items
	BOOST_CHECK_EQUAL(histo[6], 0u);  // 0x six items
}

BOOST_AUTO_TEST_SUITE_END()
