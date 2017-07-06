#include <boost/algorithm/string.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <utils/algorithm.hpp>
#include <rpg/resources.hpp>

namespace rpg {

BaseResource::BaseResource()
	: internal_name{}
	, last_error{} {
}

bool BaseResource::loadFromFile(std::string const& fname) {
	// parse xml
	utils::ptree_type ptree;
	try {
		boost::property_tree::read_xml(fname, ptree);
	} catch (std::exception const & error) {
		last_error = error.what();
		std::cerr << last_error << std::endl;
		return false;
	}
	// load data
	// note: no rollback implemented, here!
	try {
		loadFromTree(ptree);
	} catch (utils::ptree_error const& error) {
		last_error = error.what();
		std::cerr << last_error << std::endl;
		return false;
	}
	return true;
}

bool BaseResource::saveToFile(std::string const& fname) const {
	// save data
	utils::ptree_type ptree;
	try {
		saveToTree(ptree);
	} catch (utils::ptree_error const& error) {
		last_error = error.what();
		std::cerr << last_error << std::endl;
		return false;
	}
	// dump xml
	try {
		boost::property_tree::xml_writer_settings<char> settings('\t', 1);
		boost::property_tree::write_xml(fname, ptree, std::locale(), settings);
	} catch (std::exception const & error) {
		last_error = error.what();
		std::cerr << last_error << std::endl;
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------

void parse(utils::ptree_type const & ptree, sf::Color& color) {
	color = sf::Color{};
	color.r = ptree.get<sf::Uint8>("<xmlattr>.red");
	color.g = ptree.get<sf::Uint8>("<xmlattr>.green");
	color.b = ptree.get<sf::Uint8>("<xmlattr>.blue");
}

void dump(utils::ptree_type& ptree, sf::Color const & color) {
	ptree.put("<xmlattr>.red", color.r);
	ptree.put("<xmlattr>.green", color.g);
	ptree.put("<xmlattr>.blue", color.b);
}

// ---------------------------------------------------------------------------

void Keybinding::loadFromTree(utils::ptree_type const& ptree) {
	std::size_t keys{0u}, pads{0u};
	
	for (auto action : utils::EnumRange<PlayerAction>{}) {
		utils::InputAction input;
		// parse action
		auto& item = ptree.get_child(to_string(action));
		auto type = item.get<std::string>("<xmlattr>.type");
		if (type == "Key") {
			auto key = item.get<std::string>("<xmlattr>.key");
			input = utils::InputAction{thor::toKeyboardKey(key)};
			++keys;
		} else if (type == "Axis") {
			auto axis = item.get<std::string>("<xmlattr>.axis");
			auto threshold = item.get<float>("<xmlattr>.threshold");
			// note: gamepad_id is not loaded from a profile
			input =
				utils::InputAction{0u, thor::toJoystickAxis(axis), threshold};
			++pads;
		} else if (type == "Button") {
			auto button = item.get<unsigned int>("<xmlattr>.button");
			// note: gamepad_id is not loaded from a profile
			input = utils::InputAction{0u, button};
			++pads;
		} else {
			throw utils::ptree_error{"Invalid input type '" + type + "'"};
		}
		// set action
		map.set(action, input);
	}
	
	/*
	if (keys > 0u && pads > 0u) {
		throw utils::ptree_error{"Inconsistent input type"};
	}
	*/
	is_gamepad = (pads > 0u);
}

void Keybinding::saveToTree(utils::ptree_type& ptree) const {
	utils::EnumMap<PlayerAction, int> helper;

	for (auto action : utils::EnumRange<PlayerAction>{}) {
		// get action
		auto input = map.get(action);
		// dump action
		utils::ptree_type item;
		switch (input.type) {
			case utils::InputAction::Key:
				item.put("<xmlattr>.type", "Key");
				item.put("<xmlattr>.key", thor::toString(input.key.key));
				break;
			case utils::InputAction::Axis:
				item.put("<xmlattr>.type", "Axis");
				item.put("<xmlattr>.axis", thor::toString(input.axis.axis));
				item.put("<xmlattr>.threshold", input.axis.threshold);
				// note: gamepad_id is not saved to a profile
				break;
			case utils::InputAction::Button:
				item.put("<xmlattr>.type", "Button");
				item.put("<xmlattr>.button", input.button.button);
				// note: gamepad_id is not saved to a profile
				break;
		}
		ptree.add_child(to_string(action), item);
	}
}

// ---------------------------------------------------------------------------

TilesetTemplate::TilesetTemplate()
	: BaseResource{}
	, tileset_name{}
	, tilesize{}
	, floors{}
	, walls{}
	, tileset{nullptr} {}

void TilesetTemplate::loadFromTree(utils::ptree_type const& ptree) {
	auto load_options = [](utils::ptree_type const& ptree,
		std::string const& tag, std::vector<sf::Vector2u>& arr) {
		arr.reserve(ptree.size());
		utils::parse_vector(ptree, tag, "offset", arr,
			[](utils::ptree_type const& child, sf::Vector2u& elem) {
				elem.x = child.get<unsigned int>("<xmlattr>.x");
				elem.y = child.get<unsigned int>("<xmlattr>.y");
			});
	};

	tileset_name = ptree.get<std::string>("general.<xmlattr>.filename");
	// parse tilesize
	auto node = ptree.get_child("tilesize");
	tilesize.x = node.get<int>("<xmlattr>.width");
	tilesize.y = node.get<int>("<xmlattr>.height");
	// parse terrain
	load_options(ptree, "floors", floors);
	load_options(ptree, "walls", walls);
}

void TilesetTemplate::saveToTree(utils::ptree_type& ptree) const {
	auto save_options = [](utils::ptree_type& ptree, std::string const& tag,
		std::vector<sf::Vector2u> const& arr) {
		utils::ptree_type array;
		utils::dump_vector(ptree, tag, "offset", arr,
			[](utils::ptree_type& child, sf::Vector2u const& elem) {
				child.put("<xmlattr>.x", elem.x);
				child.put("<xmlattr>.y", elem.y);
			});
	};

	ptree.put("general.<xmlattr>.filename", tileset_name);
	// dump tilesize
	ptree.put("tilesize.<xmlattr>.width", tilesize.x);
	ptree.put("tilesize.<xmlattr>.height", tilesize.y);
	// dump terrain
	save_options(ptree, "floors", floors);
	save_options(ptree, "walls", walls);
}

// ---------------------------------------------------------------------------

SpriteTemplate::SpriteTemplate()
	: BaseResource{}
	, legs{}
	, torso{}
	, frameset_name{}
	, frameset{nullptr}
	, edges{} {}

void SpriteTemplate::loadFromTree(utils::ptree_type const& ptree) {
	auto load_frames = [](utils::ptree_type const& ptree,
		std::string const& root_tag, utils::ActionFrames& frames) {
		frames.frames.reserve(ptree.size());
		utils::parse_vector(ptree, root_tag, "frame", frames.frames,
			[](utils::ptree_type const& child, utils::Frame& elem) {
				elem.clip.left = child.get<int>("<xmlattr>.left");
				elem.clip.top = child.get<int>("<xmlattr>.top");
				elem.clip.width = child.get<int>("<xmlattr>.width");
				elem.clip.height = child.get<int>("<xmlattr>.height");
				elem.origin.x = child.get<float>("<xmlattr>.x");
				elem.origin.y = child.get<float>("<xmlattr>.y");
				elem.duration =
					sf::milliseconds(child.get<int>("<xmlattr>.duration"));
			});
		frames.refresh();
	};
	
	// parse general data
	frameset_name = ptree.get<std::string>("general.<xmlattr>.filename");
	// parse legs
	auto legs_opt_node = ptree.get_child_optional("legs");
	if (legs_opt_node) {
		load_frames(ptree, "legs", legs);
	}
	// parse torso
	for (auto& pair : torso) {
		auto key = "torso." + to_string(pair.first);
		auto opt_node = ptree.get_child_optional(key);
		if (opt_node) {
			load_frames(ptree, key, pair.second);
		}
	}
	// parse shadow
	auto shadow_opt_node = ptree.get_child_optional("shadow");
	if (shadow_opt_node) {
		// parse points
		std::vector<sf::Vector2f> points;
		utils::parse_vector(ptree, "shadow", "point", points,
			[](utils::ptree_type const & child, sf::Vector2f& point) {
				point.x = child.get<float>("<xmlattr>.x");
				point.y = child.get<float>("<xmlattr>.y");
		});
		// create edges
		edges.clear();
		std::size_t i{0u};
		while (i + 1 < points.size()) {
			utils::Edge e;
			e.u = points[i];
			e.v = points[i+1];
			edges.push_back(e);
			++i;
		}
		utils::Edge e;
		e.u = points.back();
		e.v = points.front();
		edges.push_back(e);
	}
}

void SpriteTemplate::saveToTree(utils::ptree_type& ptree) const {
	auto save_frames = [](utils::ptree_type& ptree, std::string const& root_tag,
		utils::ActionFrames const& frames) {
		utils::dump_vector(ptree, root_tag, "frame", frames.frames,
			[](utils::ptree_type& child, utils::Frame const& elem) {
				child.put("<xmlattr>.left", elem.clip.left);
				child.put("<xmlattr>.top", elem.clip.top);
				child.put("<xmlattr>.width", elem.clip.width);
				child.put("<xmlattr>.height", elem.clip.height);
				child.put("<xmlattr>.x", elem.origin.x);
				child.put("<xmlattr>.y", elem.origin.y);
				child.put("<xmlattr>.duration", elem.duration.asMilliseconds());
			});
	};

	// dump general data
	ptree.put<std::string>("general.<xmlattr>.filename", frameset_name);
	// dump legs
	if (!legs.frames.empty()) {
		save_frames(ptree, "legs", legs);
	}
	// dump torso
	for (auto const & pair : torso) {
		if (!pair.second.frames.empty()) {
			save_frames(ptree, "torso." + to_string(pair.first), pair.second);
		}
	}
	// dump shadow
	if (!edges.empty()) {
		std::vector<sf::Vector2f> points;
		for (auto const & edge: edges) {
			points.push_back(edge.u);
		}
		utils::dump_vector(ptree, "shadow", "point", points,
			[](utils::ptree_type& child, sf::Vector2f const & point) {
				child.put("<xmlattr>.x", point.x);
				child.put("<xmlattr>.y", point.y);
		});
	}
}

bool SpriteTemplate::isAnimated() const {
	if (!legs.frames.empty()) {
		// is animated, because it has legs
		return true;
	}
	for (auto const& pair : torso) {
		if (pair.first == core::AnimationAction::Idle) {
			if (pair.second.frames.size() > 1u) {
				// is animated, because it has multiple idle frames
				return true;
			}
		} else {
			if (pair.second.frames.size() > 0u) {
				// is animated, because non-idle actions are supported
				return true;
			}
		}
	}
	// is not animated
	return false;
}

// ---------------------------------------------------------------------------

EntityTemplate::EntityTemplate()
	: BaseResource{}
	, is_projectile{false}
	, collide{false}
	, flying{false}
	, max_sight{0.f}
	, max_speed{0.f}
	, display_name{}
	, sprite_name{}
	, sprite{nullptr}
	, sounds{}
	, light{nullptr}
	, interact{nullptr}
	, blood_color{sf::Color::Transparent} {
}

void EntityTemplate::loadFromTree(utils::ptree_type const& ptree) {
	// parse general data
	is_projectile = ptree.get<bool>("general.<xmlattr>.is_projectile");
	collide = ptree.get<bool>("general.<xmlattr>.collide");
	flying = ptree.get<bool>("general.<xmlattr>.flying", false);
	max_sight = ptree.get<float>("general.<xmlattr>.max_sight");
	max_speed = ptree.get<float>("general.<xmlattr>.max_speed");
	display_name = ptree.get<std::string>("general.<xmlattr>.display_name", "");
	sprite_name = ptree.get<std::string>("general.<xmlattr>.sprite_name");
	// parse interact data
	auto buffer = ptree.get<std::string>("general.<xmlattr>.interact", "");
	interact = nullptr;
	if (!buffer.empty()) {
		auto key = buffer.substr(0, 1);
		boost::algorithm::to_upper(key);
		key += buffer.substr(1);
		interact = std::make_unique<rpg::InteractType>(rpg::from_string<rpg::InteractType>(key));
	}
	// parse sound data
	parseEnumMap(ptree, sounds, "sounds", [](utils::ptree_type const & child, std::vector<SoundNode>& array) {
		utils::parse_vector(child, "sound", array, [](utils::ptree_type const & p, SoundNode& node) {
			node.first = p.get<std::string>("<xmlattr>.name", "");
			node.second = nullptr;
		});
	});
	// parse light data
	auto light_node = ptree.get_child_optional("light");
	light = nullptr;
	if (light_node) {
		light = std::make_unique<utils::Light>();
		parse(*light_node, light->color);
		light->radius = light_node->get<float>("<xmlattr>.radius");
		light->intensity = light_node->get<sf::Uint8>("<xmlattr>.intensity");
		light->cast_shadow = light_node->get<bool>("<xmlattr>.cast_shadow");
		light->lod = light_node->get<std::size_t>("<xmlattr>.lod");
	}
	// parse blood data
	auto blood_node = ptree.get_child_optional("blood");
	if (blood_node) {
		parse(*blood_node, blood_color);
	} else {
		blood_color = sf::Color::Transparent;
	}
}

void EntityTemplate::saveToTree(utils::ptree_type& ptree) const {
	// dump general data
	ptree.put("general.<xmlattr>.is_projectile", is_projectile);
	ptree.put("general.<xmlattr>.collide", collide);
	ptree.put("general.<xmlattr>.flying", flying);
	ptree.put("general.<xmlattr>.max_sight", max_sight);
	ptree.put("general.<xmlattr>.max_speed", max_speed);
	ptree.put("general.<xmlattr>.sprite_name", sprite_name);
	ptree.put("general.<xmlattr>.display_name", display_name);
	if (interact != nullptr) {
		ptree.put("general.<xmlattr>.interact", rpg::to_string(*interact));
	}
	// dump sound data
	dumpEnumMap(ptree, sounds, "sounds", [](utils::ptree_type& child, std::vector<SoundNode> const & array) {
		utils::dump_vector(child, "sound", array, [](utils::ptree_type& p, SoundNode const & node) {
			p.put("<xmlattr>.name", node.first);
		});
	});
	
	// dump light data
	if (light != nullptr) {
		utils::ptree_type p;
		dump(p, light->color);
		ptree.add_child("light", p);
		ptree.put("light.<xmlattr>.radius", light->radius);
		ptree.put("light.<xmlattr>.intensity", light->intensity);
		ptree.put("light.<xmlattr>.cast_shadow", light->cast_shadow);
		ptree.put("light.<xmlattr>.lod", light->lod);
	}
	
	// dump blood color
	if (blood_color != sf::Color::Transparent) {
		utils::ptree_type p;
		dump(p, blood_color);
		ptree.add_child("blood", p);
	}
}

bool EntityTemplate::hasSounds() const {
	for (auto const& pair : sounds) {
		for (auto const & node: pair.second) {
			if (!node.first.empty() || node.second != nullptr) {
				// has a sound name
				return true;
			}
		}
	}
	// has no sound
	return false;
}

// ---------------------------------------------------------------------------

StatsBoni::StatsBoni()
	: properties{0}
	, defense{0.f} {
}

// ---------------------------------------------------------------------------

EffectTemplate::EffectTemplate()
	: BaseResource{}
	, display_name{}
	, inflict_sound{}
	, duration{sf::Time::Zero}
	, sound{nullptr}
	, boni{}
	, recover{0.f}
	, damage{0.f} {}

void EffectTemplate::loadFromTree(utils::ptree_type const& ptree) {
	// parse general data
	display_name = ptree.get<std::string>("general.<xmlattr>.display_name");
	inflict_sound =
		ptree.get<std::string>("general.<xmlattr>.inflict_sound", "");
	duration =
		sf::milliseconds(ptree.get<int>("general.<xmlattr>.duration", 0));

	parse(ptree, recover, "recover", 0.f);
	parse(ptree, damage, "damage", 0.f);

	parse(ptree, boni.defense, "boni.defense", 0.f);
	parse(ptree, boni.properties, "boni.properties", 0);
}

void EffectTemplate::saveToTree(utils::ptree_type& ptree) const {
	// dump general data
	ptree.put("general.<xmlattr>.display_name", display_name);
	if (!inflict_sound.empty()) {
		ptree.put("general.<xmlattr>.inflict_sound", inflict_sound);
	}
	if (duration != sf::Time::Zero) {
		ptree.put("general.<xmlattr>.duration", duration.asMilliseconds());
	}

	dump(ptree, recover, "recover", 0.f);
	dump(ptree, damage, "damage", 0.f);

	dump(ptree, boni.defense, "boni.defense", 0.f);
	dump(ptree, boni.properties, "boni.properties", 0);
}

// ---------------------------------------------------------------------------

EffectEmitter::EffectEmitter() : name{}, ratio{0.f}, effect{nullptr} {}

void EffectEmitter::loadFromTree(utils::ptree_type const& ptree) {
	name = ptree.get<std::string>("<xmlattr>.name");
	ratio = ptree.get<float>("<xmlattr>.ratio");
}

void EffectEmitter::saveToTree(utils::ptree_type& ptree) const {
	ptree.put("<xmlattr>.name", name);
	ptree.put("<xmlattr>.ratio", ratio);
}

// ---------------------------------------------------------------------------

BulletTemplate::BulletTemplate()
	: BaseResource{}, entity_name{}, radius{0.f}, entity{nullptr} {}

void BulletTemplate::loadFromTree(utils::ptree_type const& ptree) {
	// parse general data
	entity_name = ptree.get<std::string>("general.<xmlattr>.entity_name");
	radius = ptree.get<float>("general.<xmlattr>.radius");
}

void BulletTemplate::saveToTree(utils::ptree_type& ptree) const {
	// dump general data
	ptree.put("general.<xmlattr>.entity_name", entity_name);
	ptree.put("general.<xmlattr>.radius", radius);
}

// ---------------------------------------------------------------------------

BulletEmitter::BulletEmitter()
	: name{}
	, bullet{nullptr}
	, color{sf::Color::White} {
}

void BulletEmitter::loadFromTree(utils::ptree_type const& ptree) {
	name = ptree.get<std::string>("<xmlattr>.name");
	
	auto color_node = ptree.get_child_optional("color");
	if (color_node) {
		parse(*color_node, color);
	} else {
		color = sf::Color::White;
	}
}

void BulletEmitter::saveToTree(utils::ptree_type& ptree) const {
	ptree.put("<xmlattr>.name", name);
	
	if (color != sf::Color::White) {
		utils::ptree_type p;
		dump(p, color);
		ptree.add_child("color", p);
	}
}

// ---------------------------------------------------------------------------

ItemTemplate::ItemTemplate()
	: BaseResource{}
	, type{default_value<ItemType>()}
	, display_name{}
	, icon_name{}
	, use_sound{}
	, sprite_name{}
	, slot{default_value<EquipmentSlot>()}
	, melee{false}
	, two_handed{false}
	, worth{0u}
	, bullet{}
	, effect{}
	, icon{nullptr}
	, sound{nullptr}
	, sprite{nullptr}
	, damage{0.f}
	, require{0.f}
	, recover{0.f}
	, revive{false}
	, boni{} {}

void ItemTemplate::loadFromTree(utils::ptree_type const& ptree) {
	// parse general data
	type =
		from_string<ItemType>(ptree.get<std::string>("general.<xmlattr>.type"));
	display_name = ptree.get<std::string>("general.<xmlattr>.display_name");
	icon_name = ptree.get<std::string>("general.<xmlattr>.icon_name");
	use_sound = ptree.get<std::string>("general.<xmlattr>.use_sound", "");
	sprite_name = ptree.get<std::string>("general.<xmlattr>.sprite_name", "");
	worth = ptree.get<unsigned int>("general.<xmlattr>.worth");
	// parse equipment data
	slot = from_string<EquipmentSlot>(
		ptree.get<std::string>("equip.<xmlattr>.slot"));
	if (slot == EquipmentSlot::Weapon) {
		melee = ptree.get<bool>("equip.<xmlattr>.melee");
		two_handed = ptree.get<bool>("equip.<xmlattr>.two_handed");
	}
	// parse bullet data
	auto bullet_node = ptree.get_child_optional("bullet");
	if (bullet_node) {
		bullet.loadFromTree(*bullet_node);
	}
	// parse effect data
	auto effect_node = ptree.get_child_optional("effect");
	if (effect_node) {
		effect.loadFromTree(*effect_node);
	}

	parse(ptree, damage, "damage", 0.f);
	parse(ptree, require, "require", 0u);
	parse(ptree, recover, "recover", 0);
	revive = ptree.get<bool>("general.<xmlattr>.revive", false);

	parse(ptree, boni.defense, "boni.defense", 0.f);
	parse(ptree, boni.properties, "boni.properties", 0);
}

void ItemTemplate::saveToTree(utils::ptree_type& ptree) const {
	// dump general data
	ptree.put("general.<xmlattr>.type", to_string(type));
	ptree.put("general.<xmlattr>.display_name", display_name);
	ptree.put("general.<xmlattr>.icon_name", icon_name);
	if (!use_sound.empty()) {
		ptree.put("general.<xmlattr>.use_sound", use_sound);
	}
	if (!sprite_name.empty()) {
		ptree.put("general.<xmlattr>.sprite_name", sprite_name);
	}
	ptree.put("general.<xmlattr>.worth", worth);
	// dump equipment data
	ptree.put("equip.<xmlattr>.slot", to_string(slot));
	if (slot == EquipmentSlot::Weapon) {
		ptree.put("equip.<xmlattr>.melee", melee);
		ptree.put("equip.<xmlattr>.two_handed", two_handed);
	}
	// dump bullet data
	utils::ptree_type child;
	bullet.saveToTree(child);
	ptree.put_child("bullet", child);
	// dump effect data
	utils::ptree_type child2;
	effect.saveToTree(child2);
	ptree.put_child("effect", child2);

	dump(ptree, damage, "damage", 0.f);
	dump(ptree, require, "require", 0u);
	dump(ptree, recover, "recover", 0);
	ptree.put("general.<xmlattr>.revive", revive);

	dump(ptree, boni.defense, "boni.defense", 0.f);
	dump(ptree, boni.properties, "boni.properties", 0);
}

// ---------------------------------------------------------------------------

PerkTemplate::PerkTemplate()
	: BaseResource{}
	, type{default_value<PerkType>()}
	, display_name{}
	, icon_name{}
	, use_sound{}
	, revive{false}
	, bullet{}
	, effect{}
	, icon{nullptr}
	, sound{nullptr}
	, damage{0.f}
	, recover{0.f} {}

void PerkTemplate::loadFromTree(utils::ptree_type const& ptree) {
	// parse general data
	type =
		from_string<PerkType>(ptree.get<std::string>("general.<xmlattr>.type"));
	display_name = ptree.get<std::string>("general.<xmlattr>.display_name");
	icon_name = ptree.get<std::string>("general.<xmlattr>.icon_name");
	use_sound = ptree.get<std::string>("general.<xmlattr>.use_sound", "");
	revive = ptree.get<bool>("general.<xmlattr>.revive", false);
	// parse bullet data
	auto bullet_node = ptree.get_child_optional("bullet");
	if (bullet_node) {
		bullet.loadFromTree(*bullet_node);
	}
	// parse effect data
	auto effect_node = ptree.get_child_optional("effect");
	if (effect_node) {
		effect.loadFromTree(*effect_node);
	}
	// parse damage data
	parse(ptree, damage, "damage", 0.f);
	// parse recover data
	parse(ptree, recover, "recover", 0.f);
}

void PerkTemplate::saveToTree(utils::ptree_type& ptree) const {
	// dump general data
	ptree.put("general.<xmlattr>.type", to_string(type));
	ptree.put("general.<xmlattr>.display_name", display_name);
	ptree.put("general.<xmlattr>.icon_name", icon_name);
	if (!use_sound.empty()) {
		ptree.put("general.<xmlattr>.use_sound", use_sound);
	}
	ptree.put("general.<xmlattr>.revive", revive);
	// dump bullet data
	utils::ptree_type child;
	bullet.saveToTree(child);
	ptree.put_child("bullet", child);
	// dump effect data
	utils::ptree_type child2;
	effect.saveToTree(child2);
	ptree.put_child("effect", child2);
	// dump damage data
	dump(ptree, damage, "damage", 0.f);
	// dump recover data
	dump(ptree, recover, "recover", 0.f);
}

// ---------------------------------------------------------------------------

TrapTemplate::TrapTemplate()
	: BaseResource{}
	, trigger_sound{}
	, bullet{}
	, effect{}
	, sound{nullptr}
	, damage{0u} {}

void TrapTemplate::loadFromTree(utils::ptree_type const& ptree) {
	trigger_sound = ptree.get<std::string>("general.<xmlattr>.sound", "");
	// parse bullet data
	bullet.loadFromTree(ptree.get_child("bullet"));
	// parse effect data
	auto effect_node = ptree.get_child_optional("effect");
	if (effect_node) {
		effect.loadFromTree(*effect_node);
	}
	// parse damage data
	parse(ptree, damage, "damage", 0u);
}

void TrapTemplate::saveToTree(utils::ptree_type& ptree) const {
	ptree.put("general.<xmlattr>.sound", trigger_sound);
	// dump bullet data
	utils::ptree_type child;
	bullet.saveToTree(child);
	ptree.put_child("bullet", child);
	// dump bullet data
	utils::ptree_type child2;
	effect.saveToTree(child2);
	ptree.put_child("effect", child2);
	// dump damage data
	dump(ptree, damage, "damage", 0u);
}

}  // ::game
