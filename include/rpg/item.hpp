#pragma once
#include <functional>

#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace item_impl {

/// Item context
struct Context {
	core::LogContext& log;
	core::AnimationSender& animation_sender;
	core::SpriteSender& sprite_sender;
	BoniSender& boni_sender;
	QuickslotSender& quickslot_sender;
	StatsSender& stats_sender;
	FeedbackSender& feedback_sender;

	StatsManager const& stats;

	Context(core::LogContext& log, core::AnimationSender& animation_sender,
		core::SpriteSender& sprite_sender, BoniSender& boni_sender,
		QuickslotSender& quickslot_sender, StatsSender& stats_sender,
		FeedbackSender& feedback_sender, StatsManager const& stats);
};

// ---------------------------------------------------------------------------

/// This will add some items
/**
 *	This will add some instances of the given item to the actor. If there is
 *	already such an item (or more) stored, they will stack.
 *
 *	@param actor ItemData of the actor to add for
 *	@param item ItemTemplate specifying the item
 *	@param quantity Number of items to add
 */
bool addItem(ItemData& actor, ItemTemplate const& item, std::size_t quantity);

/// This will remove some items
/**
 *	If not enough items are found, the function will return false - otherwise
 *	true. If enough items are found, they are unstacked. If an empty stack
 *	remains, the entire node is removed from the inventory. If the node is
 *	removed, a QuickslotEvent will be propagated to remove the item from
 *	this player's quickslot shortcuts.
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor to remove from
 *	@param item ItemTemplate specifying the item
 *	@param quantity Number of items to remove
 */
bool removeItem(Context& context, ItemData& actor, ItemTemplate const& item,
	std::size_t quantity);

/// Trigger suitable unequip
/**
 *	If the item can be equipped, suitable unequip events are propagated.
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor to unequip for
 *	@param item ItemTemplate specifying the item
 */
void triggerUnequip(
	Context& context, ItemData const& actor, ItemTemplate const& item);

/// Use the given equipment
/**
 *	This will toggle equipping the item. If it is already equipped, it will be
 *	unequipped. If another or no object is equipped to the slot, which the
 *	item is supposed to be equipped to, the existing one will be replaced with
 *	the given item. Nothing happens if the item cannot be equipped.
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor
 *	@param item ItemTemplate of the equipment
 */
bool useEquip(Context& context, ItemData& actor, ItemTemplate const& item);

/// Uses the given item
/**
 *	The item is supposed to be a potion. If it exists, its quantity is
 *	decreased. Then the stats recovery is propagates via a StatsEvent. If the
 *	last item was used, it is properly removed. If the item was successfuly
 *	used, true is returned - otherwise false
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor
 *	@param item ItemTemplate specifying the item
 *	@param true if the item was successfully used
 */
bool useItem(Context& context, ItemData& actor, ItemTemplate const& item);

// ---------------------------------------------------------------------------

/// This will update the animation and render data after itemping an item
/**
 *	The animation's layer and the sprite's texture are replaced to display
 *	the newly itemped item when drawing the object. If the item is not
 *	supposed to be drawn while itemped, nothing will happen. The actual
 *	update is only triggered by propagating AnimationEvents and SpriteEvents.
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor
 *	@param item ItemTemplate specifying the item
 */
void setAppearance(
	Context& context, ItemData const& actor, ItemTemplate const& item);

/// This will update the animation and render data after unitemping a slot
/**
 *	The animation's layer and the sprite's texture are replaced to avoid
 *	displaying the unitemped item when drawing the object. The actual
 *	update is only triggered by propagating AnimationEvents and SpriteEvents.
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor
 *	@param slot EquipmentSlot specifying which layers to update
 */
void resetAppearance(
	Context& context, ItemData const& actor, EquipmentSlot slot);

/// Checks whether an item can be itemped
/**
 *	This will check the actor's attributes to determine whether they satisfy
 *	the given item's requirements. If yes, true is returned - otherwise false.
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor
 *	@param data ItemTemplate specifying the item
 *	@return true if item can be itemped
 */
bool canEquip(
	Context const& context, ItemData const& actor, ItemTemplate const& data);

