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

// ---------------------------------------------------------------------------

// FoV Query Traverser
struct FovQuery {
	MovementManager const & move_manager;
	FocusManager const & focus_manager;
	MovementData const & actor_move;
	FocusData const & actor_focus;
	utils::Collider collider;
	ObjectID focus;
	float best_value;
	
	FovQuery(FocusData const & actor_focus, MovementData const & actor_move, MovementManager const & move_manager, FocusManager const & focus_manager);
	
	sf::IntRect getRange() const;
	void operator()(sf::Vector2f const & pos, std::vector<ObjectID> const & cell);
};

/// Query focused object
/// @param actor ObjectID for whom to query
/// @param dungeon Dungeon to query in
/// @param focus_manager Manager of multiple FocusData
/// @param movement_manager Manager of multiple MovementData
/// @return focused entity's ID
ObjectID getFocus(ObjectID actor, Dungeon const & dungeon,
	FocusManager const & focus_manager, MovementManager const & movement_manager);

} // ::focus_impl

// ---------------------------------------------------------------------------
// Focus System

/// Handles objects referring to focus
/// The focus is NOT updated automatically but on demand. So this system does
/// not do very much anymore.
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

}  // ::core
