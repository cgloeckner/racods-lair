	#include <boost/algorithm/string.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <utils/binary_utils.hpp>
#include <game/resources.hpp>
#include <game/lua.hpp>

namespace game {

GeneratorSettings::GeneratorSettings()
	: rpg::BaseResource{}
	, cell_size{31u}
	, room_density{0.5f}
	, deadend_density{0.1f}
	, ambience_density{0.25f}
	, redundant_paths_ratio{0.25f} {}

void GeneratorSettings::loadFromTree(utils::ptree_type const& ptree) {
	cell_size = ptree.get<unsigned int>("<xmlattr>.cell_size");
	if (cell_size % 2 == 0) {
		// force cell size to be an odd value
		++cell_size;
	}
	room_density = ptree.get<float>("<xmlattr>.room_density");
	deadend_density = ptree.get<float>("<xmlattr>.deadend_density");
	ambience_density = ptree.get<float>("<xmlattr>.ambience_density");
	redundant_paths_ratio = ptree.get<float>("<xmlattr>.redundant_paths_ratio");
}

void GeneratorSettings::saveToTree(utils::ptree_type& ptree) const {
	ptree.put("<xmlattr>.cell_size", cell_size);
	ptree.put("<xmlattr>.room_density", room_density);
	ptree.put("<xmlattr>.deadend_density", deadend_density);
	ptree.put("<xmlattr>.ambience_density", ambience_density);
	ptree.put("<xmlattr>.redundant_paths_ratio", redundant_paths_ratio);
}

void GeneratorSettings::verify() const {
	ASSERT(cell_size >= 3u);
	ASSERT(room_density > 0.f);
	ASSERT(room_density <= 1.f);
	ASSERT(deadend_density >= 0.f);
	ASSERT(deadend_density <= 1.f);
	ASSERT(ambience_density >= 0.f);
	ASSERT(ambience_density <= 1.f);
	ASSERT(room_density + deadend_density <= 1.f); // fails for MinGW 4.9.2 due to floating point issue
	ASSERT(redundant_paths_ratio >= 0.f);
	ASSERT(redundant_paths_ratio <= 1.f);
}

// ---------------------------------------------------------------------------

BotTemplate::BotTemplate()
	: rpg::BaseResource{}
	, entity_name{}
	, color{sf::Color::White}
	, attributes{0.f}
	, defense{0.f}
	, properties{0.f}
	, items{}
	, perks{}
	, entity{nullptr} {}

void BotTemplate::loadFromTree(utils::ptree_type const& ptree) {
	// parse general data
	display_name = ptree.get<std::string>("general.<xmlattr>.display_name");
	entity_name = ptree.get<std::string>("general.<xmlattr>.entity_name");
	auto color_opt = ptree.get_child_optional("general.color");
	if (color_opt) {
		rpg::parse(*color_opt, color);
	} else {
		color = sf::Color::White;
	}
	// parse attributes
	rpg::parse(ptree, attributes, "attributes");
	// parse boni
	rpg::parse(ptree, defense, "defense", 0.f);
	rpg::parse(ptree, properties, "properties", 0.f);
	// parse items
	utils::parse_vector(ptree, "items", "item", items,
		[](utils::ptree_type const& child, ItemNode& node) {
			std::get<0>(node) = child.get<std::string>("<xmlattr>.item");
			std::get<1>(node) = child.get<float>("<xmlattr>.quantity");
			std::get<2>(node) = nullptr;
		});
	// parse perks
	utils::parse_vector(ptree, "perks", "perk", perks,
		[](utils::ptree_type const& child, PerkNode& node) {
			std::get<0>(node) = child.get<std::string>("<xmlattr>.perk");
			std::get<1>(node) = child.get<float>("<xmlattr>.level");
			std::get<2>(node) = nullptr;
		});
}

void BotTemplate::saveToTree(utils::ptree_type& ptree) const {
	// dump general data
	ptree.put("general.<xmlattr>.display_name", display_name);
	ptree.put("general.<xmlattr>.entity_name", entity_name);
	utils::ptree_type c;
	rpg::dump(c, color);
	ptree.add_child("general.color", c);
	// dump attributes
	rpg::dump(ptree, attributes, "attributes");
	// dump boni
	rpg::dump(ptree, defense, "defense", 0.f);
	rpg::dump(ptree, properties, "properties", 0.f);
	// dump items
	utils::dump_vector(ptree, "items", "item", items,
		[](utils::ptree_type& child, ItemNode const& node) {
			child.put("<xmlattr>.item", std::get<0>(node));
			child.put("<xmlattr>.quantity", std::get<1>(node));
		});
	// dump perks
	utils::dump_vector(ptree, "perks", "perk", perks,
		[](utils::ptree_type& child, PerkNode const& node) {
			child.put("<xmlattr>.perk", std::get<0>(node));
			child.put("<xmlattr>.level", std::get<1>(node));
		});
}

// --------------------------------------------------------------------

EncounterTemplate::EncounterTemplate()
	: rpg::BaseResource{}
	, bots{} {
}

EncounterTemplate::Node::Node()
	: filename{}
	, ratio{0.f}
	, ptr{nullptr} {
}

EncounterTemplate::Node::Node(std::string const & filename, float ratio, BotTemplate const * ptr)
	: filename{filename}
	, ratio{ratio}
	, ptr{ptr} {
}

void EncounterTemplate::loadFromTree(utils::ptree_type const& ptree) {
	utils::parse_vector(ptree, "encounter", "bot", bots,
		[](utils::ptree_type const & child, Node& node) {
			node.filename = child.get<std::string>("<xmlattr>.name");
			node.ratio = child.get<float>("<xmlattr>.ratio");
	});
}

void EncounterTemplate::saveToTree(utils::ptree_type& ptree) const {
	utils::dump_vector(ptree, "encounter", "bot", bots,
		[](utils::ptree_type& child, Node const & node) {
			child.put("<xmlattr>.name", node.filename);
			child.put("<xmlattr>.ratio", node.ratio);
	});
}

BotTemplate const & EncounterTemplate::pick(float v) const {
	ASSERT(v >= 0.f);
	ASSERT(v <= 1.f);
	float sum{0.f};
	BotTemplate const * found{nullptr};
	for (auto const & node: bots) {
		sum += node.ratio;
		if (v <= sum) {
			found = node.ptr;
			break;
		}
	}
	ASSERT(found != nullptr);
	return *found;
}

// --------------------------------------------------------------------

RoomTemplate::RoomTemplate()
	: rpg::BaseResource{}
	, cells{} {
}

RoomTemplate::EntityNode::EntityNode()
	: name{}
	, direction{0, 1}
	, ptr{nullptr} {
}

RoomTemplate::RoomCell::RoomCell()
	: wall{false}
	, entity{} {
}

std::size_t RoomTemplate::VectorHasher::operator()(sf::Vector2u const & v) const {
	std::hash<unsigned int> func;
	return func(v.x) ^ func(v.y);
}

void RoomTemplate::loadFromTree(utils::ptree_type const& ptree) {
	utils::parse_map(ptree, "grid", "cell", cells,
		[](utils::ptree_type const & child, sf::Vector2u& pos, RoomCell& room) {
		// parse position
		pos.x = child.get<unsigned int>("<xmlattr>.x");
		pos.y = child.get<unsigned int>("<xmlattr>.y");
		
		// parse wall flag
		room.wall = child.get<bool>("<xmlattr>.wall", false);
		
		// parse entity
		room.entity = RoomTemplate::EntityNode{};
		room.entity.name = child.get<std::string>("<xmlattr>.entity", "");
		auto buf = child.get<std::string>("<xmlattr>.face", "");
		if (!buf.empty()) {
			auto pos = buf.find(",");
			ASSERT(pos != std::string::npos);
			room.entity.direction.x = std::stoi(buf.substr(0, pos));
			room.entity.direction.y = std::stoi(buf.substr(pos+1));
		}
	});
}

void RoomTemplate::saveToTree(utils::ptree_type& ptree) const {
	utils::dump_map(ptree, "grid", "cell", cells,
		[](utils::ptree_type& child, sf::Vector2u const & pos, RoomCell const & room) {
		// dump position
		child.put("<xmlattr>.x", pos.x);
		child.put("<xmlattr>.y", pos.y);
		
		// dump wall flag
		if (room.wall) {
			child.put("<xmlattr>.wall", room.wall);
		}
		
		// dump entity
		if (!room.entity.name.empty()) {
			child.put("<xmlattr>.entity", room.entity.name);
			
			auto dump = std::to_string(room.entity.direction.x) + ","
				+ std::to_string(room.entity.direction.y);
			child.put("<xmlattr>.face", dump);
		}
	});
}

RoomTemplate::RoomCell& RoomTemplate::create(sf::Vector2u const & pos) {
	return cells[pos] = RoomTemplate::RoomCell{};
}

void RoomTemplate::destroy(sf::Vector2u const & pos) {
	auto i = cells.find(pos);
	if (i != cells.end()) {
		cells.erase(i);
	}
}

bool RoomTemplate::isValid(utils::Logger& log, unsigned int max_room_size) const {
	bool ok{true};
	
	auto expectNonFloor = [&](sf::Vector2u const & pos) {
		if (cells.find(pos) != cells.end()) {
			ok = false;
			log << pos << " must not contain a floor tile\n";
		}
	};
	
	// test: floor doesn't neighbor the room's outside
	for (auto x = 0u; x < max_room_size; ++x) {
		expectNonFloor({x, 0u});
		expectNonFloor({x, max_room_size-1u});
	}
	for (auto y = 0u; y < max_room_size; ++y) {
		expectNonFloor({0u, y});
		expectNonFloor({max_room_size-1u, y});
	}
	
	// test: all cells are within bounds
	for (auto const & pair: cells) {
		if (pair.first.x >= max_room_size || pair.first.y >= max_room_size) {
			ok = false;
			log << pair.first << " is outside cell bounds\n";
		}
	}
	
	return ok;
}


bool operator==(RoomTemplate const & lhs, RoomTemplate const & rhs) {
	if (lhs.cells.size() != rhs.cells.size()) {
		return false;
	}
	for (auto const & node: lhs.cells) {
		auto i = rhs.cells.find(node.first);
		if (i == rhs.cells.end()) {
			return false;
		}
		auto const & a = i->second;
		auto const & b = node.second;
		if (i->first != node.first || a.wall != b.wall ||
			a.entity.name != b.entity.name ||
			a.entity.ptr != b.entity.ptr) {
			return false;
		}
	}
	return true;
}


bool operator!=(RoomTemplate const & lhs, RoomTemplate const & rhs) {
	return !(lhs == rhs);
}

// ---------------------------------------------------------------------------

AiScript::AiScript() : utils::Script{} { bindAll(*this); }

// ---------------------------------------------------------------------------

PlayerTemplate::PlayerTemplate()
	: display_name{}
	, entity_name{}
	, entity{nullptr}
	, inventory{}
	, equipment{""}
	, equip_ptr{nullptr}
	, perks{}
	, level{0u}
	, attributes{0u}
	, slot_id{0u}
	, slots{}
	, exp{0ul}
	, attrib_points{0u}
	, perk_points{0u} {
}

void PlayerTemplate::fetch(rpg::ItemData const & item, rpg::PerkData const & perk, rpg::StatsData const & stats, rpg::QuickslotData const & qslot, rpg::PlayerData const & player) {
	inventory.clear();
	for (auto const & pair: item.inventory) {
		for (auto const & i: pair.second) {
			ASSERT(i.item != nullptr);
			inventory.emplace_back(i.item->internal_name, i.quantity, i.item);
		}
	}
	for (auto const & pair: item.equipment) {
		equipment[pair.first] = pair.second != nullptr ? pair.second->internal_name : "";
	}
	
	// fetch perks
	perks.clear();
	for (auto const & p: perk.perks) {
		ASSERT(p.perk != nullptr);
		perks.emplace_back(p.perk->internal_name, p.level, p.perk);
	}
	
	// fetch stats
	level = stats.level;
	attributes = stats.attributes;
	
	// fetch quickslots
	slot_id = qslot.slot_id;
	for (auto i = 0u; i < rpg::MAX_QUICKSLOTS; ++i) {
		auto& shortcut = qslot.slots[i];
		auto& target = slots[i];
		std::get<0>(target) = "";
		std::get<1>(target) = ""; 
		std::get<2>(target) = nullptr;
		std::get<3>(target) = nullptr;
		if (shortcut.perk != nullptr) {
			std::get<1>(target) = shortcut.perk->internal_name;
			std::get<3>(target) = shortcut.perk;
		} else if (shortcut.item != nullptr) {
			std::get<0>(target) = shortcut.item->internal_name;
			std::get<2>(target) = shortcut.item;
		}
	}
	
	// fetch player
	exp = player.exp;
	attrib_points = player.attrib_points;
	perk_points = player.perk_points;
}

void PlayerTemplate::loadFromPacket(sf::Packet& stream) {
	PlayerTemplate tmp;
	stream >> tmp.display_name >> tmp.entity_name;
	
	utils::parse(stream, tmp.inventory, [&](ItemNode& elem) {
		stream >> std::get<0>(elem);
		stream >> std::get<1>(elem);
	});
	utils::parse(stream, tmp.equipment, [&](std::string& string) {
		stream >> string;
	});
	
	utils::parse(stream, tmp.perks, [&](PerkNode& elem) {
		stream >> std::get<0>(elem);
		stream >> std::get<1>(elem);
	});
	
	stream >> tmp.level;
	utils::parse(stream, tmp.attributes, [&](std::uint32_t& v) {
		stream >> v;
	});
	
	stream >> tmp.slot_id;
	utils::parse(stream, tmp.slots, [&](ShortcutNode& node) {
		stream >> std::get<0>(node);
		stream >> std::get<1>(node);
	});
	
	stream >> tmp.exp >> tmp.attrib_points >> tmp.perk_points;
	
	std::swap(*this, tmp);
}

bool PlayerTemplate::loadFromFile(std::string const & filename) {
	auto stream = utils::loadBinaryFile(filename);
	loadFromPacket(stream);
	return true;
}

sf::Packet PlayerTemplate::saveToPacket() const {
	sf::Packet stream;
	
	stream << display_name << entity_name;
	
	utils::dump(stream, inventory, [&](ItemNode const & elem) {
		stream << std::get<0>(elem);
		stream << std::get<1>(elem);
	});
	utils::dump(stream, equipment, [&](std::string const & string) {
		stream << string;
	});
	
	utils::dump(stream, perks, [&](PerkNode const & elem) {
		stream << std::get<0>(elem);
		stream << std::get<1>(elem);
	});
	
	stream << level;
	utils::dump(stream, attributes, [&](std::uint32_t const & v) {
		stream << v;
	});
	
	stream << slot_id;
	utils::dump(stream, slots, [&](ShortcutNode const & node) {
		stream << std::get<0>(node);
		stream << std::get<1>(node);
	});
	
	stream << exp << attrib_points << perk_points;
	
	return stream;
}

bool PlayerTemplate::saveToFile(std::string const & filename) const {
	auto stream = saveToPacket();
	utils::saveBinaryFile(stream, filename);
	return true;
}

// --------------------------------------------------------------------

Localization::Localization()
	: rpg::BaseResource{}
	, data{} {
}

std::string Localization::operator()(std::string const & key, std::string const & fallback) const {
	// interpret last subkey as xml attribute
	auto pos = key.rfind(".");
	if (pos == std::string::npos) {
		return fallback;
	}
	auto query = key.substr(0u, pos) + ".<xmlattr>" + key.substr(pos);
	// query string
	if (fallback.empty()) {
		return data.get<std::string>(query, key);
	} else {
		return data.get<std::string>(query, fallback);
	}
}

void Localization::loadFromTree(utils::ptree_type const& ptree) {
	data = ptree;
}

void Localization::saveToTree(utils::ptree_type& ptree) const {
	ptree = data;
}

std::string Localization::getFilename() const {
	return "locale.xml";
}

}  // ::rage

// ---------------------------------------------------------------------------

namespace utils {

template class MultiResourceCache<
	// System resources
	sf::Texture, sf::Font, sf::SoundBuffer, game::AiScript,
	// Game resources
	rpg::TilesetTemplate, rpg::SpriteTemplate, rpg::EntityTemplate,
	rpg::EffectTemplate, rpg::BulletTemplate, rpg::ItemTemplate,
	rpg::PerkTemplate, rpg::TrapTemplate, game::BotTemplate,
	game::RoomTemplate>;

void bindAll(Script& script) {
	script.bind<sf::Vector2u>();
	script.bind<sf::Vector2i>();
	
	script.bind<rpg::EquipmentSlot>();
	script.bind<rpg::ItemType>();
	script.bind<rpg::PerkType>();
	script.bind<rpg::FeedbackType>();
	
	script.bind<rpg::StatsBoni>();
	script.bind<rpg::EffectTemplate>();
	script.bind<rpg::ItemTemplate>();
	script.bind<rpg::Item>();
	script.bind<rpg::PerkTemplate>();
	script.bind<rpg::Perk>();
	script.bind<rpg::StatsData>();
	
	script.bind<game::LuaApi>();
}

}  // ::utils
