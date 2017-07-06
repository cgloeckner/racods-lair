#pragma once
#include <functional>

#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>
#include <core/entity.hpp>

namespace core {

/// max sight for focus checking
extern float const MAX_SIGHT;

namespace focus_impl {

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	FocusSender& focus_sender;
	FocusManager& focus_manager;
	DungeonSystem& dungeon_system;
	MovementManager const& movement_manager;

	Context(LogContext& log, FocusSender& focus_sender,
		FocusManager& focus_manager, DungeonSystem& dungeon_system,
		MovementManager const& movement_manager);
};

}  // ::focus_impl

// ---------------------------------------------------------------------------
// Focus System

/// Handles objects' looking direction and focusing other objects
/**
 *	Each object with a focus component is able to look into a direction.
 *	Furthermore it is able to focus another object that is also able to focus.
 *	This implies players and enemies need to have a focus component, but also
 *	chests, levlers and doors. Each focusable object has a looking direction,
 *	a focused object (references by the focused object's id or zero) and a
 *	list of objects (referenced by their ids) that are focusing the actor.
 *	When moving or rotating, the objects focus will be updated. When changing
 *	the focus, additional events are propagated and the corresponding objects
 *	are updated as well.
 */
class FocusSystem
	// Event API
	: public utils::EventListener<InputEvent, MoveEvent>,
	  public utils::EventSender<FocusEvent>
	  // Component API
	  ,
	  public FocusManager {

  protected:
	focus_impl::Context context;

  public:
	FocusSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon,
		MovementManager const& movement_manager);

	void handle(InputEvent const& event);
	void handle(MoveEvent const& event);

	void update(sf::Time const& elapsed);
};

// ---------------------------------------------------------------------------
// Internal Focus API

namespace focus_impl {

/// Traverses all cells with given depth in a direction and applies the handle
/**
 *	This function is used to search for specific objects by traversing all
 *	cells from a given origin in the given direction. The `handle` lambda
 *	is called on each object and returns true if the object is appropriate.
 *	If so, the object's id is returned immediately.
 *	Otherwise, if no appropirate object was found, the function stops at the
 *	target depth and returns 0 as non-object's id.
 *
 *	@param dungeon Corresponding spatial scene to search at
 *	@param pos Starting position for the traversal
 *	@param dir Traversal direction to step from cell to cell
 *	@param depth Maximum traversal depth
 *	@param handle Lambda to determine whether an object is appropriate
 *	@return Object's id or 0 if none found
 */
ObjectID traverseCells(Dungeon const& dungeon, sf::Vector2u pos,
	sf::Vector2i const& dir, float depth,
	std::function<bool(ObjectID, int)> handle);

void setFocus(Context const& context, FocusData& observer, FocusData* observed);

/// Updates all focusing if an object changed its looking direction
/**
 *	When an object changes its looking direction, it will change its object
 *	focus. Therefore the previous observable needs to be notified and a new
 *	observable needs to be determined and notified as well. This is done here.
 *	Also, the focus component's looking direction is updated here.
 *	Briefly, this function will change the actors focus and notify its
 *	observables.
 *
 *	@pre The actor has a movement component
 *	@pre The actor is attached to a scene
 *	@param context Focusing context to work with
 *	@param data Focus component of the actor object
 *	@param event Input event that triggered changing the looking direction
 */
void onLook(Context const& context, FocusData& data, InputEvent const& event);

/// Update all focusing if an object starts moving to another tile
/**
 *	After an object started leaving a tile to move to another one, its
 *	observation state will change. Therefore the previous observers need to be
 *	notified and new observers need to be determined and notified as well.
 *	This is done here.
 *	Briefly, this function will change the observer list and notify each of
 *	them.
 *
 *	@pre The actor has a movement component
 *	@pre The actor is attached to a scene
 *	@param context Focusing context to work with
 *	@param data Focus component of the actor object
 *	@param event Move event that triggered changing the cell
 */
void onMove(Context& context, FocusData& data, MoveEvent const& event);

}  // ::focus_impl

}  // ::core
