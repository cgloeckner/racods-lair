#pragma once
#include <functional>

#include <utils/delay_system.hpp>
#include <core/entity.hpp>
#include <core/event.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>
#include <game/common.hpp>
#include <game/builder.hpp>
#include <game/mod.hpp>
#include <game/navigator.hpp>
#include <game/powerup.hpp>
#include <game/resources.hpp>
#include <game/session.hpp>

namespace game {

extern std::uint32_t const MIN_BOT_ATTRIB;

extern float const PLAYER_LIGHT_RADIUS;
extern sf::Uint8 const PLAYER_LIGHT_INTENSITY;
extern sf::Color const PLAYER_LIGHT_COLOR;

extern float const PLAYER_ADVANTAGE_FACTOR;

extern std::size_t const MAX_POWERUP_SPAWN_DRIFT;

using BuilderModifier = std::function<void(DungeonBuilder& builder)>;

// --------------------------------------------------------------------

namespace factory_impl {

bool canHoldPowerup(Session const & session, utils::SceneID scene, sf::Vector2f const & pos, core::ObjectID ignore=0u);

} // ::factory_impl

// --------------------------------------------------------------------

/// Object and Dungeon Factory
/**
 *	This factory is used to manage the lifetime of objects and create dungeons
 *	during runtime. Additionally, it works as a listener for several gameplay
 *	events that cause object destruction. So the factory will release objects
 *	as soon as it receives a suitable event.
 */
class Factory
	: public utils::EventListener<rpg::ProjectileEvent, rpg::DeathEvent,
		  rpg::SpawnEvent, ReleaseEvent>,
	  public utils::EventSender<core::InputEvent, rpg::ActionEvent,
		  rpg::ItemEvent, rpg::StatsEvent, rpg::SpawnEvent, PowerupEvent> {

  private:
	struct EntityCache {
		rpg::EntityTemplate const * entity;
		bool hostile;
		
		EntityCache();
	};
	
	core::LogContext& log;
	std::size_t const max_num_players;
	Session& session;
	Mod& mod;
	std::vector<EntityCache> entity_cache; // by id
	
	utils::DelaySystem<core::ObjectID> release;
	rpg::PlayerID latest_player;

	void setupObject(core::ObjectID id, rpg::EntityTemplate const & entity);

  public:
	sf::Texture const * blood_texture;
	rpg::EntityTemplate const * gem_tpl;
	
	/// Create a new factory
	/**
	 *	A new factory will be created that deals with the given logging
	 *	context, session and pathfinding API.
	 *
	 *	@param log Reference to the logging context
	 *	@param session Reference to active session
	/// @param mod Reference to mod manager
	 */
	Factory(core::LogContext& log, Session& session, Mod& mod);

	/// Handle bullet explosion
	/**
	 *	This handles an incomming bullet explosion in a factory context. The
	 *	bullet's movement will be stopped and its collision component will be
	 *	dropped. After delaying some milliseconds, the entire bullet will be
	 *	removed from the system.
	 *
	 *	@param id Bullet's object ID
	 */
	void onBulletExploded(core::ObjectID id);

	/// Create a new dungeon
	/// This creates a new dungeon. The dungeon content will be randomly
	/// generated. The modifier callback is supposed to be used in
	/// editor mode.
	/// @param tileset Tileset reference to use
	/// @param grid_size Total dungeon size
	/// @param settings BuildSettings to use for dungeon
	/// @param modifier|empty Callback to modify DungeonBuilder
	/// @return scene id to identify the dungeon
	utils::SceneID createDungeon(rpg::TilesetTemplate const& tileset,
		sf::Vector2u grid_size, BuildSettings const & settings,
		BuilderModifier modifier=[](DungeonBuilder&){});

	/// Create an ambience sprite
	/// The sprite will get a random offset and rotation
	/// @param texture Texture to use
	/// @param data Metadata for spawning
	/// @param color Color used for the sprite (default: white)
	/// @param angle|-1.f Rotation of the sprite
	void createAmbience(sf::Texture const & texture,
		rpg::SpawnMetaData const & data,
		sf::Color const & color=sf::Color::White);

	/// Create a new base object
	/// This creates a new base object. It will be spawned using the given
	/// metadata. If the object is interactable, it will be created
	/// with proper settings.
	/// @param entity Entity reference that specifies the object properties
	/// @param data Metadata for spawning
	/// @return object id to identify the object
	core::ObjectID createObject(
		rpg::EntityTemplate const& entity, rpg::SpawnMetaData const& data);

	/// Create a new bullet object
	/**
	 *	This creates a new bullet object. A corresponding base object is
	 *	created automatically. The entity template is specified by the
	 *	combat metadata. A bullet can have an owner, that id is also passed
	 *	here. If a zero id is passed, the bullet has no owner.
	 *
	 *	@param meta Combat metadata that specifies the bullet properties
	 *	@param data Metadata for spawning
	 *	@param owner Owner's object ID
	 *	@return object id to identify the object.
	 */
	core::ObjectID createBullet(rpg::CombatMetaData const& meta,
		rpg::SpawnMetaData const& spawn, core::ObjectID owner);

	/// Create a new bot object
	/**
	 *	This creates a new bot object. A corresponding base object is created
	 *	automatically. The entity template is specified by the given bot
	 *	template. Each bot can be either hostile or not. A hostile bot is
	 *	supposed to behave like a pure enemy; non-hostile bots can be used as
	 *	player's minions.
	 *
	 *	@param bot Bot reference that specifies the object
	 *	@param data Metadata used for spawning
	 *	@param level Level of the bot
	 *	@param script Script reference to use for AI behavior
	 *	@param hostile Determines whether enemy or player's minion
	/// @param difficulty parameter to modify bot strength
	 *	@return object id to identify the object
	 */
	core::ObjectID createBot(BotTemplate const& bot,
		rpg::SpawnMetaData const& data, std::size_t level,
		/*utils::Script const& script,*/ bool hostile, float difficulty=1.f);

	/// Create a new player object
	/// This creates a new player object. A corresponding base object is
	/// created automatically. The entity template is specified by the given
	/// player template. The player's id is created automatically.
	/// @param player Player reference that specifies the object
	/// @param keys Keybinding to use
	/// @param data Metadata used for spawning
	/// @param color|Transparent Color used for multiplayer highlighting
	/// @return object id to identify the object
	core::ObjectID createPlayer(PlayerTemplate const & player,
		rpg::Keybinding const & keys, rpg::SpawnMetaData const & data,
		sf::Color color=sf::Color::Transparent);
	
	/// Create a powerup object
	/// This creates a powerup object that is triggered once a player
	/// enters the position.
	/// @param entity EntityTemplate to use
	/// @param spawn SpawnData
	/// @param type PowerupType
	core::ObjectID createPowerup(rpg::EntityTemplate const & entity,
		rpg::SpawnMetaData const & spawn, PowerupType type);
	
	/// Destroy the given object
	/**
	 *	The given object will vanish from the game. All components will be
	 *	dropped and the id is freed for reuse.
	 *
	 *	@param id Object id
	 */
	void destroyObject(core::ObjectID id);

	/// Create teleport trigger
	/// Add a teleport trigger for teleporting from the source scene
	/// at the src position to the target scene to the dst position.
	/// The teleporter is NOT bidirectional by default.
	///
	/// @param source SceneID of the source scene
	/// @param src Position to trigger at
	/// @param target SceneID of the target scene
	/// @param dst Position to teleport to
	void addTeleport(utils::SceneID source, sf::Vector2f const & src,
		utils::SceneID target, sf::Vector2f const & dst);
	
	/// Handle an incomming projectile event
	/**
	 *	An incomming projectile event can cause bullet creation or destruction.
	 *	The corresponding method is called here.
	 *
	 *	@param event Projectile event to deal with
	 */
	void handle(rpg::ProjectileEvent const& event);

	/// Handle an incomming death event
	/// An incomming death event will cause the character's death. The
	/// death is only handled within the factory's context. When dying,
	/// the character's body is moved to the bottom rendering layer,
	/// its collision and stats components will be dropped and it will
	/// be made unfocusable. Also all non-torso animation layers are
	/// cleared.
	///
	/// @param event Death event to deal with
	void handle(rpg::DeathEvent const& event);
	
	/// Handle an incomming respawn event
	/// An incomming respawn event will end the character's death. The
	/// respawn is only handled within the factory's context. When
	/// respawned, the character's body is moved to its original layer,
	/// its collision and stats componetns will be recreated and it
	/// will be made focusable (if it was before). All animation layers
	/// are restored, too. This requires holding the template reference
	/// for each object.
	///
	/// @param event Spawn event to deal with
	void handle(rpg::SpawnEvent const& event);
	
	/// Handle an incomming object release
	/// This will schedule the object for later release
	///
	/// @param event Release event to deal with
	void handle(ReleaseEvent const& event);

	/// Update the factory
	/**
	 *	Each update will perform event transition, advance and perform delayed
	 *	object's destruction.
	 *
	 *	@param elapsed Elapsed time since last update
	 */
	void update(sf::Time const& elapsed);

	/// Reset factory's internal state
	/// This effects the delay system and the template cache.
	void reset();
};

}  // ::rage
