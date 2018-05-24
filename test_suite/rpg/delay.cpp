#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/delay.hpp>

struct DelayFixture {
	sf::Texture dummy_tileset;
	core::LogContext log;
	core::IdManager ids;
	std::vector<core::ObjectID> objects;

	core::AnimationSender animation_sender;
	rpg::CombatSender combat_sender;
	rpg::ProjectileSender projectile_sender;
	rpg::InteractSender interact_sender;

	core::DungeonSystem dungeon;
	core::MovementManager movement;
	core::FocusManager focus;
	core::AnimationManager animation;
	rpg::ItemManager item;
	rpg::StatsManager stats;
	rpg::InteractManager interact;
	rpg::PlayerManager player;

	rpg::delay_impl::Context context;

	utils::EnumMap<core::AnimationAction, utils::ActionFrames> demo_ani;
	rpg::BulletTemplate bullet;
	rpg::PerkTemplate fireball, freeze, heal, protect;
	rpg::ItemTemplate sword, bow;

	DelayFixture()
		: dummy_tileset{}
		, log{}
		, ids{}
		, objects{}
		, animation_sender{}
		, combat_sender{}
		, projectile_sender{}
		, interact_sender{}
		, dungeon{}
		, movement{}
		, focus{}
		, animation{}
		, item{}
		, stats{}
		, interact{}
		, player{}
		, context{log, animation_sender, combat_sender, projectile_sender,
			  interact_sender, dungeon, movement, focus, animation, item, stats,
			  interact, player}
		, demo_ani{}
		, bullet{}
		, fireball{}
		, freeze{}
		, heal{}
		, protect{}
		, sword{}
		, bow{} {
		log.debug.add(std::cout);
		auto scene = dungeon.create(
			dummy_tileset, sf::Vector2u{12u, 10u}, sf::Vector2f{1.f, 1.f});
		assert(scene == 1u);
		auto& d = dungeon[1u];
		for (auto y = 1u; y < 10u; ++y) {
			for (auto x = 1u; x < 12u; ++x) {
				d.getCell({x, y}).terrain = core::Terrain::Floor;
			}
		}

		for (auto& pair : demo_ani) {
			pair.second.duration = sf::seconds(1.f);
		}

		fireball.bullet.bullet = &bullet;
		freeze.type = rpg::PerkType::Enemy;
		heal.type = rpg::PerkType::Allied;
		protect.type = rpg::PerkType::Self;
		sword.melee = true;
		bow.melee = false;
	}

	core::ObjectID addActor(sf::Vector2f const& pos = {},
		sf::Vector2f const& look = {1, 0}, rpg::PlayerID player_id = 0u) {
		auto id = ids.acquire();
		objects.push_back(id);
		auto& move = movement.acquire(id);
		move.pos = pos;
		move.scene = 1u;
		move.look = look;
		dungeon[1u].getCell(sf::Vector2u{pos}).entities.push_back(id);
		auto& foc = focus.acquire(id);
		foc.fov = 120.f;
		auto& ani = animation.acquire(id);
		ani.tpl.torso = &demo_ani;
		item.acquire(id);
		auto& stat = stats.acquire(id);
		stat.stats[rpg::Stat::Life] = 1;
		if (player_id > 0u) {
			auto& pl = player.acquire(id);
			pl.player_id = player_id;
		}
		return id;
	}

	core::ObjectID addInteractable(sf::Vector2f const& pos = {}) {
		auto id = ids.acquire();
		objects.push_back(id);
		auto& move = movement.acquire(id);
		move.pos = pos;
		dungeon[1u].getCell(sf::Vector2u{pos}).entities.push_back(id);
		focus.acquire(id);
		interact.acquire(id);
		return id;
	}

