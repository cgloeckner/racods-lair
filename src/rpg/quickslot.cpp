#include <utils/algorithm.hpp>
#include <core/collision.hpp>
#include <rpg/quickslot.hpp>

namespace rpg {

namespace quickslot_impl {

unsigned int const SLOT_COOLDOWN = 250u;

Context::Context(core::LogContext& log, ItemSender& item_sender,
	PerkSender& perk_sender, FeedbackSender& feedback_sender)
	: log{log}
	, item_sender{item_sender}
	, perk_sender{perk_sender}
	, feedback_sender{feedback_sender} {}

// ---------------------------------------------------------------------------

void onSwitchSlot(Context& context, QuickslotData& data, bool next) {
	if (data.cooldown != sf::Time::Zero) {
		// not ready
		return;
	}

	auto max = MAX_QUICKSLOTS - 1u;
	// change slot id
	if (next) {
		if (data.slot_id == max) {
			// handle overflow
			data.slot_id = 0u;
		} else {
			++data.slot_id;
		}
	} else {
		if (data.slot_id == 0u) {
			// handle underflow
			data.slot_id = max;
		} else {
			--data.slot_id;
		}
	}
	ASSERT(data.slot_id <= max);
	context.log.debug << "[Rpg/Quickslot] " << "Changed slot to #" << data.slot_id << "\n";

	// set cooldown
	data.cooldown = sf::milliseconds(SLOT_COOLDOWN);
}

void onUseSlot(Context& context, QuickslotData& data) {
	auto const& shortcut = data.slots[data.slot_id];
	if (shortcut.item != nullptr) {
		// trigger item use
		ItemEvent event;
		event.actor = data.id;
		event.type = ItemEvent::Use;
		event.item = shortcut.item;
		context.item_sender.send(event);

	} else if (shortcut.perk != nullptr) {
		// trigger perk use
		PerkEvent event;
		event.actor = data.id;
		event.type = PerkEvent::Use;
		event.perk = shortcut.perk;
		context.perk_sender.send(event);

	} else {
		// empty shortcut
		FeedbackEvent event;
		event.actor = data.id;
		event.type = FeedbackType::EmptyShortcut;
		context.feedback_sender.send(event);
	}
}

void assignSlot(
	QuickslotData& data, ItemTemplate const& item, std::size_t slot_id) {
	ASSERT(slot_id < MAX_QUICKSLOTS);
	data.slots[slot_id] = Shortcut{item};
}

void assignSlot(
	QuickslotData& data, PerkTemplate const& perk, std::size_t slot_id) {
	ASSERT(slot_id < MAX_QUICKSLOTS);
	data.slots[slot_id] = Shortcut{perk};
}

void releaseSlot(QuickslotData& data, ItemTemplate const& item) {
	for (auto& slot : data.slots) {
		if (slot.item == &item) {
			slot = Shortcut{};
		}
	}
}

void releaseSlot(QuickslotData& data, PerkTemplate const& perk) {
	for (auto& slot : data.slots) {
		if (slot.perk == &perk) {
			slot = Shortcut{};
		}
	}
}

void onUpdate(Context& context, QuickslotData& data, sf::Time const& elapsed) {
	data.cooldown -= elapsed;
	if (data.cooldown < sf::Time::Zero) {
		data.cooldown = sf::Time::Zero;
	}
}

}  // ::quickslot_impl

// ---------------------------------------------------------------------------

QuickslotSystem::QuickslotSystem(core::LogContext& log, std::size_t max_objects)
	: utils::EventListener<ActionEvent, QuickslotEvent>{}
	, utils::EventSender<ItemEvent, PerkEvent, FeedbackEvent>{}
	, QuickslotManager{max_objects}
	, context{log, *this, *this, *this} {}

void QuickslotSystem::handle(ActionEvent const& event) {
	if (!has(event.actor)) {
		// object has no quickslot component
		return;
	}
	auto& data = query(event.actor);

	switch (event.action) {
		case PlayerAction::PrevSlot:
			quickslot_impl::onSwitchSlot(context, data, false);
			break;

		case PlayerAction::NextSlot:
			quickslot_impl::onSwitchSlot(context, data, true);
			break;

		case PlayerAction::UseSlot:
			quickslot_impl::onUseSlot(context, data);
			break;

		default:
			break;
	}
}

void QuickslotSystem::handle(QuickslotEvent const& event) {
	if (!has(event.actor)) {
		// object has no quickslot component
		return;
	}
	auto& data = query(event.actor);

	switch (event.type) {
		case QuickslotEvent::Assign:
			if (event.item != nullptr) {
				quickslot_impl::assignSlot(data, *event.item, event.slot_id);
			} else if (event.perk != nullptr) {
				quickslot_impl::assignSlot(data, *event.perk, event.slot_id);
			}
			break;

		case QuickslotEvent::Release:
			if (event.item != nullptr) {
				quickslot_impl::releaseSlot(data, *event.item);
			} else if (event.perk != nullptr) {
				quickslot_impl::releaseSlot(data, *event.perk);
			}
	}
}

void QuickslotSystem::update(sf::Time const& elapsed) {
	dispatch<ActionEvent>(*this);
	dispatch<QuickslotEvent>(*this);

	for (auto& data : *this) {
		quickslot_impl::onUpdate(context, data, elapsed);
	}

	propagate<ItemEvent>();
	propagate<PerkEvent>();
	propagate<FeedbackEvent>();
}

}  // ::game
