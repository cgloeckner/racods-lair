#include <boost/algorithm/string.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <utils/atlas.hpp>
#include <utils/filesystem.hpp>
#include <utils/xml_utils.hpp>

#include <rpg/resources.hpp>
#include <rpg/spritepacker.hpp>

namespace rpg {

SpritePacker::SpritePacker(std::string const & source, std::string const & target, utils::Logger& log)
	: source{source}
	, target{target}
	, log{log} {
}

bool SpritePacker::operator()(std::string const & fname) {
	utils::AtlasGenerator<std::string> builder;
	utils::Atlas<std::string> atlas;
	utils::ptree_type config;
	rpg::SpriteTemplate sprite;
	boost::property_tree::read_xml(source + "/" + fname + ".xml", config);
	
	// load legs
	auto leg_node = config.get_child_optional("legs");
	if (leg_node) {
		auto n = leg_node->get<unsigned int>("<xmlattr>.size");
		log << "    Adding 'Move' frames to texture atlas\n";
		for (auto i = 0u; i < n; ++i) {
			auto key = "move_" + std::to_string(i) + ".png";
			sf::Image tmp;
			log << "        " << key << "\n";
			if (!tmp.loadFromFile(source + "/" + fname + "_" + key)) {
				log << "Cannot load '" << source << "/" << fname
					<< "_" << key << "\n";
				return false;
			}
			auto origin = sf::Vector2f{tmp.getSize()} / 2.f;
			builder.add(key, std::move(tmp), origin);
		}
	}
	
	// load torso
	auto const & torso_node = config.get_child("torso");
	for (auto action: utils::EnumRange<core::AnimationAction>{}) {
		auto dump = to_string(action);
		auto child = torso_node.get_child_optional(dump);
		if (!child) {
			continue;
		}
		auto n = child->get<unsigned int>("<xmlattr>.size");
		log << "    Adding '" << dump << "' frames to texture atlas\n";
		for (auto i = 0u; i < n; ++i) {
			auto key = dump + "_" + std::to_string(i) + ".png";
			boost::algorithm::to_lower(key);
			sf::Image tmp;
			log << "        " << key << "\n";
			if (!tmp.loadFromFile(source + "/" + fname + "_" + key)) {
				log << "Cannot load '" << source << "/" << fname
					<< "_" << key << "\n";
				return false;
			}
			auto origin = sf::Vector2f{tmp.getSize()} / 2.f;
			builder.add(key, std::move(tmp), origin);
		}
	}
	// create atlas
	log << "    Generate texture atlas\n";
	builder.generate({16u, 16u}, 256u, atlas);
	
	// create legs
	if (leg_node) {
		auto n = leg_node->get<unsigned int>("<xmlattr>.size");
		for (auto i = 0u; i < n; ++i) {
			auto key = "move_" + std::to_string(i) + ".png";
			auto const & frame = atlas.frames[key];
			auto duration = leg_node->get<unsigned int>(std::to_string(i) + ".<xmlattr>.duration");
			sprite.legs.append(frame.clipping, frame.origin, sf::milliseconds(duration));
		}
		log << "    Processed " << n << " frames for 'Move' animation\n";
	} else {
		log << "    Processed 0 frames for 'Move' animation\n";
	}
	// create torso
	for (auto action: utils::EnumRange<core::AnimationAction>{}) {
		auto dump = to_string(action);
		auto child = torso_node.get_child_optional(dump);
		if (!child) {
			log << "    Processed 0 frames for '" << dump << "' animation\n";
			continue;
		}
		auto& dst = sprite.torso[action];
		auto n = child->get<unsigned int>("<xmlattr>.size");
		for (auto i = 0u; i < n; ++i) {
			auto key = dump + "_" + std::to_string(i) + ".png";
			boost::algorithm::to_lower(key);
			auto const & frame = atlas.frames[key];
			auto duration = child->get<unsigned int>(std::to_string(i) + ".<xmlattr>.duration");
			dst.append(frame.clipping, frame.origin, sf::milliseconds(duration));
		}
		log << "    Processed " << n << " frames for '" << dump << "' animation\n";
	}
	// save sprite template
	sprite.frameset_name = fname;
	sprite.saveToFile(target + "/xml/sprite/" + fname + ".xml");
	log << "    Saving sprite config to " << target << "/xml/sprite/" << fname << ".xml\n";
	atlas.image.saveToFile(target + "/gfx/" + fname + ".png");
	log << "    Saving sprite atlas to " << target << "/gfx/" << fname << ".png\n";
	
	return true;
}

} // ::rpg
