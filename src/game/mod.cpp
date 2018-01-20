#include <boost/algorithm/string.hpp>

#include <utils/ortho_tile.hpp>
#include <game/mod.hpp>

namespace game {

namespace mod_impl {

std::string concat(std::string const & lhs, std::string const & rhs) {
	if (lhs.empty()) {
		return rhs;
	} else {
		return lhs + "/" + rhs;
	}
}

// --------------------------------------------------------------------

template <typename T>
bool verify(utils::Logger& log, std::string const& key, T const& resource) {
	// fallback
	return true;
}

template <> 
bool verify<rpg::TilesetTemplate>(utils::Logger& log,
	std::string const& key, rpg::TilesetTemplate const& resource) {
	utils::Verifier verify{log};
	verify(!resource.tileset_name.empty(),	"            tileset required");
	verify(resource.tilesize.x > 0,			"            tile width > 0 required");
	verify(resource.tilesize.y > 0,			"            tile height > 0 required");
	verify(!resource.floors.empty(),		"            floor tile(s) required");
	verify(!resource.walls.empty(),			"            wall tile(s) required");
	verify(resource.tileset != nullptr,		"            tileset cannot be loaded");
	return verify.result;
}

template <>
bool verify<rpg::SpriteTemplate>(utils::Logger& log,
	std::string const& key, rpg::SpriteTemplate const& resource) {
	utils::Verifier verify{log};
	verify(!resource.torso[core::AnimationAction::Idle].frames.empty(),
		"            torso idle frame(s) required");
	verify(
		resource.torso[core::AnimationAction::Idle].duration > sf::Time::Zero,
		"            torso idle duration > 0 required");
	verify(!resource.frameset_name.empty(), "            frameset required");
	verify(resource.frameset != nullptr, "            frameset cannot be loaded");
	if (!resource.edges.empty()) {
		verify(resource.edges.size() >= 3u,
		"            at least 3 edges required (or none)");
	}
	return verify.result;
}

template <>
bool verify<rpg::EntityTemplate>(utils::Logger& log,
	std::string const& key, rpg::EntityTemplate const& resource) {
	utils::Verifier verify{log};
	if (resource.is_projectile) {
		verify(resource.collide, "            collide required if projectile");
	}
	verify(resource.max_sight >= 0.f, "            max sight >= 0.f required");
	verify(resource.max_sight <= core::MAX_SIGHT,
		"            max sight <= " + std::to_string(core::MAX_SIGHT) +
			" required");
	if (resource.max_sight > 0.f) {
		verify(!resource.display_name.empty(), "            display name required to be focusable");
	}
	verify(resource.max_speed >= 0, "            max speed >= 0.f required");
	verify(resource.max_speed <= core::movement_impl::MAX_SPEED,
		"            max speed <= " +
			std::to_string(core::movement_impl::MAX_SPEED) + " required");
	verify(resource.fov >= 0.f, "            fov >= 0.f required");
	verify(resource.fov <= 180.f, "            fov <= 180.f required");
	verify(!resource.sprite_name.empty(), "            sprite name required");
	verify(resource.sprite != nullptr, "            sprite cannot be loaded");
	for (auto const& pair : resource.sounds) {
		for (auto const & node: pair.second) {
			verify(!node.first.empty(),  "            sound needs filename");
			verify(node.second != nullptr,  "            sound needs buffer");
		}
	}
	if (resource.interact != nullptr) {
		if (*resource.interact == rpg::InteractType::Barrier) {
			verify(resource.max_speed > 0.f, "            max speed > 0.f required by barrier");
		}
		verify(resource.max_sight == 0.f, "            max sight == 0.f required by interactable");
		verify(!resource.display_name.empty(), "            display name required by interactable");
	}
	return verify.result;
}

template <>
bool verify<rpg::EffectTemplate>(utils::Logger& log,
	std::string const& key, rpg::EffectTemplate const& resource) {
	utils::Verifier verify{log};
	verify(!resource.display_name.empty(), "            display name required");
	if (!resource.inflict_sound.empty()) {
		verify(resource.sound != nullptr,
			"            inflict sound cannot be loaded");
	}
	return verify.result;
}

template <>
bool verify<rpg::BulletTemplate>(utils::Logger& log,
	std::string const& key, rpg::BulletTemplate const& resource) {
	auto max_radius = core::collision_impl::MAX_PROJECTILE_RADIUS;

	utils::Verifier verify{log};
	verify(!resource.entity_name.empty(), "            entity name required");
	verify(resource.radius >= 0.f, "            radius >= 0.f required");
	verify(resource.radius <= max_radius,
		"            radius <= " + std::to_string(max_radius) + " required");
	verify(resource.entity != nullptr, "            entity cannot be loaded");
	return verify.result;
}

template <>
bool verify<rpg::ItemTemplate>(utils::Logger& log,
	std::string const& key, rpg::ItemTemplate const& resource) {
	utils::Verifier verify{log};
	verify(!resource.display_name.empty(), "            display name required");
	verify(!resource.icon_name.empty(), "            icon name required");
	if (resource.type == rpg::ItemType::Weapon && !resource.melee) {
		verify(!resource.bullet.name.empty(), "            bullet required for range weapon");
	}
	if (!resource.bullet.name.empty()) {
		verify(resource.bullet.bullet != nullptr,
			"            bullet cannot be loaded");
	}
	if (!resource.effect.name.empty()) {
		verify(resource.effect.effect != nullptr,
			"            effect cannot be loaded");
		verify(resource.effect.ratio > 0.f,
			"            effect ratio > 0.0 required");
		verify(resource.effect.ratio <= 1.f,
			"            effect ratio <= 1.0 required");
	}
	verify(resource.icon != nullptr, "            icon cannot be loaded");
	if (!resource.use_sound.empty()) {
		verify(
			resource.sound != nullptr, "            use sound cannot be loaded");
	}
	if (resource.type == rpg::ItemType::Weapon ||
		resource.type == rpg::ItemType::Armor) {
		// item is equipment
		verify(resource.slot != rpg::EquipmentSlot::None,
			"            invalid equipment slot be loaded");
		verify(!resource.sprite_name.empty(),
			"            equipment sprite name is required");
	}
	if (resource.revive) {
		verify(resource.type == rpg::ItemType::Potion,
			"            revive item requires to be potion");
		verify(resource.recover[rpg::Stat::Life] > 0,
			"            revive item requires life recovery");
	}
	if (!resource.sprite_name.empty()) {
		verify(resource.sprite != nullptr, "            sprite cannot be loaded");
	}
	return verify.result;
}

template <>
bool verify<rpg::PerkTemplate>(utils::Logger& log,
	std::string const& key, rpg::PerkTemplate const& resource) {
	utils::Verifier verify{log};
	verify(!resource.display_name.empty(), "            display name required");
	verify(!resource.icon_name.empty(), "            icon name required");
	verify(resource.icon != nullptr, "            icon cannot be loaded");
	if (resource.type == rpg::PerkType::Self) {
		verify(resource.bullet.name.empty(),
			"            bullet not allowed for defensive perk");
	}
	if (!resource.bullet.name.empty()) {
		verify(resource.bullet.bullet != nullptr,
			"            bullet cannot be loaded");
	}
	if (resource.revive) {
		verify(resource.type == rpg::PerkType::Allied,
			"            revive perk requires to be supporting");
		verify(resource.recover[rpg::Stat::Life] > 0,
			"            revive perk requires life recovery");
	}
	if (!resource.effect.name.empty()) {
		verify(resource.effect.effect != nullptr,
			"            effect cannot be loaded");
		verify(resource.effect.ratio > 0.f,
			"            effect ratio > 0.0 required");
		verify(resource.effect.ratio <= 1.f,
			"            effect ratio <= 1.0 required");
	}
	if (!resource.use_sound.empty()) {
		verify(
			resource.sound != nullptr, "            use sound cannot be loaded");
	}
	return verify.result;
}

template <>
bool verify<rpg::TrapTemplate>(utils::Logger& log,
	std::string const& key, rpg::TrapTemplate const& resource) {
	utils::Verifier verify{log};
	verify(!resource.bullet.name.empty(), "            bullet name required");
	verify(
		resource.bullet.bullet != nullptr, "            bullet cannot be loaded");
	if (!resource.effect.name.empty()) {
		verify(resource.effect.effect != nullptr,
			"            effect cannot be loaded");
		verify(resource.effect.ratio > 0.f,
			"            effect ratio > 0.0 required");
		verify(resource.effect.ratio <= 1.f,
			"            effect ratio <= 1.0 required");
	}
	if (!resource.trigger_sound.empty()) {
		verify(resource.sound != nullptr,
			"          trigger sound cannot be loaded");
	}
	return verify.result;
}

template <>
bool verify<game::BotTemplate>(utils::Logger& log,
	std::string const& key, game::BotTemplate const& resource) {
	utils::Verifier verify{log};
	verify(!resource.display_name.empty(), "            display name required");
	verify(!resource.entity_name.empty(), "            entity name required");
	verify(resource.entity != nullptr, "            entity cannot be loaded");
	for (auto const& node : resource.items) {
		verify(std::get<1>(node) > 0.f,
			"            relative item quantity > 0.0 required");
		verify(std::get<2>(node) != nullptr,
			"            item '" + std::get<0>(node) + "' cannot be loaded");
	}
	for (auto const& node : resource.perks) {
		verify(std::get<1>(node) > 0.f,
			"            relative perk level > 0.0 required");
		verify(std::get<2>(node) != nullptr,
			"            perk '" + std::get<0>(node) + "' cannot be loaded");
	}
	float attrib_sum{0.f};
	for (auto const& pair : resource.attributes) {
		verify(pair.second >= 0.f, "            relative attribute '" +
									   to_string(pair.first) +
									   "' >= 0.0 required");
		verify(pair.second <= 1.f, "            relative attribute '" +
									   to_string(pair.first) +
									   "' <= 1.0 required");
		attrib_sum += pair.second;
	}
	verify(attrib_sum == 1.f, "            sum of attributes required == 1.0");
	for (auto const & pair: resource.properties) {
		verify(pair.second >= 0.f, "            property '"
			+ to_string(pair.first) + "' bonus >= 0.0 required");
	}
	for (auto const & pair: resource.defense) {
		verify(pair.second >= 0.f, "            defense '"
			+ to_string(pair.first) + "' bonus >= 0.0 required");
	}
	return verify.result;
}

template <>
bool verify<game::EncounterTemplate>(utils::Logger& log,
	std::string const& key, game::EncounterTemplate const& resource) {
	utils::Verifier verify{log};
	verify(!resource.bots.empty(), "            bots required");
	float total{0.f};
	for (auto const & bot: resource.bots) {
		verify(!bot.filename.empty(), "            bot name required");
		verify(bot.ratio > 0.f, "            bot ratio > 0.0 required");
		verify(bot.ratio <= 1.f, "            bot ratio <= 1.0 required");
		verify(bot.ptr != nullptr, "            bot cannot be loaded");
		total += bot.ratio;
	}
	verify(total == 1.f, "            total ratio must match 1.0");
	return verify.result;
}

template <>
bool verify<game::RoomTemplate>(utils::Logger& log,
	std::string const& key, game::RoomTemplate const& resource) {
	utils::Verifier verify{log};
	for (auto const & pair: resource.cells) {
		auto const & entity = pair.second.entity;
		if (!entity.name.empty()) {
			verify(entity.ptr != nullptr, "            entity cannot be loaded");
			verify(entity.direction.x >= -1, "            invalid entity direction");
			verify(entity.direction.x <= 1, "            invalid entity direction");
			verify(entity.direction.y >= -1, "            invalid entity direction");
			verify(entity.direction.y <= 1, "            invalid entity direction");
		}
	}
	return verify.result;
}

} // ::mod_impl

// --------------------------------------------------------------------

Mod::Mod(core::LogContext& log, ResourceCache& cache, std::string const & name)
	: log{log}
	, cache{cache}
	, processed_tilesets{}
	, name{name} {
}

// --------------------------------------------------------------------

template <>
void Mod::prepare(rpg::TilesetTemplate& resource) {
	auto& tex = query<sf::Texture>(resource.tileset_name);
	
	// fix tileset if necessary
	if (!utils::contains(processed_tilesets, resource.tileset_name)) {
		auto tmp = utils::fixTileset(tex.copyToImage(), resource.tilesize);
		bool success = tex.loadFromImage(tmp);
		ASSERT(success);
		processed_tilesets.push_back(resource.tileset_name);
	}
	
	resource.tileset = &tex;
}

template <>
void Mod::prepare(rpg::SpriteTemplate& resource) {
	resource.frameset = &get<sf::Texture>(resource.frameset_name);
}

template <>
void Mod::prepare(rpg::EntityTemplate& resource) {
	auto& sprite = query<rpg::SpriteTemplate>(resource.sprite_name);
	prepare(sprite);
	resource.sprite = &sprite;
	for (auto& pair : resource.sounds) {
		for (auto& node: pair.second) {
			node.second = &get<sf::SoundBuffer>(node.first);
		}
	}
}

template <>
void Mod::prepare(rpg::EffectTemplate& resource) {
	if (!resource.inflict_sound.empty()) {
		resource.sound = &get<sf::SoundBuffer>(resource.inflict_sound);
	}
}

template <>
void Mod::prepare(rpg::BulletTemplate& resource) {
	auto& entity = query<rpg::EntityTemplate>(resource.entity_name);;
	prepare(entity);
	resource.entity = &entity;
}

template <>
void Mod::prepare(rpg::ItemTemplate& resource) {
	if (!resource.bullet.name.empty()) {
		auto& bullet = query<rpg::BulletTemplate>(resource.bullet.name);
		prepare(bullet);
		resource.bullet.bullet = &bullet;
	}
	if (!resource.effect.name.empty()) {
		auto& effect = query<rpg::EffectTemplate>(resource.effect.name);
		prepare(effect);
		resource.effect.effect = &effect;
	}
	resource.icon = &get<sf::Texture>(resource.icon_name);
	if (!resource.use_sound.empty()) {
		resource.sound = &get<sf::SoundBuffer>(resource.use_sound);
	}
	auto& sprite = query<rpg::SpriteTemplate>(resource.sprite_name);
	prepare(sprite);
	resource.sprite = &sprite;
}

template <>
void Mod::prepare(rpg::PerkTemplate& resource) {
	auto& bullet = query<rpg::BulletTemplate>(resource.bullet.name);
	prepare(bullet);
	resource.bullet.bullet = &bullet;
	if (!resource.effect.name.empty()) {
		auto& effect = query<rpg::EffectTemplate>(resource.effect.name);
		prepare(effect);
		resource.effect.effect = &effect;
	}
	resource.icon = &get<sf::Texture>(resource.icon_name);
	if (!resource.use_sound.empty()) {
		resource.sound = &get<sf::SoundBuffer>(resource.use_sound);
	}
}

template <>
void Mod::prepare(rpg::TrapTemplate& resource) {
	auto& bullet = query<rpg::BulletTemplate>(resource.bullet.name);
	prepare(bullet);
	resource.bullet.bullet = &bullet;
	if (!resource.effect.name.empty()) {
		auto& effect = query<rpg::EffectTemplate>(resource.effect.name);
		prepare(effect);
		resource.effect.effect = &effect;
	}
	if (!resource.trigger_sound.empty()) {
		resource.sound = &get<sf::SoundBuffer>(resource.trigger_sound);
	}
}

template <>
void Mod::prepare(game::BotTemplate& resource) {
	auto& entity = query<rpg::EntityTemplate>(resource.entity_name);
	prepare(entity);
	resource.entity = &entity;
	for (auto& node : resource.items) {
		auto& item = query<rpg::ItemTemplate>(std::get<0>(node));
		prepare(item);
		std::get<2>(node) = &item;
	}
	for (auto& node : resource.perks) {
		auto& perk = query<rpg::PerkTemplate>(std::get<0>(node));
		prepare(perk);
		std::get<2>(node) = &perk;
	}
}

template <>
void Mod::prepare(game::EncounterTemplate& resource) {
	for (auto& node: resource.bots) {
		auto& bot = query<game::BotTemplate>(node.filename);
		prepare(bot);
		node.ptr = &bot;
	}
}

template <>
void Mod::prepare(game::RoomTemplate& resource) {
	for (auto& pair: resource.cells) {
		auto& entity = pair.second.entity;
		if (!entity.name.empty()) {
			auto& res = query<rpg::EntityTemplate>(entity.name);
			prepare(res);
			entity.ptr = &res;
		}
	}
}

// ---------------------------------------------------------------------------

void Mod::preload(bool force) {
	log.debug << "Loading resources\n";
	preload<sf::Texture>(force);
	preload<sf::SoundBuffer>(force);
	preload<sf::Font>(force);
	preload_and_prepare<rpg::TilesetTemplate>(force);
	preload_and_prepare<rpg::SpriteTemplate>(force);
	preload_and_prepare<rpg::EntityTemplate>(force);
	preload_and_prepare<rpg::EffectTemplate>(force);
	preload_and_prepare<rpg::BulletTemplate>(force);
	preload_and_prepare<rpg::ItemTemplate>(force);
	preload_and_prepare<rpg::PerkTemplate>(force);
	preload_and_prepare<rpg::TrapTemplate>(force);
	preload_and_prepare<game::BotTemplate>(force);
	preload_and_prepare<game::EncounterTemplate>(force);
	preload_and_prepare<game::RoomTemplate>(force);
	log.debug << "Done\n";
}

bool Mod::verify(utils::Logger& log) {
	bool result = true;
	log << "Verifying resources\n";
	result &= verify<rpg::TilesetTemplate>(log);
	result &= verify<rpg::SpriteTemplate>(log);
	result &= verify<rpg::EntityTemplate>(log);
	result &= verify<rpg::EffectTemplate>(log);
	result &= verify<rpg::BulletTemplate>(log);
	result &= verify<rpg::ItemTemplate>(log);
	result &= verify<rpg::PerkTemplate>(log);
	result &= verify<rpg::TrapTemplate>(log);
	result &= verify<game::BotTemplate>(log);
	result &= verify<game::EncounterTemplate>(log);
	result &= verify<game::RoomTemplate>(log);
	log << "Done\n";
	return result;
}

// --------------------------------------------------------------------

AiScript& Mod::createScript(std::string const & fname) {
	scripts.emplace_back();
	auto& script = scripts.back();
	auto filename = get_filename<AiScript>(fname);
	ASSERT(script.loadFromFile(filename));
	return script;
}

std::list<AiScript>& Mod::getAllScripts() {
	return scripts;
}

std::vector<sf::Texture const *> Mod::getAllAmbiences() {
	std::vector<sf::Texture const *> out;
	auto path = get_path<sf::Texture>() + "/ambience";
	auto ext = Mod::get_ext<sf::Texture>();
	utils::for_each_file(path, ext, [&](std::string const & p, std::string const& key) {
		auto full = "ambience/" + mod_impl::concat(p, key);
		out.push_back(&get<sf::Texture>(full));
	});
	return out;
}

}  // ::rage