	void reset() {
		auto& d = dungeon[1u];
		for (auto y = 0u; y < 10u; ++y) {
			for (auto x = 0u; x < 12u; ++x) {
				d.getCell({x, y}).entities.clear();
			}
		}
		for (auto id : objects) {
			movement.release(id);
			focus.release(id);
			if (animation.has(id)) {
				animation.release(id);
			}
			if (item.has(id)) {
				item.release(id);
			}
			if (stats.has(id)) {
				stats.release(id);
			}
			if (interact.has(id)) {
				interact.release(id);
			}
			if (player.has(id)) {
				player.release(id);
			}
		}
		ids.reset();
		objects.clear();
		movement.cleanup();
		focus.cleanup();
		animation.cleanup();
		item.cleanup();
		stats.cleanup();
		interact.cleanup();
		player.cleanup();

		animation_sender.clear();
		combat_sender.clear();
		projectile_sender.clear();
		interact_sender.clear();

		context.combats.reset();
		context.projectiles.reset();
		context.interacts.reset();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}
};

BOOST_AUTO_TEST_SUITE(delay_test)

BOOST_AUTO_TEST_CASE(can_attack_near_enemy) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target = fix.addActor({2u, 1u}, {0, 1});
	auto found = rpg::delay_impl::queryAttackable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, target);
}

BOOST_AUTO_TEST_CASE(can_attack_pretty_near_enemy) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target = fix.addActor({2u, 1u}, {0, 1});
	fix.movement.query(target).pos.x = 2.99f;
	auto found = rpg::delay_impl::queryAttackable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, target);
}

BOOST_AUTO_TEST_CASE(cannot_attack_far_enemy) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target = fix.addActor({3u, 1u}, {0, 1});
	fix.movement.query(target).pos.x = 3.1f;
	auto found = rpg::delay_impl::queryAttackable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, 0u);
}

BOOST_AUTO_TEST_CASE(cannot_attack_self) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	fix.dungeon[1u].getCell({2u, 1u}).entities.push_back(actor);
	auto found = rpg::delay_impl::queryAttackable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, 0u);
}

BOOST_AUTO_TEST_CASE(can_attack_enemy_if_dead_body_is_near) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto dead = fix.addActor({2u, 1u}, {0, 1});
	fix.stats.query(dead).stats[rpg::Stat::Life] = 0;
	auto target = fix.addActor({2u, 1u}, {0, 1});
	auto found = rpg::delay_impl::queryAttackable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, target);
}

BOOST_AUTO_TEST_CASE(cannot_melee_attack_enemy_if_only_a_corpse_can_be_found) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto dead = fix.addActor({2u, 1u}, {0, 1});
	fix.stats.query(dead).stats[rpg::Stat::Life] = 0;
	auto found = rpg::delay_impl::queryAttackable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, 0u);
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(can_interact_with_near_interactable) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target = fix.addInteractable({2u, 1u});
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, target);
}

BOOST_AUTO_TEST_CASE(player_can_interact_with_a_corpse) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target = fix.addInteractable({2u, 1u});
	auto& i = fix.interact.query(target);
	i.type = rpg::InteractType::Corpse;
	i.loot.resize(1u);
	i.loot[0].resize(1u);
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, target);
}

BOOST_AUTO_TEST_CASE(player_cannot_interact_with_looted_corpse) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target = fix.addInteractable({2u, 1u});
	auto& i = fix.interact.query(target);
	i.type = rpg::InteractType::Corpse;
	i.loot.resize(1u);
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, 0u);
}

BOOST_AUTO_TEST_CASE(queryInteractable_returns_closest_corpse) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();
	
	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target1 = fix.addInteractable({2u, 2u});
	auto target2 = fix.addInteractable({2u, 2u});
	fix.movement.query(target1).pos.y += 0.1f; // so this one is less optimal
	auto& i = fix.interact.query(target1);
	i.type = rpg::InteractType::Corpse;
	i.loot.resize(1u);
	i.loot[0].resize(1u);
	auto& j = fix.interact.query(target2);
	j.type = rpg::InteractType::Corpse;
	j.loot.resize(1u);
	j.loot[0].resize(1u);
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, target2);
}

