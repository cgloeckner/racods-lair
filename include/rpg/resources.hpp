#pragma once
#include <vector>
#include <tuple>
#include <SFML/System.hpp>

#include <utils/keybinding.hpp>
#include <utils/lighting_system.hpp>
#include <utils/xml_utils.hpp>
#include <core/entity.hpp>
#include <rpg/common.hpp>

namespace rpg {

/// BaseResource that can be cached
/**
 *	A base resource holds an internal name, which specifies the actual
 *	filename after dropping all path-related data from it. It is used as base
 *	for all non-third-party game resources, which are loaded from XML. To do
 *	so, all base resources implement methods to load from or save to an XML
 *	tree. All base resource provide basic functionallity to `loadFromFile` and
 *	`saveToFile` through these virtual methods.
 */
struct BaseResource {
	std::string internal_name;
	mutable std::string last_error;

	BaseResource();

	virtual void loadFromTree(utils::ptree_type const& ptree) = 0;
	virtual void saveToTree(utils::ptree_type& ptree) const = 0;

	/// Load the resource from the given file
	/**
	 *	This will load the resource from the given file by parsing its XML
	 *	content. If the file cannot be loaded or parsed (e.g. by missing or
	 *	corrupted data), the method will return false - otherwise true.
	 *
	 *	@param fname Path to the resource file
	 *	@return true if loading was successful
	 */
	bool loadFromFile(std::string const& fname);

	/// Save the resource to the given file
	/**
	 *	This will save the resource to the given file by duping its content to
	 *	XML. If the content cannot be dumped (e.g. by corrputed data) or the
	 *	file cannot be written, the method will return false - otherwise true.
	 *
	 *	@param fname Path to the resource file
	 *	@return true if saving was successful
	 */
	bool saveToFile(std::string const& fname) const;
};

// ---------------------------------------------------------------------------

/// Helper function to parse an enumeration map from a property tree
/**
 *	This will aprse an enumeration map from the property tree. Each enum value
 *	is converted from string using a `from_string<Enum>()` overload. The
 *	corresponding value is parsed from the XML attribute, keyed by stringifyed
 *	enum value. This operation requires all enum values to have
 *	representations within the specified property tree. No children nodes are
 *	considered. All values are loaded from the given prefix similar to
 *	<prefix enum0="value0" enum1="value1" .. />
 *
 *	@param ptree PropertyTree to parse from
 *	@param map EnumMap to parse to
 *	@param prefix Children path to parse from
 */
template <typename Enum, typename T>
void parse(utils::ptree_type const& ptree, utils::EnumMap<Enum, T>& map,
	std::string const& prefix);

template <typename Enum, typename T, typename Handle>
void parseEnumMap(utils::ptree_type const & ptree, utils::EnumMap<Enum, T>& map,
	std::string const& prefix, Handle func);

/// Overload to the default enumeration map parse
/**
 *	This works just as the default enumeration map parse function, but with
 *	one difference: If a node doesn't exist, the given default_value is used.
 *
 *	@param ptree PropertyTree to parse from
 *	@param map EnumMap to parse to
 *	@param prefix Children path to parse from
 *	@param default_value used for unexisting nodes while parsing
 */
template <typename Enum, typename T>
void parse(utils::ptree_type const& ptree, utils::EnumMap<Enum, T>& map,
	std::string const& prefix, T default_value);

void parse(utils::ptree_type const & ptree, sf::Color& color);

/// Helper function to dump an enumeration map to a property tree
/**
 *	This will dump an enumeration map to the property tree. Each enum value
 *	is converted to string using a non-standard `to_string()` overload. The
 *	corresponding value is dumped using the property tree's default
 *	mechanisms. Each pair is mapped to an XML attribute, so no children nodes
 *	are created. The the values are written to the given prefix similar to
 *	<prefix enum0="value0" enum1="value1" .. />
 *
 *	@param ptree PropertyTree to dump to
 *	@param map EnumMap to dump from
 *	@param prefix Children path to dump to
 */
template <typename Enum, typename T>
void dump(utils::ptree_type& ptree, utils::EnumMap<Enum, T> const& map,
	std::string const& prefix);

template <typename Enum, typename T, typename Handle>
void dumpEnumMap(utils::ptree_type& ptree, utils::EnumMap<Enum, T> const & map,
	std::string const& prefix, Handle func);

/// Overload to the default enumeration map dump
/**
 *	This works just as the default enumeration map dump function, but with
 *	one difference: If a node's value equals the given default_value, the node
 *	will not be dumped to the property tree.
 *
 *	@param ptree PropertyTree to dump to
 *	@param map EnumMap to dump from
 *	@param prefix Children path to dump to
 *	@param default_value specifies the nodes to ignore while dumping
 */
template <typename Enum, typename T>
void dump(utils::ptree_type& ptree, utils::EnumMap<Enum, T> const& map,
	std::string const& prefix, T default_value);

void dump(utils::ptree_type& ptree, sf::Color const & color);

// ---------------------------------------------------------------------------

/// Contains profile-specific keybindings
/**
 *	This data never provides a gamepad id, despite it is describing keyboard
 *	or gamepad input. If it describes gamepad input, the actual gamepad id
 *	will be applied when creating the player's input component.
 *
 *	@note Valid keybindings do not mix keyboard and gamepad input. But this is
 *	not checked here, because this is only from a resource's point of view.
 *	@note Keybindings are never cached.
 */
struct Keybinding: BaseResource {
	bool is_gamepad;
	
