#pragma once
#include <core/dungeon.hpp>
#include <core/entity.hpp>
#include <core/event.hpp>
#include <core/collision.hpp>

namespace core {

/// Spawn an object at specific position within a dungeon
/**
 *	This operation requires the object to be unspawned at any dungeon. It will
 *	correctly spawn the object at the dungeon. The given position is used to
 *	specify the target cell. No collision detection is done here. When
 *	spawning the object, the dungeon's collision grid and the object's
 *	movement data are updated.
 *
 *	@pre data.scene == 0u
 *	@param dungeon Dungeon to spawn at
 *	@param data MovementData of the object to spawn
 *	@param pos Position to spawn at
 */
void spawn(Dungeon& dungeon, MovementData& data, sf::Vector2f const& pos);

/// Vanish the given object from the dungeon
/**
 *	This operation requires the obejct to be already spawned at the given
 *	dungeon. It will correctly remove the obejct from the dungeon. The
 *	object's current position is used to specify the source cell, which yet
 *	holds the object. When vanishing the object, the dungeon's collision grid
 *	and the object's movement data are updated.
 *
 *	@pre data.scene == dungeon.id
 *	@param dungeon Dungeon to vanish from
 *	@param data MovementData of the object to vanish
 */
void vanish(Dungeon& dungeon, MovementData& data);

// ---------------------------------------------------------------------------

struct SpawnHelper {
	CollisionManager const& collision;
	MovementManager const & movement;
	Dungeon const& dungeon;
	ObjectID actor;
	
	CollisionResult result;

	SpawnHelper(CollisionManager const& collision, MovementManager const & movement,
		Dungeon const& dungeon, ObjectID actor);
	bool operator()(sf::Vector2f const& pos);
};

struct TriggerHelper {
	Dungeon const& dungeon;

	TriggerHelper(Dungeon const& dungeon);
	bool operator()(sf::Vector2f const& pos);
};

template <typename Pred>
bool getFreePosition(Pred pred, sf::Vector2f& pos, std::size_t max_drift = 0u);

// ---------------------------------------------------------------------------

/// This realizes teleporting an object to the specified target
/**
 *	Some additional data like event senders, component managers and the
 *	corresponding dungeon system are required by reference, in order to
 *	correctly move the actor while teleport.
 *
 *	To realize more high-level stuff while teleporting, a derived teleport
 *	trigger can be written and created in the actual game.
 */
class TeleportTrigger : public BaseTrigger {

  private:
	TeleportSender& teleport_sender;
	MovementManager& movement;
	CollisionManager const& collision;
	DungeonSystem& dungeon;

  protected:
	utils::SceneID const target;
	sf::Vector2f const pos;

  public:
	/// Create a new teleport trigger
	/**
	 *	This will bind the given event senders, component managers and the
	 *	dungeon system in order to perform teleports. The target dungeon
	 *	and position are also specified by the arguments.
	 *	In order to notify the focus system about "leaving a tile", the
	 *	event is supposed to be parts of the collision system.
	 *
	/// @param teleport_sender EventSender for TeleportEvents
	 *	@param movement ComponentManager for MovementData
	 *	@param collision ComponentManger for CollisionData
	 *	@param dungeon DungeonSystem that will be used
	 *	@param target SceneID of the teleport's target dungeon
	 *	@param pos Target position within the target dungeon.
	 */
	TeleportTrigger(TeleportSender& teleport_sender, MovementManager& movement,
		CollisionManager const& collision, DungeonSystem& dungeon,
		utils::SceneID target, sf::Vector2f pos);

	/// Execute the actual teleport
	/**
	 *	This will move the specified actor from its current position and
	 *	dungeon to the specified ones. If the specified position is not
	 *	accessable, a near by location is searched. If found, the telport
	 *	is executed.
	 *	The MoveSender is notified about entering the new tile. In order
	 *	to make the focus system able to renew the focus, a TileLeft is
	 *	propagated. The object is stopped on teleport.
	 *
	 *	@param actor ObjectID of the actor object.
	 */
	virtual void execute(core::ObjectID actor) override;
	
	/// Returns false because teleporters do not expire
	bool isExpired() const override;
};

}  // ::core

// include implementation details
#include <core/teleport.inl>