BOOST_AUTO_TEST_CASE(queryInteractable_returns_closest_suitable_corpse) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();
	
	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target1 = fix.addInteractable({2u, 2u});
	auto target2 = fix.addInteractable({2u, 2u});
	fix.movement.query(target1).pos.y += 0.1f; // so this one is less optimal
	auto& i = fix.interact.query(target1);
	i.type = rpg::InteractType::Corpse;
	i.loot.resize(1u);
	i.loot[0].resize(1u);
	auto& j = fix.interact.query(target2);
	j.type = rpg::InteractType::Corpse;
	j.loot.resize(1u);
	j.loot[0].clear();
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, target1);
}

BOOST_AUTO_TEST_CASE(queryInteractable_ignores_behind_line_of_sight) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {0, -1}, 1u);
	auto target = fix.addInteractable({1u, 2u});
	auto& i = fix.interact.query(target);
	i.type = rpg::InteractType::Corpse;
	i.loot.resize(1u);
	i.loot[0].resize(1u);
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, 0u);
}

BOOST_AUTO_TEST_CASE(queryInteractable_returns_first_non_empty_corpse) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto target1 = fix.addInteractable({2u, 1u});
	auto target2 = fix.addInteractable({2u, 2u});
	auto& i = fix.interact.query(target1);
	i.type = rpg::InteractType::Corpse;
	i.loot.resize(1u);
	i.loot[0].clear();
	auto& j = fix.interact.query(target2);
	j.type = rpg::InteractType::Corpse;
	j.loot.resize(1u);
	j.loot[0].resize(1u);
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, target2);
}

BOOST_AUTO_TEST_CASE(cannot_interact_with_corpse_if_not_a_player) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 0u);
	auto target = fix.addInteractable({2u, 1u});
	auto& i = fix.interact.query(target);
	i.type = rpg::InteractType::Corpse;
	i.loot.resize(1u);
	i.loot[0].resize(1u);
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, 0u);
}

BOOST_AUTO_TEST_CASE(barrier_interact_is_prefered_over_looting) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0}, 1u);
	auto corpse = fix.addInteractable({2u, 1u});
	auto& i = fix.interact.query(corpse);
	i.type = rpg::InteractType::Corpse;
	i.loot.resize(1u);
	i.loot[0].resize(1u);
	auto barrier = fix.addInteractable({2u, 1u});
	fix.interact.query(barrier).type = rpg::InteractType::Barrier;
	auto found = rpg::delay_impl::queryInteractable(fix.context, actor);
	BOOST_CHECK_EQUAL(found, barrier);
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(attack_by_fists_schedules_combat_event) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::delay_impl::onAttack(fix.context, actor);

	auto const& data = fix.context.combats.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK_EQUAL(data[0].value.actor, actor);
	BOOST_CHECK_EQUAL(data[0].value.target, 0u);  // target is specified later
	BOOST_CHECK(data[0].value.meta_data.emitter == rpg::EmitterType::Weapon);
	BOOST_CHECK(data[0].value.meta_data.primary == nullptr);
	BOOST_CHECK(data[0].value.meta_data.secondary == nullptr);
}

BOOST_AUTO_TEST_CASE(attack_by_fists_triggers_melee_animation) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::delay_impl::onAttack(fix.context, actor);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Melee);
}

BOOST_AUTO_TEST_CASE(attack_by_sword_schedules_combat_event) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	auto& item = fix.item.query(actor);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.sword;
	rpg::delay_impl::onAttack(fix.context, actor);

	auto const& data = fix.context.combats.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK_EQUAL(data[0].value.actor, actor);
	BOOST_CHECK_EQUAL(data[0].value.target, 0u);  // target is specified later
	BOOST_CHECK(data[0].value.meta_data.emitter == rpg::EmitterType::Weapon);
	BOOST_CHECK_EQUAL(data[0].value.meta_data.primary, &fix.sword);
	BOOST_CHECK(data[0].value.meta_data.secondary == nullptr);
}

BOOST_AUTO_TEST_CASE(attack_by_sword_triggers_melee_animation) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	auto& item = fix.item.query(actor);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.sword;
	rpg::delay_impl::onAttack(fix.context, actor);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Melee);
}