	utils::Keybinding<PlayerAction> map;

	void loadFromTree(utils::ptree_type const& ptree);
	void saveToTree(utils::ptree_type& ptree) const;
};

// ---------------------------------------------------------------------------

/// Contains tileset-specific data
/**
 *	Those data are shared between multiple tilesets.
 */
struct TilesetTemplate : BaseResource {
	std::string tileset_name;
	sf::Vector2u tilesize;
	std::vector<sf::Vector2u> floors, walls;
	sf::Texture const* tileset;

	TilesetTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

// ---------------------------------------------------------------------------

struct SpriteTemplate : BaseResource {
	utils::ActionFrames legs;
	utils::EnumMap<core::AnimationAction, utils::ActionFrames> torso;
	std::string frameset_name;
	sf::Texture const* frameset;
	std::vector<utils::Edge> edges;

	SpriteTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;

	bool isAnimated() const;
};

// ---------------------------------------------------------------------------

/// Contains entity-specific data
/**
 *	Those data are shared between multiple game objects.
 */
struct EntityTemplate : BaseResource {
	using SoundNode = std::pair<std::string, sf::SoundBuffer const *>;
	
	bool is_projectile, collide, flying;
	float max_sight, radius, max_speed, fov;
	std::string display_name, sprite_name;
	utils::Collider shape;
	SpriteTemplate const* sprite;
	utils::EnumMap<core::SoundAction, std::vector<SoundNode>> sounds;
	std::unique_ptr<utils::Light> light;
	std::unique_ptr<rpg::InteractType> interact;
	sf::Color blood_color;

	EntityTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;

	bool hasSounds() const;
};

// ---------------------------------------------------------------------------

struct StatsBoni {
	utils::EnumMap<Property, int> properties;
	utils::EnumMap<DamageType, float> defense;

	StatsBoni();
};

// ---------------------------------------------------------------------------

/// Contains effect-specific data
/**
 *	Those data are shared between multiple game objects.
 */
struct EffectTemplate : BaseResource {
	std::string display_name, inflict_sound;
	sf::Time duration;
	sf::SoundBuffer const* sound;
	StatsBoni boni;
	utils::EnumMap<Stat, float> recover;
	utils::EnumMap<DamageType, float> damage;

	EffectTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

// ---------------------------------------------------------------------------

struct EffectEmitter {
	std::string name;
	float ratio;
	EffectTemplate const* effect;

	EffectEmitter();

	void loadFromTree(utils::ptree_type const& ptree);
	void saveToTree(utils::ptree_type& ptree) const;
};

// ---------------------------------------------------------------------------

/// Contains bullet-specific data
/**
 *	Those data are used by the object factory.
 */
struct BulletTemplate : BaseResource {
	std::string entity_name;
	float radius;
	EntityTemplate const* entity;

	BulletTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

// ---------------------------------------------------------------------------

struct BulletEmitter {
	std::string name;
	BulletTemplate const* bullet;
	sf::Color color;

	BulletEmitter();

	void loadFromTree(utils::ptree_type const& ptree);
	void saveToTree(utils::ptree_type& ptree) const;
};

// ---------------------------------------------------------------------------

/// Contains item-specific data
/**
 *	Those data are shared between multiple game objects.
 */
struct ItemTemplate : BaseResource {
	ItemType type;
	std::string display_name, icon_name, use_sound, sprite_name;
	EquipmentSlot slot;
	bool melee, two_handed;
	unsigned int worth;
	BulletEmitter bullet;
	EffectEmitter effect;
	sf::Texture const* icon;						// use when quickslotted
	sf::SoundBuffer const* sound;					// used when used
	SpriteTemplate const* sprite;					// used when equipped
	utils::EnumMap<DamageType, float> damage;		// as equipment
	utils::EnumMap<Attribute, unsigned int> require;		// for equipment
	utils::EnumMap<Stat, int> recover;			// by potion
	bool revive;									// by potion
	StatsBoni boni;

	ItemTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

// ---------------------------------------------------------------------------

/// Contains perk-specific data
/**
 *	Those data are shared between multiple game objects.
 *
 *	note: A perk with type "Self" or "Allied" applies an effect (heal,
 *	buff etc.) to the target. A perk with type "Enemy" creates a bullet.
 */
struct PerkTemplate : BaseResource {
	PerkType type;
	std::string display_name, icon_name, use_sound;
	bool revive;  // as defensive perk
	BulletEmitter bullet;
	EffectEmitter effect;
	sf::Texture const* icon;	   // use when quickslotted
	sf::SoundBuffer const* sound;  // used when used
	utils::EnumMap<DamageType, float> damage;
	utils::EnumMap<Stat, float> recover;

	PerkTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

// ---------------------------------------------------------------------------

/// Contains trap-specific data
/**
 *	Those data are shared between multiple game objects.
 */
struct TrapTemplate : BaseResource {
	std::string trigger_sound;
	BulletEmitter bullet;
	EffectEmitter effect;
	sf::SoundBuffer const* sound;
	utils::EnumMap<DamageType, unsigned int> damage;

	TrapTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

}  // ::game

// include implementation details
#include <rpg/resources.inl>
