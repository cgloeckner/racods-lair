#pragma once
#include <utils/input_mapper.hpp>
#include <core/dungeon.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

// ---------------------------------------------------------------------------
// Private Input API

namespace input_impl {

/// Cooldown in ms for toggling auto-look
extern unsigned int const TOGGLE_COOLDOWN;

/// Context of the input handling
struct Context {
	core::LogContext& log;
	core::InputSender& input_sender;
	ActionSender& action_sender;
	utils::InputMapper mapper;
	core::DungeonSystem const& dungeon;
	core::MovementManager const& movement;
	core::FocusManager const& focus;

	std::vector<PlayerAction> gameplay_actions;

	Context(core::LogContext& log, core::InputSender& input_sender,
		ActionSender& action_sender, core::DungeonSystem const& dungeon,
		core::MovementManager const& movement, core::FocusManager const& focus);
};

// ---------------------------------------------------------------------------

/// Shortcut to determine whether the given action is activated for this object
/**
 *	This shortcut returns true if the given action is currently active for the
 *	specified object. This is done by querying the context's mapper.
 *
 *	@param context Input context to deal with
 *	@param data InputData that specifies the object's controls
 *	@param action Player action that should be checked
 *	@return true if the given action is currently active
 */
bool isActive(
	Context const& context, InputData const& data, PlayerAction action);

/// Generates an input event for the given object
/**
 *	This will return an input event that describes the object's upcomming
 *	behavior which is determined by its controls.
 *	If the object's auto_look is enabled, the looking direction will be set
 *	to the specified movement direction. If movement and looking are applied
 *	simultaneously, the auto_look is disabled. If auto_look is disabled, the
 *	last looking direction is not automatically adjusted to the movement
 *	direction while moving.
 *	Additionally, an object can perform an action (also while moving or
 *	looking). But this action never equals `ToggleAutoLook`, because this
 *	action is always handled internally.
 *
 *	@post event.idle is true if no action is performed
 *	@param context Input context to deal with
 *	@param data InputData that specifies the object's controls
 *	@param event InputEvent to populate
 *	@param action ActionEvent to populated
 */
void queryInput(Context const& context, InputData& data,
	core::InputEvent& input_event, ActionEvent& action_event);

/// Update the given object's input
/**
 *	This will create an input event. But the event is only propagated if a
 *	movement, looking or an action is detected. If one of those conditions is
 *	satisfied, the event is forwarded. E.g. moving and attacking is forwarded.
 *	Looking-only is also forwarded. Pure idle is not forwarded.
 *
 *	@param context Input context to deal with
 *	@param data InputData that specifies the object's controls
 *	@param elapsed Time since last update
 */
void updateInput(Context& context, InputData& data, sf::Time const& elapsed);

/// Modifies movement if necessary
/**
 *	This modifies the movement vector if necessary and possible by rotating
 *	it either clockwise or counterclockwise using 45 degree. If no change is
 *	necessary, the vector is not changed. Only tile collision is checked here.
 *
 *	@param context Input context to deal with
 *	@param data InputData to fix for
 *	@param vector Movement vector to modify.
 */
void adjustMovement(
	Context const& context, InputData const& data, sf::Vector2i& vector);

/// Handle actor's death
/**
 *	This handels the actor's death by disabling his inputs. Once he is
 *	respawned, they can be enabled for further use.
 *
 *	@param data InputData to update
 */
void onDeath(InputData& data);

/// Handle actor's respawn
/**
 *	This handels the actor's respawn by enabling his inputs.
 *
 *	@param data InputData to update
 */
void onSpawn(InputData& data);

}  // ::input_impl

// ---------------------------------------------------------------------------
// Public Input API

/// The Input System triggers players' actions
/**
 *	The Input System will generate InputEvents on each player action. If the
 *	player wants to move or change looking direction, this is propagated to
 *	further systems. If the player wants to perform actions, suitable other
 *	events like ItemEvent etc. are propagated. Actions can only be applied if
 *	the object is idling, yet. The system is notified about each SFML-Event in
 *	order to keep its own state of all input devices. The input state can be
 *	`reset()`.
 */
class InputSystem
	// Event API
	: public utils::EventListener<DeathEvent, SpawnEvent>,
	  public utils::EventSender<core::InputEvent, ActionEvent>
	  // Component API
	  ,
	  public InputManager {

  private:
	input_impl::Context context;

  public:
	InputSystem(core::LogContext& log, std::size_t max_objects, core::DungeonSystem const& dungeon,
		core::MovementManager const& movement, core::FocusManager const& focus);

	void reset();
	
	void handle(DeathEvent const& event);
	void handle(SpawnEvent const& event);
	void handle(sf::Event const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game