BOOST_AUTO_TEST_CASE(attack_by_two_swords_schedules_combat_event) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	auto& item = fix.item.query(actor);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.sword;
	item.equipment[rpg::EquipmentSlot::Extension] = &fix.sword;
	rpg::delay_impl::onAttack(fix.context, actor);

	auto const& data = fix.context.combats.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK_EQUAL(data[0].value.actor, actor);
	BOOST_CHECK_EQUAL(data[0].value.target, 0u);  // target is specified later
	BOOST_CHECK(data[0].value.meta_data.emitter == rpg::EmitterType::Weapon);
	BOOST_CHECK_EQUAL(data[0].value.meta_data.primary, &fix.sword);
	BOOST_CHECK_EQUAL(data[0].value.meta_data.secondary, &fix.sword);
}

BOOST_AUTO_TEST_CASE(attack_by_two_swords_triggers_melee_animation) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	auto& item = fix.item.query(actor);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.sword;
	item.equipment[rpg::EquipmentSlot::Extension] = &fix.sword;
	rpg::delay_impl::onAttack(fix.context, actor);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Melee);
}

BOOST_AUTO_TEST_CASE(attack_by_bow_schedules_projectile_event) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({2u, 3u});
	auto& item = fix.item.query(actor);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.bow;
	rpg::delay_impl::onAttack(fix.context, actor);

	auto const& data = fix.context.projectiles.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK(data[0].value.type == rpg::ProjectileEvent::Create);
	BOOST_CHECK_EQUAL(data[0].value.id, actor);
	BOOST_CHECK_EQUAL(data[0].value.spawn.scene, 1u);
	// note: position and direction are NOT set here
	BOOST_CHECK(data[0].value.meta_data.emitter == rpg::EmitterType::Weapon);
	BOOST_CHECK_EQUAL(data[0].value.meta_data.primary, &fix.bow);
	BOOST_CHECK(data[0].value.meta_data.secondary == nullptr);
}

BOOST_AUTO_TEST_CASE(attack_by_bow_triggers_range_animation) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	auto& item = fix.item.query(actor);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.bow;
	rpg::delay_impl::onAttack(fix.context, actor);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Range);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(interact_schedules_interact_event) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u});
	rpg::delay_impl::onInteract(fix.context, actor);

	auto const& data = fix.context.interacts.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK_EQUAL(data[0].value.actor, actor);
	BOOST_CHECK_EQUAL(data[0].value.target, 0u);  // target is specified later
}

BOOST_AUTO_TEST_CASE(interact_triggers_use_animation) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u});
	fix.addInteractable({2u, 1u});
	rpg::delay_impl::onInteract(fix.context, actor);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Use);
}

BOOST_AUTO_TEST_CASE(also_interact_triggers_use_if_target_too_far_away) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u});
	rpg::delay_impl::onInteract(fix.context, actor);

	BOOST_CHECK(!fix.context.interacts.data().empty());
	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Use);
}

BOOST_AUTO_TEST_CASE(
	also_interact_triggers_use_if_target_offside_field_of_view) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u});
	rpg::delay_impl::onInteract(fix.context, actor);

	BOOST_CHECK(!fix.context.interacts.data().empty());
	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Use);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(use_perk_triggers_magic_animation) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::delay_impl::onPerk(fix.context, actor, fix.heal);

	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Magic);
}

BOOST_AUTO_TEST_CASE(
	use_offensive_non_bullet_perk_schedules_combat_event_targeting_focused_object) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::delay_impl::onPerk(fix.context, actor, fix.freeze);

	auto const& data = fix.context.combats.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK_EQUAL(data[0].value.actor, actor);
	BOOST_CHECK_EQUAL(data[0].value.target, 0u);  // target is specified later
	BOOST_CHECK(data[0].value.meta_data.emitter == rpg::EmitterType::Perk);
	BOOST_CHECK_EQUAL(data[0].value.meta_data.perk, &fix.freeze);
}

