#pragma once
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace quickslot_impl {

/// Cooldown in ms for switching or using a quickslot
extern unsigned int const SLOT_COOLDOWN;

/// Quickslot context
struct Context {
	core::LogContext& log;
	ItemSender& item_sender;
	PerkSender& perk_sender;
	FeedbackSender& feedback_sender;

	Context(core::LogContext& log, ItemSender& item_sender,
		PerkSender& perk_sender, FeedbackSender& feedback_sender);
};

// ---------------------------------------------------------------------------

/// This will switch the quickslot selection
/**
 *	If next is true, the next quickslot will be selected, otherwise the
 *	previous one. Overflow is handled; the successor of the last slot is the
 *	first one and verse vica. After switching the slot, a cooldown is applied.
 *
 *	@param context QuickslotContext to use
 *	@param data QuickslotData to switch for
 *	@param next Specifies whether the next or the previous slot will be selected
 */
void onSwitchSlot(Context& context, QuickslotData& data, bool next);

/// This will use the current quickslot's shortcut
/**
 *	If there is a shortcut assigned to the current quickslot, it will be used.
 *	If a perk is assigned, a PerkEvent about using the perk is propagated. If
 *	an item is assigned, an ItemEvent about using that item is propagated.
 *	Using a potion and equipping or unequipping items is possible. If nothing
 *	is assigned to the shortcut, nothing happens. If a non-empty shortcut was
 *	used, a cooldown is applied.
 *
 *	@param context QuickslotContext to use
 *	@param data QuickslotData to use
 */
void onUseSlot(Context& context, QuickslotData& data);

/// Assign an item to a slot
/**
 *	This will assign the given item to the specified slot. The existing
 *	shortcut will be replaced.
 *
 *	@pre slot_id < MAX_QUICKSLOTS
 *	@param data QuickslotData to assign for
 *	@param item ItemTemplate to assign
 *	@param slot_id Slot to assign at
 */
void assignSlot(
	QuickslotData& data, ItemTemplate const& item, std::size_t slot_id);

/// Assign a perk to a slot
/**
 *	This will assign the given perk to the specified slot. The existing
 *	shortcut will be replaced.
 *
 *	@pre slot_id < MAX_QUICKSLOTS
 *	@param data QuickslotData to assign for
 *	@param perk PerkTemplate to assign
 *	@param slot_id Slot to assign at
 */
void assignSlot(
	QuickslotData& data, PerkTemplate const& perk, std::size_t slot_id);

/// Remove an item from all slots
/**
 *	This will remove the given item from all slots. The shortcuts will be
 *	replaced with empty shortcuts.
 *
 *	@param data QuickslotData to release for
 *	@param item Itememplate to release
 */
void releaseSlot(QuickslotData& data, ItemTemplate const& item);

/// Remove a perk from all slots
/**
 *	This will remove the given perk from all slots. The shortcuts will be
 *	replaced with empty shortcuts.
 *
 *	@param data QuickslotData to release for
 *	@param perk PerkTemplate to release
 */
void releaseSlot(QuickslotData& data, PerkTemplate const& perk);

/// This will update the quickslot data
/**
 *	The update will count down the slot's cooldown using the given elapsed
 *	time.
 *
 *	@param context QuickslotContext to use
 *	@param data QuickslotData to update
 *	@param elapsed Time since last update
 */
void onUpdate(Context& context, QuickslotData& data, sf::Time const& elapsed);

}  // ::quickslot_impl

// ---------------------------------------------------------------------------

/// The QuickslotSystem handles quick use of items and perks
/**
 *	ActionEvents are used to switch or use the current slot. A cooldown is
 *	applied to avoid too fast switching or using. If a perk is used by
 *	quickslot, a PerkEvent about using the corresponding perk it is
 *	propagated. If an item is used by quickslot, an ItemEvent about using the
 *	corresponding item is propagated.
 *	QuickslotEvents are handled either by assigning an item / a perk to a
 *	shortcut or by releasing an item / a perk from all shortcuts.
 */
class QuickslotSystem
	// Event API
	: public utils::EventListener<ActionEvent, QuickslotEvent>,
	  public utils::EventSender<ItemEvent, PerkEvent, FeedbackEvent>
	  // Component API
	  ,
	  public QuickslotManager {

  private:
	quickslot_impl::Context context;

  public:
	QuickslotSystem(core::LogContext& log, std::size_t max_objects);

	void handle(ActionEvent const& event);
	void handle(QuickslotEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game
