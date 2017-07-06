#pragma once
#include <vector>
#include <tuple>
#include <unordered_map>
#include <SFML/Audio.hpp>
#include <SFML/Network/Packet.hpp>

#include <utils/logger.hpp>
#include <utils/lua_utils.hpp>
#include <utils/resource_cache.hpp>
#include <utils/xml_utils.hpp>

#include <rpg/entity.hpp>
#include <rpg/resources.hpp>
#include <rpg/gameplay.hpp>
#include <game/common.hpp>

namespace game {

/// Combines several dungeon generator settings
/// cell_size describes the size of each grid cell
/// room_density describes how many rooms are created
/// deadend_density describes how many deadends are created
/// ambience_density describes how many ambiences are created
/// redudant_paths_ratio describes how many redundant paths are created
struct GeneratorSettings : rpg::BaseResource {
	unsigned int cell_size;
	float room_density, deadend_density, ambience_density, redundant_paths_ratio;

	/// Default settings
	GeneratorSettings();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;

	/// Verify the generator settings
	/// @pre cell_size >= 3
	/// @pre room density in (0,1]
	/// @pre deadend density in [0,1]
	/// @pre ambience density in [0,1]
	/// @pre room density + deadend density <= 1
	/// @pre redudant paths ratio in [0,1]
	void verify() const;
};

// ---------------------------------------------------------------------------

/// Contains bot-specific data
/**
 *	Those data are used by the object factory. All floating point values are
 *	related to the actual bot level.
 */
struct BotTemplate : rpg::BaseResource {
	using ItemNode =
		std::tuple<std::string, std::size_t, rpg::ItemTemplate const*>;
	using PerkNode = std::tuple<std::string, float, rpg::PerkTemplate const*>;

	std::string display_name, entity_name;
	sf::Color color;
	utils::EnumMap<rpg::Attribute, float> attributes;
	utils::EnumMap<rpg::DamageType, float> defense; // boni
	utils::EnumMap<rpg::Property, float> properties; // boni
	std::vector<ItemNode> items;
	std::vector<PerkNode> perks;
	rpg::EntityTemplate const* entity;

	BotTemplate();

	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

// --------------------------------------------------------------------

struct EncounterTemplate: rpg::BaseResource {
	struct Node {
		std::string filename;
		float ratio;
		BotTemplate const * ptr;
		
		Node();
		Node(std::string const & filename, float ratio, BotTemplate const * ptr=nullptr);
	};
	
	std::vector<Node> bots;
	
	EncounterTemplate();
	
	BotTemplate const & pick(float v) const;
	
	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
};

// --------------------------------------------------------------------

/// Contains room-specific data
struct RoomTemplate: rpg::BaseResource {
	struct EntityNode {
		std::string name;
		sf::Vector2i direction;
		rpg::EntityTemplate const * ptr;
		
		EntityNode();
	};
	
	struct RoomCell {
		bool wall;
		
		EntityNode entity;
		
		RoomCell();
	};
	
	struct VectorHasher {
		std::size_t operator()(sf::Vector2u const & v) const;
	};
	
	std::unordered_map<sf::Vector2u, RoomCell, VectorHasher> cells;
	
	RoomTemplate();
	
	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
	
	RoomCell& create(sf::Vector2u const & pos);
	void destroy(sf::Vector2u const & pos);
	
	bool isValid(utils::Logger& log, unsigned int max_room_size) const;
};

bool operator==(RoomTemplate const & lhs, RoomTemplate const & rhs);
bool operator!=(RoomTemplate const & lhs, RoomTemplate const & rhs);

// ---------------------------------------------------------------------------

/// Contains ai-specific scripting data
struct AiScript : utils::Script {
	AiScript();
};

// ---------------------------------------------------------------------------

/// Contains player-specific data
struct PlayerTemplate {
	using ShortcutNode = std::tuple<std::string, std::string,
		rpg::ItemTemplate const*, rpg::PerkTemplate const*>;
	using ItemNode = std::tuple<std::string, std::uint32_t, rpg::ItemTemplate const*>;
	using PerkNode = std::tuple<std::string, std::uint32_t, rpg::PerkTemplate const*>;

	// general data
	std::string display_name, entity_name;
	rpg::EntityTemplate const* entity;
	// item data
	std::vector<ItemNode> inventory;
	utils::EnumMap<rpg::EquipmentSlot, std::string> equipment;
	utils::EnumMap<rpg::EquipmentSlot, rpg::ItemTemplate const*> equip_ptr;
	// perk data
	std::vector<PerkNode> perks;
	// stats data
	std::uint32_t level;
	utils::EnumMap<rpg::Attribute, std::uint32_t> attributes;
	// quickslot data
	std::uint8_t slot_id;
	std::array<ShortcutNode, rpg::MAX_QUICKSLOTS> slots;
	// player data
	sf::Uint64 exp;
	std::uint32_t attrib_points, perk_points;

	PlayerTemplate();
	
	void fetch(rpg::ItemData const & item, rpg::PerkData const & perk,
		rpg::StatsData const & stats, rpg::QuickslotData const & qslot,
		rpg::PlayerData const & player);

	void loadFromPacket(sf::Packet& stream);
	sf::Packet saveToPacket() const;

	bool loadFromFile(std::string const & filename);
	bool saveToFile(std::string const & filename) const;
};

// --------------------------------------------------------------------

class Localization
	: public rpg::BaseResource {
  private:
	utils::ptree_type data;
	
  public:
	Localization();
	
	std::string operator()(std::string const & key, std::string const & fallback="") const;
	
	void loadFromTree(utils::ptree_type const& ptree) override;
	void saveToTree(utils::ptree_type& ptree) const override;
	
	std::string getFilename() const;
};

// ---------------------------------------------------------------------------

/// Type Alias for resource cache
using ResourceCache = utils::MultiResourceCache<
	// System resources
	sf::Texture, sf::Font, sf::SoundBuffer,
	// Game resources
	rpg::TilesetTemplate, rpg::SpriteTemplate, rpg::EntityTemplate,
	rpg::EffectTemplate, rpg::BulletTemplate, rpg::ItemTemplate,
	rpg::PerkTemplate, rpg::TrapTemplate, BotTemplate, EncounterTemplate,
	RoomTemplate>;

}  // ::rage