BOOST_AUTO_TEST_CASE(
	use_support_non_bullet_perk_schedules_combat_event_targeting_focused_object) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::delay_impl::onPerk(fix.context, actor, fix.heal);

	auto const& data = fix.context.combats.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK_EQUAL(data[0].value.actor, actor);
	BOOST_CHECK_EQUAL(data[0].value.target, 0u);  // target is specified later
	BOOST_CHECK(data[0].value.meta_data.emitter == rpg::EmitterType::Perk);
	BOOST_CHECK_EQUAL(data[0].value.meta_data.perk, &fix.heal);
}

BOOST_AUTO_TEST_CASE(
	use_defensive_non_bullet_perk_schedules_combat_event_targeting_self) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::delay_impl::onPerk(fix.context, actor, fix.protect);

	auto const& data = fix.context.combats.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK_EQUAL(data[0].value.actor, actor);
	BOOST_CHECK_EQUAL(
		data[0].value.target, actor);  // target IS specified here!
	BOOST_CHECK(data[0].value.meta_data.emitter == rpg::EmitterType::Perk);
	BOOST_CHECK_EQUAL(data[0].value.meta_data.perk, &fix.protect);
}

BOOST_AUTO_TEST_CASE(use_bullet_perk_schedules_projectile_event) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({2u, 3u});
	rpg::delay_impl::onPerk(fix.context, actor, fix.fireball);

	auto const& data = fix.context.projectiles.data();
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_TIME_EQUAL(data[0].delay, sf::milliseconds(750));
	BOOST_CHECK(data[0].value.type == rpg::ProjectileEvent::Create);
	BOOST_CHECK_EQUAL(data[0].value.id, actor);
	BOOST_CHECK_EQUAL(data[0].value.spawn.scene, 1u);
	// note: position and direction are NOT set here
	BOOST_CHECK(data[0].value.meta_data.emitter == rpg::EmitterType::Perk);
	BOOST_CHECK_EQUAL(data[0].value.meta_data.perk, &fix.fireball);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(interact_target_is_unspecified_until_delay_was_finished) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	// trigger interaction
	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::InteractEvent event;
	event.actor = actor;
	fix.context.interacts.push(event, sf::milliseconds(400));
	auto step = sf::milliseconds(25);

	// spawn target
	auto target = fix.addInteractable({2u, 1u});

	// wait until delay was finished
	auto const& events = fix.interact_sender.data();
	for (auto t = sf::Time::Zero; t < sf::milliseconds(400); t += step) {
		BOOST_REQUIRE(events.empty());
		rpg::delay_impl::onUpdate(fix.context, step);
	}

	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK_EQUAL(events[0].target, target);
}

BOOST_AUTO_TEST_CASE(combat_target_is_unspecified_until_delay_was_finished) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	// trigger combat
	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::CombatEvent event;
	event.actor = actor;
	event.meta_data.emitter = rpg::EmitterType::Perk;
	event.meta_data.perk = &fix.freeze;
	fix.context.combats.push(event, sf::milliseconds(400));
	auto step = sf::milliseconds(25);

	// spawn target
	auto target = fix.addActor({2u, 1u}, {1, 0});

	// wait until delay was finished
	auto const& events = fix.combat_sender.data();
	for (auto t = sf::Time::Zero; t < sf::milliseconds(400); t += step) {
		BOOST_REQUIRE(events.empty());
		rpg::delay_impl::onUpdate(fix.context, step);
	}

	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK_EQUAL(events[0].target, target);
	BOOST_CHECK(events[0].meta_data.emitter == event.meta_data.emitter);
	BOOST_CHECK_EQUAL(events[0].meta_data.perk, event.meta_data.perk);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(interact_with_target_is_forwarded_after_delay) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	auto target = fix.addInteractable({2u, 1u});
	rpg::InteractEvent event;
	event.actor = actor;
	fix.context.interacts.push(event, sf::milliseconds(400));
	auto step = sf::milliseconds(25);
	auto const& events = fix.interact_sender.data();
	for (auto t = sf::Time::Zero; t < sf::milliseconds(400); t += step) {
		BOOST_REQUIRE(events.empty());
		rpg::delay_impl::onUpdate(fix.context, step);
	}

	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK_EQUAL(events[0].target, target);
}