/// Items the given item to the given slot
/**
 *	Items the given item to the given itemment slot if possible. If not, the
 *	function will return false - otherwise true. If the item does not fit the
 *	specified slot, it cannot be applied.
 *	Two one-handed weapons can be itemped, if both items exist. If a
 *	two-handed weapon is itemped and a shield should be itemped, the weapon
 *	slot is reset. If a shield is itemped and a two-handed weapon should be
 *	itemped, the shield is reset. If two one-handed weapons are itemped and
 *	a two-handed weapon should be itemped, the second weapon is removed and
 *	the first one is replaced.
 *	A BoniEvent is propagated if newly itemped item applies boni. All
 *	unitemped items are unitemped using `unitemItem()`.
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor
 *	@param data ItemTemplate specifying the item
 *	@param slot EquipmentSlot to item to
 *	@return true if the operation was successful
 */
bool onEquip(Context& context, ItemData& actor, ItemTemplate const& data,
	EquipmentSlot slot);

/// Unitems the given slot's item
/**
 *	If the item provided boni/mali, a BoniEvent is propagated in order to
 *	remove them. After that the slot is cleared.
 *
 *	@param context ItemContext to use
 *	@param actor ItemData of the actor
 *	@param slot EquipmentSlot to clear
 */
bool onUnequip(Context& context, ItemData& actor, EquipmentSlot slot);

}  // ::item_impl

// ---------------------------------------------------------------------------

namespace drop {

/// Predicate used while dropping items to specify its "worth"
using Predicate = std::function<std::size_t(Item const& node)>;

/// Predicate returning the items' quantity
/**
 *	This predicate can be used while dropping items. If it is used, items will
 *	be dropped only by its quantity.
 *
 *	@param node Item node to use
 *	@return number of items in this node
 */
std::size_t byQuantity(Item const& node);

/// Predicate returning the items' total worth
/**
 *	This predicate can be used while dropping items. If it is used, items will
 *	be dropped by their total worth with respect to their quantity as well.
 *
 *	@param node Item node to use
 *	@return total worth of the items
 */
std::size_t byWorth(Item const& node);

}  // ::drop

// ---------------------------------------------------------------------------
// Public Item API

/// Checks whether the actor has the given amount of the specified object
/**
 *	@param actor ItemData of the actor
 *	@param data ItemTemplate specifying the item
 *	@param quantity Number of items that are expected
 *	@return true if the actor has at least that much of these items
 */
bool hasItem(
	ItemData const& actor, ItemTemplate const& data, std::size_t quantity = 1u);

/// Drops items from an actor to a corpse
/**
 *	This will drop some (or all) items from the actor to the corpse, with
 *	respect to the number of players available. The loot will be assigned per
 *	player and will be nearly equally referring to the used drop predicate.
 *	The loot ratio also specify how many of the total items are dropped. The
 *	actor's entire inventory is cleared after dropping - even if not all items
 *	were dropped. The loot is distributed randomly.
 *
 *	@pre loot_ratio in [0.0, 1.0]
 *	@param actor ItemData to drop from
 *	@param corpse InteractData to drop to
 *	@param num_players Number of players what participate in the game
 *	@param loot_ratio specifies how many items of the total inventory will be
 *dropped
 *	@param pred Dropping predicated used to specify the item's worth
 */
void dropItems(ItemData& actor, InteractData& corpse, std::size_t num_players,
	float loot_ratio, drop::Predicate pred);

// ---------------------------------------------------------------------------

/// The ItemSystem is used to handle item storage and use
/**
 *	ItemEvents can refer to adding, removing or using an item. If an item is
 *	added it will be stacked to the inventory. If it is removed, it will be
 *	unstacked. If all items were removed, a QuickslotEvent is generated to let
 *	the QuickslotSystem remove the item from all shortcuts of the object. Also
 *	an EquipEvent is generated to remove the item from all equipment slots.
 *	If an item is used, the behavior depends on the item type. If a potion is
 *	used, it will be removed and a StatsEvent is propagated to let the
 *	StatsSystem apply the potion. If an equippable item is used, an EquipEvent
 *	is propagated. Whether this equips or unequips the item is not specified
 *	here.
 */
class ItemSystem
	// Event API
	: public utils::EventListener<ItemEvent>,
	  public utils::EventSender<core::AnimationEvent, core::SpriteEvent,
		ItemEvent, BoniEvent, QuickslotEvent, StatsEvent, FeedbackEvent>
	  // Component API
	  ,
	  public ItemManager {

  private:
	item_impl::Context context;

  public:
	ItemSystem(core::LogContext& log, std::size_t max_objects, StatsManager const& stats);

	void handle(ItemEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game
