#include <utils/algorithm.hpp>
#include <rpg/item.hpp>

namespace rpg {

namespace item_impl {

Context::Context(core::LogContext& log, core::AnimationSender& animation_sender,
	core::SpriteSender& sprite_sender, BoniSender& boni_sender,
	QuickslotSender& quickslot_sender, StatsSender& stats_sender,
	FeedbackSender& feedback_sender, StatsManager const& stats)
	: log{log}
	, animation_sender{animation_sender}
	, sprite_sender{sprite_sender}
	, boni_sender{boni_sender}
	, quickslot_sender{quickslot_sender}
	, stats_sender{stats_sender}
	, feedback_sender{feedback_sender}
	, stats{stats} {}

// ---------------------------------------------------------------------------

bool addItem(ItemData& actor, ItemTemplate const& data, std::size_t quantity) {
	auto& slot = actor.inventory[data.type];
	auto i = utils::find_if(
		slot, [&](Item const& node) { return node.item == &data; });
	if (i != slot.end()) {
		// increase quantity
		i->quantity += quantity;
	} else {
		// create new node
		slot.emplace_back(data, quantity);
	}
	return true;
}

bool removeItem(Context& context, ItemData& actor, ItemTemplate const& data,
	std::size_t quantity) {
	// find existing item
	auto& slot = actor.inventory[data.type];
	auto i = utils::find_if(slot, [&](Item const& node) { return node.item == &data; });
	if (i == slot.end() || i->quantity < quantity) {
		context.log.debug << "[Rpg/Item] " << "Cannot remove item '" << data.display_name
						  << "' x" << quantity << "\n";

		// send feedback event
		FeedbackEvent event;
		event.actor = actor.id;
		event.type = FeedbackType::ItemNotFound;
		context.feedback_sender.send(event);

		return false;
	}
	// decrease quantity
	i->quantity -= quantity;
	if (i->quantity == 0u) {
		// remove node (stably!)
		utils::pop(slot, i, true);
		// remove from equipment
		if (actor.equipment[data.slot] != nullptr) {
			onUnequip(context, actor, data.slot);
		}
		// remove from quickslot
		QuickslotEvent event;
		event.actor = actor.id;
		event.type = QuickslotEvent::Release;
		event.item = &data;
		context.quickslot_sender.send(event);
	}
	return true;
}

bool useEquip(Context& context, ItemData& actor, ItemTemplate const& item) {
	if (!hasItem(actor, item, 1u)) {
		// failed! skip equipping the item
		context.log.debug << "[Rpg/Item] " << "Cannot use item '"
			<< item.display_name << "' : Not found\n";
		return false;
	}

	if (actor.equipment[item.slot] == &item) {
		return onUnequip(context, actor, item.slot);
	} else {
		return onEquip(context, actor, item, item.slot);
	}
}

bool useItem(Context& context, ItemData& actor, ItemTemplate const& item) {
	// remove an instance of the item
	if (!removeItem(context, actor, item, 1u)) {
		// failed! skip using the item
		context.log.debug << "[Rpg/Item] " << "Cannot use item '" << item.display_name
						  << "': Not found\n";

		return false;
	}

	// propagate stats change
	StatsEvent event;
	event.actor = actor.id;
	event.causer = actor.id;
	event.delta = item.recover;
	context.stats_sender.send(event);

	return true;
}

// ---------------------------------------------------------------------------

void setAppearance(
	Context& context, ItemData const& actor, ItemTemplate const& item) {
	core::AnimationEvent ani_event;
	core::SpriteEvent sprite_event;
	ani_event.actor = actor.id;
	sprite_event.actor = actor.id;

	switch (item.slot) {
		case EquipmentSlot::None:
			return;
			
		case EquipmentSlot::Weapon:
			ASSERT(item.sprite != nullptr);
			ASSERT(item.sprite->frameset != nullptr);
			// use item on weapon layer
			ani_event.type = core::AnimationEvent::Torso;
			ani_event.torso_layer = core::SpriteTorsoLayer::Weapon;
			ani_event.torso = &item.sprite->torso;
			sprite_event.type = core::SpriteEvent::Torso;
			sprite_event.torso_layer = core::SpriteTorsoLayer::Weapon;
			sprite_event.texture = item.sprite->frameset;
			context.animation_sender.send(ani_event);
			context.sprite_sender.send(sprite_event);
			break;

		case EquipmentSlot::Body:
			ASSERT(item.sprite != nullptr);
			ASSERT(item.sprite->frameset != nullptr);
			// use item on legs-armor layer
			ani_event.type = core::AnimationEvent::Legs;
			ani_event.leg_layer = core::SpriteLegLayer::Armor;
			ani_event.legs = &item.sprite->legs;
			sprite_event.type = core::SpriteEvent::Legs;
			sprite_event.leg_layer = core::SpriteLegLayer::Armor;
			sprite_event.texture = item.sprite->frameset;
			context.animation_sender.send(ani_event);
			context.sprite_sender.send(sprite_event);
			// use item on torso-armor layer
			ani_event.type = core::AnimationEvent::Torso;
			ani_event.torso_layer = core::SpriteTorsoLayer::Armor;
			ani_event.torso = &item.sprite->torso;
			sprite_event.type = core::SpriteEvent::Torso;
			sprite_event.torso_layer = core::SpriteTorsoLayer::Armor;
			sprite_event.texture = item.sprite->frameset;
			context.animation_sender.send(ani_event);
			context.sprite_sender.send(sprite_event);
			break;

		case EquipmentSlot::Extension:
			ASSERT(item.sprite != nullptr);
			ASSERT(item.sprite->frameset != nullptr);
			// use item on torso-shield layer
			ani_event.type = core::AnimationEvent::Torso;
			ani_event.torso_layer = core::SpriteTorsoLayer::Shield;
			ani_event.torso = &item.sprite->torso;
			sprite_event.type = core::SpriteEvent::Torso;
			sprite_event.torso_layer = core::SpriteTorsoLayer::Shield;
			sprite_event.texture = item.sprite->frameset;
			context.animation_sender.send(ani_event);
			context.sprite_sender.send(sprite_event);
			break;

		case EquipmentSlot::Head:
			ASSERT(item.sprite != nullptr);
			ASSERT(item.sprite->frameset != nullptr);
			// use item on torso-armor layer
			ani_event.type = core::AnimationEvent::Torso;
			ani_event.torso_layer = core::SpriteTorsoLayer::Helmet;
			ani_event.torso = &item.sprite->torso;
			sprite_event.type = core::SpriteEvent::Torso;
			sprite_event.torso_layer = core::SpriteTorsoLayer::Helmet;
			sprite_event.texture = item.sprite->frameset;
			context.animation_sender.send(ani_event);
			context.sprite_sender.send(sprite_event);
			break;
	}
}

void resetAppearance(
	Context& context, ItemData const& actor, EquipmentSlot slot) {
	core::AnimationEvent ani_event;
	core::SpriteEvent sprite_event;
	ani_event.actor = actor.id;
	sprite_event.actor = actor.id;

	switch (slot) {
		case EquipmentSlot::None:
			return;
		
		case EquipmentSlot::Weapon:
			// clear weapon layer
			ani_event.type = core::AnimationEvent::Torso;
			ani_event.torso_layer = core::SpriteTorsoLayer::Weapon;
			ani_event.torso = nullptr;
			context.animation_sender.send(ani_event);
			sprite_event.type = core::SpriteEvent::Torso;
			sprite_event.torso_layer = core::SpriteTorsoLayer::Weapon;
			sprite_event.texture = nullptr;
			context.sprite_sender.send(sprite_event);
			break;

		case EquipmentSlot::Body:
			// clear torso-armor layer
			ani_event.type = core::AnimationEvent::Torso;
			ani_event.torso_layer = core::SpriteTorsoLayer::Armor;
			ani_event.torso = nullptr;
			context.animation_sender.send(ani_event);
			sprite_event.type = core::SpriteEvent::Torso;
			sprite_event.torso_layer = core::SpriteTorsoLayer::Armor;
			sprite_event.texture = nullptr;
			context.sprite_sender.send(sprite_event);
			// clear and legs-armor layer
			ani_event.type = core::AnimationEvent::Legs;
			ani_event.leg_layer = core::SpriteLegLayer::Armor;
			ani_event.legs = nullptr;
			context.animation_sender.send(ani_event);
			sprite_event.type = core::SpriteEvent::Legs;
			sprite_event.leg_layer = core::SpriteLegLayer::Armor;
			sprite_event.texture = nullptr;
			context.sprite_sender.send(sprite_event);
			break;

		case EquipmentSlot::Extension:
			// clear torso-shield layer
			ani_event.type = core::AnimationEvent::Torso;
			ani_event.torso_layer = core::SpriteTorsoLayer::Shield;
			ani_event.torso = nullptr;
			context.animation_sender.send(ani_event);
			sprite_event.type = core::SpriteEvent::Torso;
			sprite_event.torso_layer = core::SpriteTorsoLayer::Shield;
			sprite_event.texture = nullptr;
			context.sprite_sender.send(sprite_event);
			break;

		case EquipmentSlot::Head:
			// clear torso-head layer
			ani_event.type = core::AnimationEvent::Torso;
			ani_event.torso_layer = core::SpriteTorsoLayer::Helmet;
			ani_event.torso = nullptr;
			context.animation_sender.send(ani_event);
			sprite_event.type = core::SpriteEvent::Torso;
			sprite_event.torso_layer = core::SpriteTorsoLayer::Helmet;
			sprite_event.texture = nullptr;
			context.sprite_sender.send(sprite_event);
			break;
	}
}

bool canEquip(
	Context const& context, ItemData const& actor, ItemTemplate const& data) {
	auto const& stats = context.stats.query(actor.id);
	for (auto const& pair : data.require) {
		if (stats.attributes[pair.first] < pair.second) {
			return false;
		}
	}
	return true;
}

bool onEquip(Context& context, ItemData& actor, ItemTemplate const& data,
	EquipmentSlot slot) {
	ItemTemplate const* prev = nullptr;

	// search existing node
	auto& s = actor.inventory[data.type];
	auto i =
		utils::find_if(s, [&](Item const& node) { return node.item == &data; });
	if (i == s.end()) {
		context.log.debug << "[Rpg/Item] " << "Cannot equip '" << data.display_name << "' x1\n";

		// send feedback event
		FeedbackEvent event;
		event.actor = actor.id;
		event.type = FeedbackType::ItemNotFound;
		context.feedback_sender.send(event);

		return false;
	}

	if (!canEquip(context, actor, data)) {
		context.log.debug << "[Rpg/Item] " << "Cannot equip '" << data.display_name << "'\n";

		// send feedback event
		FeedbackEvent event;
		event.actor = actor.id;
		event.type = FeedbackType::CannotUseThis;
		context.feedback_sender.send(event);

		return false;
	}

	// use item's slot if not specified
	if (slot == EquipmentSlot::None) {
		slot = data.slot;
	}

	prev = actor.equipment[slot];
	// handle some special cases
	if (slot == EquipmentSlot::Extension) {
		if (data.slot == EquipmentSlot::Weapon) {
			// item should be used as second weapon
			auto ptr = actor.equipment[data.slot];
			if (ptr == nullptr || ptr->two_handed) {
				// either: no weapon itemped, yet (so we use the primary slot)
				// or: current weapon requires both hands (so we replace it)
				// anyway: change slot to weapon
				slot = EquipmentSlot::Weapon;
			} else if (&data == ptr) {
				// both weapons have the same type, so we need to of them
				if (i->quantity < 2) {
					context.log.debug << "[Rpg/Item] " << "Cannot equip '" << data.display_name
									  << "' x2\n";

					// send feedback event
					FeedbackEvent event;
					event.actor = actor.id;
					event.type = FeedbackType::ItemNotFound;
					context.feedback_sender.send(event);

					return false;
				}
			}
		} else if (data.slot == slot) {
			// item should be used as pure extension
			auto ptr = actor.equipment[EquipmentSlot::Weapon];
			if (ptr != nullptr && ptr->two_handed) {
				// current weapon requires two hands, unitem it!
				prev = ptr;
				actor.equipment[EquipmentSlot::Weapon] = nullptr;
			}
		} else {
			// weapon cannot be itemped to another slot

			// send feedback event
			FeedbackEvent event;
			event.actor = actor.id;
			event.type = FeedbackType::CannotUseThis;
			context.feedback_sender.send(event);

			return false;
		}
	} else if (slot == EquipmentSlot::Weapon) {
		if (data.two_handed) {
			// new weapon requires both hands, unitem extension!
			prev = actor.equipment[EquipmentSlot::Extension];
			actor.equipment[EquipmentSlot::Extension] = nullptr;
		}
	} else if (data.slot != slot || data.slot == EquipmentSlot::None) {
		// item cannot be equipped to another slot

		// send feedback event
		FeedbackEvent event;
		event.actor = actor.id;
		event.type = FeedbackType::CannotUseThis;
		context.feedback_sender.send(event);

		return false;
	}

	// finally item item
	if (prev != nullptr) {
		onUnequip(context, actor, prev->slot);
	}
	actor.equipment[slot] = &data;

	// propagate boni add
	BoniEvent event;
	event.actor = actor.id;
	event.type = BoniEvent::Add;
	event.boni = &data.boni;
	context.boni_sender.send(event);

	// update appearance
	setAppearance(context, actor, data);

	return true;
}

bool onUnequip(Context& context, ItemData& actor, EquipmentSlot slot) {
	ASSERT(slot != EquipmentSlot::None);
	if (actor.equipment[slot] != nullptr) {
		// propagate boni remove
		BoniEvent event;
		event.actor = actor.id;
		event.type = BoniEvent::Remove;
		event.boni = &actor.equipment[slot]->boni;
		context.boni_sender.send(event);
		
		// reset slot
		actor.equipment[slot] = nullptr;
		
		// update appearance
		resetAppearance(context, actor, slot);
		
		return true;
	}
	
	return false;
}

}  // ::item_impl

// ---------------------------------------------------------------------------

namespace drop {

std::size_t byQuantity(Item const& node) { return node.quantity; }

std::size_t byWorth(Item const& node) {
	return node.item->worth * node.quantity;
}

}  // ::drop

// ---------------------------------------------------------------------------

bool hasItem(ItemData const& actor, ItemTemplate const& data, std::size_t quantity) {
	auto& slot = actor.inventory[data.type];
	auto i = utils::find_if(
		slot, [&](Item const& node) { return node.item == &data; });
	return i != slot.end() && i->quantity >= quantity;
}

void dropItems(ItemData& actor, InteractData& corpse, std::size_t num_players,
	float loot_ratio, drop::Predicate pred) {
	ASSERT(loot_ratio >= 0.f);
	ASSERT(loot_ratio <= 1.f);
	// collect, count and shuffle all items
	std::vector<Item> items;
	std::size_t total{0u};
	for (auto& pair : actor.inventory) {
		for (auto const& item : pair.second) {
			items.push_back(item);
			total += pred(item);
		}
		pair.second.clear();
	}
	utils::shuffle(items);

	// prepare dropping
	decltype(corpse.loot) loot;
	loot.resize(num_players);
	corpse.loot.resize(num_players);
	auto const per_player =
		static_cast<std::size_t>(loot_ratio * total / num_players);
	auto const remain =
		static_cast<std::size_t>(loot_ratio * total) - per_player * num_players;
	if (per_player == 0u) {
		// nothing to distribute
		return;
	}

	// distribute most items equally
	auto i = items.begin();
	for (auto player = 0u; player < num_players; ++player) {
		auto remain = per_player;
		while (remain > 0u) {
			ASSERT(i != items.end());
			// determine worth of items that need to be dropped from this slot
			auto slot_worth = pred(*i);
			auto drop_worth = std::min(remain, slot_worth);
			// determine number of items to drop
			auto single_worth = slot_worth / i->quantity;
			auto num_drop = drop_worth / single_worth;
			auto delta_worth = num_drop * single_worth;
			ASSERT(num_drop > 0u);
			ASSERT(num_drop <= i->quantity);
			ASSERT(delta_worth <= remain);
			// drop items
			loot[player].emplace_back(*i->item, num_drop);
			i->quantity -= num_drop;
			remain -= delta_worth;
			if (i->quantity == 0u) {
				++i;
			}
		}
	}

	// distribute remaining items somehow
	i = items.begin();
	std::size_t extra{0u};
	for (auto player = 0u; player < num_players; ++player) {
		if (extra >= remain) {
			// extra drops are done
			break;
		}
		// search next available item
		while (i != items.end() && i->quantity == 0u) {
			++i;
		}
		if (i == items.end()) {
			// done
			break;
		}
		// determine target node
		auto& target = loot[player];
		auto j = utils::find_if(
			target, [&](Item const& node) { return node.item == i->item; });
		// drop item
		if (j == target.end()) {
			target.emplace_back(*i->item, 1u);
		} else {
			++j->quantity;
		}
		--i->quantity;
		++extra;
	}

	// finally shuffle loot and merge it with previous loot
	utils::shuffle(loot);
	for (auto player = 0u; player < num_players; ++player) {
		utils::append(corpse.loot[player], loot[player]);
	}
}

// ---------------------------------------------------------------------------

ItemSystem::ItemSystem(core::LogContext& log, std::size_t max_objects, StatsManager const& stats)
	: utils::EventListener<ItemEvent>{}
	, utils::EventSender<core::AnimationEvent, core::SpriteEvent,
		ItemEvent, BoniEvent, QuickslotEvent, StatsEvent, FeedbackEvent>{}
	, ItemManager{max_objects}
	, context{log, *this, *this, *this, *this, *this, *this, stats} {}

void ItemSystem::handle(ItemEvent const& event) {
	if (!has(event.actor)) {
		// no item component
		return;
	}
	ASSERT(event.item != nullptr);
	auto& actor = query(event.actor);

	bool forward{false};
	switch (event.type) {
		case ItemEvent::Add:
			forward = item_impl::addItem(actor, *event.item, event.quantity);
			break;

		case ItemEvent::Remove:
			forward = item_impl::removeItem(context, actor, *event.item, event.quantity);
			break;

		case ItemEvent::Use:
			switch (event.item->type) {
				case ItemType::Weapon:
				case ItemType::Armor:
					// toggle equipment
					forward = item_impl::useEquip(context, actor, *event.item);
					break;

				case ItemType::Potion:
					// use potion
					forward = item_impl::useItem(context, actor, *event.item);
					break;

				default:
					context.log.debug << "[Rpg/Item] "
						<< "Invalid use of item '"
						<< event.item->display_name << "'\n";
					break;
			}
			break;
	}
	
	if (forward) {
		send(event);
	}
}

void ItemSystem::update(sf::Time const& elapsed) {
	dispatch<ItemEvent>(*this);

	propagate<core::AnimationEvent>();
	propagate<core::SpriteEvent>();
	propagate<ItemEvent>();
	propagate<BoniEvent>();
	propagate<QuickslotEvent>();
	propagate<StatsEvent>();
	propagate<FeedbackEvent>();
}

}  // ::game