BOOST_AUTO_TEST_CASE(interact_without_target_is_not_forwarded_after_delay) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::InteractEvent event;
	event.actor = actor;
	fix.context.interacts.push(event, sf::milliseconds(400));
	auto step = sf::milliseconds(25);
	auto const& events = fix.interact_sender.data();
	for (auto t = sf::Time::Zero; t < sf::milliseconds(400); t += step) {
		BOOST_REQUIRE(events.empty());
		rpg::delay_impl::onUpdate(fix.context, step);
	}

	BOOST_REQUIRE(events.empty());
}

BOOST_AUTO_TEST_CASE(combat_with_target_is_forwarded_after_delay) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	auto target = fix.addActor({2u, 1u}, {1, 0});
	rpg::CombatEvent event;
	event.actor = actor;
	event.meta_data.emitter = rpg::EmitterType::Perk;
	event.meta_data.perk = &fix.freeze;
	fix.context.combats.push(event, sf::milliseconds(400));
	auto step = sf::milliseconds(25);
	auto const& events = fix.combat_sender.data();
	for (auto t = sf::Time::Zero; t < sf::milliseconds(400); t += step) {
		BOOST_REQUIRE(events.empty());
		rpg::delay_impl::onUpdate(fix.context, step);
	}

	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, actor);
	BOOST_CHECK_EQUAL(events[0].target, target);
	BOOST_CHECK(events[0].meta_data.emitter == event.meta_data.emitter);
	BOOST_CHECK_EQUAL(events[0].meta_data.perk, event.meta_data.perk);
}

BOOST_AUTO_TEST_CASE(combat_without_target_is_not_forwarded_after_delay) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::CombatEvent event;
	event.actor = actor;
	event.meta_data.emitter = rpg::EmitterType::Perk;
	event.meta_data.perk = &fix.freeze;
	fix.context.combats.push(event, sf::milliseconds(400));
	auto step = sf::milliseconds(25);
	auto const& events = fix.combat_sender.data();
	for (auto t = sf::Time::Zero; t < sf::milliseconds(400); t += step) {
		BOOST_REQUIRE(events.empty());
		rpg::delay_impl::onUpdate(fix.context, step);
	}

	BOOST_REQUIRE(events.empty());
}

BOOST_AUTO_TEST_CASE(projectile_events_are_forwarded_after_delay) {
	auto& fix = Singleton<DelayFixture>::get();
	fix.reset();

	auto actor = fix.addActor({1u, 1u}, {1, 0});
	rpg::ProjectileEvent event;
	event.type = rpg::ProjectileEvent::Create;
	event.id = actor;
	event.spawn.scene = 1u;
	event.spawn.pos = {2u, 3u};
	event.spawn.direction = {1, 1};
	event.meta_data.emitter = rpg::EmitterType::Perk;
	event.meta_data.perk = &fix.fireball;
	fix.context.projectiles.push(event, sf::milliseconds(400));
	auto step = sf::milliseconds(25);
	auto const& events = fix.projectile_sender.data();
	for (auto t = sf::Time::Zero; t < sf::milliseconds(400); t += step) {
		BOOST_REQUIRE(events.empty());
		rpg::delay_impl::onUpdate(fix.context, step);
	}

	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK(events[0].type == rpg::ProjectileEvent::Create);
	BOOST_CHECK_EQUAL(events[0].id, actor);
	BOOST_CHECK_EQUAL(events[0].spawn.scene, event.spawn.scene);
	BOOST_CHECK_VECTOR_EQUAL(events[0].spawn.pos, event.spawn.pos);
	BOOST_CHECK_VECTOR_EQUAL(events[0].spawn.direction, event.spawn.direction);
	BOOST_CHECK(events[0].meta_data.emitter == event.meta_data.emitter);
	BOOST_CHECK_EQUAL(events[0].meta_data.perk, event.meta_data.perk);
}

BOOST_AUTO_TEST_SUITE_END()
