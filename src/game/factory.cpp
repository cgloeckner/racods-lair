#include <utils/algorithm.hpp>
#include <utils/color.hpp>
#include <core/teleport.hpp>
#include <core/render.hpp>
#include <rpg/balance.hpp>
#include <rpg/item.hpp>
#include <rpg/stats.hpp>
#include <game/factory.hpp>
#include <game/item.hpp>

namespace game {

std::uint32_t const MIN_BOT_ATTRIB = 5u;

float const PLAYER_LIGHT_RADIUS = 10.f;
sf::Uint8 const PLAYER_LIGHT_INTENSITY = 255u;
sf::Color const PLAYER_LIGHT_COLOR = sf::Color::White;

float const PLAYER_ADVANTAGE_FACTOR = 1.5f;

std::size_t const MAX_POWERUP_SPAWN_DRIFT = 20u;

// --------------------------------------------------------------------

namespace factory_impl {

bool canHoldPowerup(Session const & session, utils::SceneID scene, sf::Vector2f const & pos, core::ObjectID ignore) {
	auto const & dungeon = session.dungeon[scene];
	if (!dungeon.has(sf::Vector2u{pos})) {
		return false;
	} 
	auto const & cell = dungeon.getCell(sf::Vector2u{pos});
	if (cell.terrain != core::Terrain::Floor || cell.trigger != nullptr) {
		return false;
	}
	for (auto id: cell.entities) {
		if (id == ignore) {
			continue;
		}
		if (session.collision.has(id) && !session.projectile.has(id)) {
			// can collide and is no bullet ... not a good spot to spawn!
			return false;
		}
	}
	return true;
}

} // ::factory_impl

// --------------------------------------------------------------------

Factory::Factory(core::LogContext& log, Session& session, Mod& mod)
	: utils::EventListener<rpg::ProjectileEvent, rpg::DeathEvent,
		rpg::SpawnEvent, ReleaseEvent>{}
	, utils::EventSender<core::InputEvent, rpg::ActionEvent, rpg::ItemEvent,
		rpg::StatsEvent, rpg::SpawnEvent, PowerupEvent>{}
	, log{log}
	, max_num_players{session.movement.capacity()}
	, session{session}
	, mod{mod}
	, entity_cache{}
	, release{}
	, latest_player{0u}
	, blood_texture{nullptr}
	, gem_tpl{nullptr} {
	entity_cache.resize(max_num_players);
}

Factory::EntityCache::EntityCache()
	: entity{nullptr}
	, hostile{true} {
}

void Factory::setupObject(core::ObjectID id, rpg::EntityTemplate const & entity) {
	ASSERT(entity.sprite != nullptr);
	ASSERT(entity.sprite->frameset != nullptr);

	// create object physics
	auto& m = session.movement.query(id);
	m.max_speed = entity.max_speed;
	if (session.focus.has(id)) {
		auto& f = session.focus.query(id);
		f.display_name = entity.display_name;
		f.sight = entity.max_sight;
		f.fov = entity.fov;
		f.is_active = true;
		f.has_changed = true;
	}
	if (entity.collide) {
		// add collision component
		auto& c = session.collision.query(id);
		c.is_projectile = entity.is_projectile;
	}

	// setup render component (including optional light)
	auto& r = session.render.query(id);
	r.blood_color = entity.blood_color;
	if (entity.light != nullptr) {
		auto& a = session.animation.query(id);
		r.light = std::make_unique<utils::Light>(*entity.light);
		a.light_radius.current = r.light->radius;
	}

	// identify id with the used template
	entity_cache[id].entity = &entity;
}

void Factory::onBulletExploded(core::ObjectID id) {
	ASSERT(session.movement.has(id));
	ASSERT(session.collision.has(id));

	// stop movement
	auto& mv = session.movement.query(id);
	mv.move = sf::Vector2f{};

	// release collision component
	session.collision.release(id);

	release.push(id, sf::milliseconds(FADE_DELAY));
}

utils::SceneID Factory::createDungeon(rpg::TilesetTemplate const& tileset,
	sf::Vector2u grid_size, BuildSettings const & settings,
	BuilderModifier modifier) {
	ASSERT(tileset.tileset != nullptr);
	
	session.generator.layoutifySize(grid_size);
	
	// create empty dungeon
	auto tilesize = sf::Vector2f{tileset.tilesize};
	auto id = session.dungeon.create(*tileset.tileset, grid_size, tilesize);
	
	// generate dungeon content
	session.generator.generate(id, grid_size);
	
	// populate dungeon
	auto& dungeon = session.dungeon[id];
	auto& builder = session.generator[id].builder;
	modifier(builder);
	if (settings.random_transform) {
		for (auto& room: builder.rooms) {
			room.angle = thor::random(0, 3) * 90.f;
			room.flip_x = (bool)(thor::random(0, 1));
			room.flip_y = (bool)(thor::random(0, 1));
		}
	}
	builder(tileset, dungeon, settings);
	
	// create pathfinding navigator
	auto& navigator = session.navigation.create(id, session.movement,
		session.collision, dungeon, builder);
	session.path.addScene(id, navigator);
	
	// create provided entities
	rpg::SpawnMetaData spawn;
	spawn.scene = id;
	for (auto const & room: builder.rooms) {
		for (auto const & pair: room.tpl.cells) {
			auto const & entity = pair.second.entity;
			if (entity.ptr == nullptr) {
				continue;
			}
			// determine position and direction
			sf::Vector2u pos{pair.first};
			sf::Vector2i dir{entity.direction};
			dungeon_impl::transform(pos, settings.cell_size, room.angle, room.flip_x, room.flip_y);
			dungeon_impl::transform(dir, room.angle, room.flip_x, room.flip_y);
			// spawn (centered to the tile)
			spawn.pos       = sf::Vector2f{pos + room.offset} + utils::HalfTilePos;
			spawn.direction = sf::Vector2f{dir};
			createObject(*entity.ptr, spawn);
		}
	}
	
	return id;
}

void Factory::createAmbience(sf::Texture const & texture, rpg::SpawnMetaData const & data,
	sf::Color const & color) {
	ASSERT(data.scene > 0u);
	
	// query target cell
	auto& dungeon = session.dungeon[data.scene];
	auto& target = dungeon.getCell(sf::Vector2u{data.pos}).ambiences;
	
	// create sprite
	target.emplace_back(texture);
	auto& sprite = target.back();
	sprite.setOrigin(sf::Vector2f{texture.getSize()} / 2.f);
	sprite.setColor(color);
	
	// set random offset and rotation
	auto screen_pos = dungeon.toScreen(sf::Vector2f{data.pos});
	auto tile_size = dungeon.getTileSize();
	screen_pos.x += thor::random(-tile_size.x, tile_size.x) / 2.f;
	screen_pos.y += thor::random(-tile_size.y, tile_size.y) / 2.f;
	sprite.setPosition(screen_pos);
	sprite.setRotation(thor::random(0.f, 360.f));
}

core::ObjectID Factory::createObject(
	rpg::EntityTemplate const& entity, rpg::SpawnMetaData const& data) {
	ASSERT(entity.sprite != nullptr);
	ASSERT(entity.sprite->frameset != nullptr);

	if (entity.interact != nullptr) {
		ASSERT(entity.collide);
		ASSERT(entity.max_sight == 0.f);
	}
	
	if (entity.max_sight > 0.f) {
		ASSERT(!entity.display_name.empty());
	}
	
	auto id = session.id_manager.acquire();
	//log.debug << "[Game/Factory] " << "Next ID: " << id << "\n";
	
	// create components
	auto& m = session.movement.acquire(id);
	m.look = data.direction;
	core::spawn(session.dungeon[data.scene], m, data.pos);
	if (entity.max_sight > 0.f || !entity.display_name.empty()) {
		auto& f = session.focus.acquire(id);
	}
	if (entity.collide) {
		auto& c = session.collision.acquire(id);
		c.shape = entity.shape;
		if (c.shape.is_aabb) {
			c.shape.updateRadiusAABB();
		}
		c.has_changed = true;
	}
	session.render.acquire(id);
	session.animation.acquire(id);
	if (entity.hasSounds()) {
		session.audio.acquire(id);
	}
	
	// setup components
	setupObject(id, entity);
	
	// create object graphics
	auto& r = session.render.query(id);
	r.default_rotation = entity.sprite->rotation;
	auto color = utils::ptrToColor(&r);
	color.a = 32u;
	r.fov.setFillColor(color);
	color.a = 127u;
	r.fov.setOutlineColor(color);
	r.fov.setOutlineThickness(1.f);
	if (entity.collide) {
		if (entity.shape.is_aabb) {
			r.shape = std::make_unique<sf::RectangleShape>(entity.shape.size);
		} else {
			r.shape = std::make_unique<sf::CircleShape>(entity.shape.radius);
		}
		color.a = 64u;
		r.shape->setFillColor(color);
		color.a = 127u;
		r.shape->setOutlineColor(color);
		r.shape->setOutlineThickness(1.f);
	}
	r.edges = entity.sprite->edges;
	auto const& frameset = *entity.sprite->frameset;
	r.torso[core::SpriteTorsoLayer::Base].setTexture(frameset);
	if (!entity.sprite->legs.frames.empty()) {
		r.legs[core::SpriteLegLayer::Base].setTexture(frameset);
	}
	
	// create object animations
	auto& a = session.animation.query(id);
	a.flying = entity.flying;
	if (!entity.sprite->legs.frames.empty()) {
		a.tpl.legs[core::SpriteLegLayer::Base] = &entity.sprite->legs;
	}
	a.tpl.torso[core::SpriteTorsoLayer::Base] = &entity.sprite->torso;

	// create object sounds
	if (entity.hasSounds()) {
		auto& s = session.audio.query(id);
		for (auto const & pair: entity.sounds) {
			auto& target = s.sfx[pair.first];
			for (auto const & node: pair.second) {
				target.push_back(node.second);
			}
		}
	}
	
	// make interactable if necessary
	if (entity.interact != nullptr) {
		// override object layer
		auto& r = session.render.query(id);
		r.layer = core::ObjectLayer::Middle;
		
		// create object interactables
		auto& i = session.interact.acquire(id);
		i.type = *entity.interact;
	}

	// trigger (re)spawn
	rpg::SpawnEvent event;
	event.actor = id;
	send(event);

	return id;
}

core::ObjectID Factory::createBullet(rpg::CombatMetaData const& meta,
	rpg::SpawnMetaData const& spawn, core::ObjectID owner) {
	rpg::BulletTemplate const* bullet{nullptr};
	auto color = sf::Color::White;
	switch (meta.emitter) {
		case rpg::EmitterType::Weapon:
			ASSERT(meta.primary != nullptr);
			bullet = meta.primary->bullet.bullet;
			color = meta.primary->bullet.color;
			break;

		case rpg::EmitterType::Perk:
			ASSERT(meta.perk != nullptr);
			bullet = meta.perk->bullet.bullet;
			color = meta.perk->bullet.color;
			break;

		case rpg::EmitterType::Trap:
			ASSERT(meta.trap != nullptr);
			bullet = meta.trap->bullet.bullet;
			color = meta.trap->bullet.color;
			break;

		default:
			// not handled here
			break;
	}
	ASSERT(bullet != nullptr);
	ASSERT(bullet->entity != nullptr);
	ASSERT(bullet->entity->collide);
	ASSERT(bullet->entity->is_projectile);
	ASSERT(bullet->entity->max_sight == 0.f);

	// refresh spawn if owner given
	auto spwn = spawn;
	if (owner > 0u) {
		ASSERT(session.movement.has(owner));
		auto const& m = session.movement.query(owner);

		auto pos = m.pos;
		pos.x = pos.x;
		pos.y = pos.y;
		spwn.pos = pos;
		spwn.direction = m.look;
	}

	auto id = createObject(*bullet->entity, spwn);

	// override object layer
	auto& r = session.render.query(id);
	r.layer = core::ObjectLayer::Top;
	r.torso[core::SpriteTorsoLayer::Base].setColor(color);

	// override collision radius
	auto& c = session.collision.query(id);
	c.shape.radius  = bullet->radius;

	// create object projectile
	auto& p = session.projectile.acquire(id);
	p.owner = owner;
	if (owner > 0u) {
		c.ignore.push_back(owner);
		p.ignore.push_back(owner);
	}
	p.bullet = bullet;
	p.meta_data = meta;

	// move projectile
	core::InputEvent event;
	event.actor = id;
	event.move = spwn.direction;
	event.look = spwn.direction;
	send(event);

	return id;
}

core::ObjectID Factory::createBot(BotTemplate const& bot,
	rpg::SpawnMetaData const& data, std::size_t level,
	/*utils::Script const& script,*/ bool hostile, float difficulty) {
	ASSERT(bot.entity != nullptr);
	ASSERT(bot.entity->collide);
	ASSERT(bot.entity->max_sight > 0.f);

	auto id = createObject(*bot.entity, data);

	// override display name
	auto& f = session.focus.query(id);
	f.display_name = bot.display_name;
	// override object layer
	auto& r = session.render.query(id);
	r.layer = core::ObjectLayer::Top;
	r.legs[core::SpriteLegLayer::Base].setColor(bot.color);
	r.torso[core::SpriteTorsoLayer::Base].setColor(bot.color);

	// create object character
	session.quickslot.acquire(id);
	session.effect.acquire(id);
	session.action.acquire(id);
	auto& s = session.stats.acquire(id);
	s.factor = difficulty;
	auto& i = session.item.acquire(id);
	auto& p = session.perk.acquire(id);
	for (auto& pair : bot.attributes) {
		auto increase = static_cast<std::uint32_t>(std::ceil(pair.second * rpg::ATTRIB_POINTS_PER_LEVEL * level));
		s.attributes[pair.first] = MIN_BOT_ATTRIB + increase;
	}
	for (auto& pair: bot.defense) {
		s.base_def[pair.first] = pair.second;
	}
	for (auto& pair: bot.properties) {
		s.prop_boni[pair.first] = rpg::getPropertyValue(pair.second, level);
	}
	for (auto const& node : bot.items) {
		ASSERT(std::get<2>(node) != nullptr);
		rpg::item_impl::addItem(i, *std::get<2>(node), std::get<1>(node));
	}
	for (auto const& node : bot.perks) {
		ASSERT(std::get<2>(node) != nullptr);
		auto lvl = static_cast<std::size_t>(std::ceil(std::get<1>(node) * level));
		if (lvl == 0u) { ++lvl; }
		p.perks.emplace_back(*std::get<2>(node), lvl);
	}
	s.level = level;
	rpg::stats_impl::refresh(s);
	s.stats[rpg::Stat::Life] = s.properties[rpg::Property::MaxLife];
	s.stats[rpg::Stat::Mana] = s.properties[rpg::Property::MaxMana];
	s.stats[rpg::Stat::Stamina] = s.properties[rpg::Property::MaxStamina];
	// note: equipment is chosen by AI later

	// setup ai
	/*
	auto& a = session.script.acquire(id);
	a.api = std::make_unique<LuaApi>(log, id, hostile, session,
		session.script, *this, *this, *this, session.path);
	a.script = &script;
	script("onInit", a.api.get());
	*/
	
	entity_cache[id].hostile = hostile;

	return id;
}

core::ObjectID Factory::createPlayer(PlayerTemplate const & player,
	rpg::Keybinding const & keys, rpg::SpawnMetaData const & data,
	sf::Color color) {
	ASSERT(player.entity != nullptr);
	ASSERT(player.entity->collide);
	ASSERT(player.entity->max_sight > 0.f);
	auto id = createObject(*player.entity, data);

	// override display name and sight
	auto& f = session.focus.query(id);
	f.display_name = player.display_name;
	f.sight = PLAYER_LIGHT_RADIUS;
	// override object layer
	auto& r = session.render.query(id);
	r.layer = core::ObjectLayer::Top;
	
	// override light
	auto& a = session.animation.query(id);
	if (r.light == nullptr) {
		r.light = std::make_unique<utils::Light>();
	}
	r.light->cast_shadow = true;
	r.light->color = PLAYER_LIGHT_COLOR;
	r.light->intensity = PLAYER_LIGHT_INTENSITY;
	r.light->radius = PLAYER_LIGHT_RADIUS;
	a.light_radius.current = PLAYER_LIGHT_RADIUS;
	
	// setup highlight sprite
	r.highlight = std::make_unique<sf::Sprite>();
	r.highlight->setColor(color);
	auto scale = 3.f * PLAYER_LIGHT_RADIUS / utils::MAX_LIGHT_RADIUS;
	r.highlight->setScale({scale, scale});
	
	// create object character
	session.effect.acquire(id);
	session.action.acquire(id);
	auto& s = session.stats.acquire(id);
	auto& i = session.item.acquire(id);
	auto& p = session.perk.acquire(id);
	s.attributes = player.attributes;
	for (auto const& node : player.inventory) {
		auto ptr = std::get<2>(node);
		ASSERT(ptr != nullptr);
		rpg::item_impl::addItem(i, *ptr, std::get<1>(node));
	}
	for (auto const& node : player.equip_ptr) {
		if (node.second == nullptr) {
			continue;
		}
		rpg::ItemEvent event;
		event.actor = id;
		event.type = rpg::ItemEvent::Use;
		event.item = node.second;
		event.slot = node.first;
		send(event);
	}
	for (auto const& node : player.perks) {
		auto ptr = std::get<2>(node);
		ASSERT(ptr != nullptr);
		p.perks.emplace_back(*ptr, std::get<1>(node));
	}
	s.level = player.level;
	s.factor = PLAYER_ADVANTAGE_FACTOR;
	rpg::stats_impl::refresh(s);
	s.stats[rpg::Stat::Life] = s.properties[rpg::Property::MaxLife];
	s.stats[rpg::Stat::Mana] = s.properties[rpg::Property::MaxMana];
	s.stats[rpg::Stat::Stamina] = s.properties[rpg::Property::MaxStamina];

	// create player components
	auto& in = session.input.acquire(id);
	auto& q = session.quickslot.acquire(id);
	auto& pl = session.player.acquire(id);
	in.keys = keys.map;
	q.slot_id = player.slot_id;
	for (auto i = 0u; i < rpg::MAX_QUICKSLOTS; ++i) {
		auto const& node = player.slots[i];
		if (!std::get<0>(node).empty()) {
			auto ptr = std::get<2>(node);
			ASSERT(ptr != nullptr);
			q.slots[i] = {*ptr};
		} else if (!std::get<1>(node).empty()) {
			auto ptr = std::get<3>(node);
			ASSERT(ptr != nullptr);
			q.slots[i] = {*ptr};
		}
	}
	pl.exp = player.exp;
	pl.base_exp = rpg::getNextExp(player.level);
	pl.next_exp = rpg::getNextExp(player.level+1u);
	pl.attrib_points = player.attrib_points;
	pl.perk_points = player.perk_points;
	pl.player_id = ++latest_player;

	// create dedicated camera
	auto& c = session.camera.acquire();
	c.objects.push_back(id);

	// create hud component
	auto& hud = session.hud.acquire(id);
	hud.hud = std::make_unique<ui::PlayerHud>();
	hud.hud->setLife(s.stats[rpg::Stat::Life], s.properties[rpg::Property::MaxLife]);
	hud.hud->setMana(s.stats[rpg::Stat::Mana], s.properties[rpg::Property::MaxMana]);
	hud.hud->setStamina(s.stats[rpg::Stat::Stamina], s.properties[rpg::Property::MaxStamina]);
	hud.hud->setExp(pl.exp, pl.base_exp, pl.next_exp);
	if (color != sf::Color::Transparent) {
		hud.hud->setColor(color);
	}

	entity_cache[id].hostile = false;

	return id;
}

core::ObjectID Factory::createPowerup(rpg::EntityTemplate const & entity,
	rpg::SpawnMetaData const & spawn, PowerupType type) {
	ASSERT(!entity.collide);
	ASSERT(entity.max_sight == 0.f);
	auto id = createObject(entity, spawn);
	
	// setup appearance
	auto& r = session.render.query(id);
	sf::Color color;
	switch (type) {
		case PowerupType::Life:
			color = sf::Color::Red;
			break;
		
		case PowerupType::Mana:
			color = sf::Color::Blue;
			break;
		
		case PowerupType::Rejuvenation:
			color = sf::Color::Green;
			break;
	}
	r.layer = core::ObjectLayer::Middle;
	r.torso[core::SpriteTorsoLayer::Base].setColor(color);
	if (r.light != nullptr) {
		r.light->color = color;
	}
	
	// create trigger
	auto& trigger = session.dungeon[spawn.scene].getCell(sf::Vector2u{spawn.pos}).trigger;
	auto& stats_sender = dynamic_cast<rpg::StatsSender&>(*this);
	auto& powerup_sender = dynamic_cast<PowerupSender&>(*this);
	auto& release_listener = dynamic_cast<ReleaseListener&>(*this);
	trigger = std::make_unique<PowerupTrigger>(
		id, session.stats, session.player, stats_sender, powerup_sender,
		release_listener, type);
	
	return id;
}

void Factory::destroyObject(core::ObjectID id) {
	if (!session.movement.has(id)) {
		// object has already been released last frame, ignore
		return;
	}
	
	// vanish object
	auto& m = session.movement.query(id);
	if (m.scene == 0u) {
		// object has vanished but not released yet - ignore, too
		return;
	}
	
	auto& d = session.dungeon[m.scene];
	core::vanish(d, m);

	// release all components
	for (auto ptr : session.systems) {
		ptr->tryRelease(id);
	}

	// release id
	session.id_manager.release(id);
}

void Factory::addTeleport(utils::SceneID source, sf::Vector2f const & src,
	utils::SceneID target, sf::Vector2f const & dst) {
	auto& trigger = session.dungeon[source].getCell(sf::Vector2u{src}).trigger;
	auto& teleport_sender = dynamic_cast<core::TeleportSender&>(session.collision);
	trigger = std::make_unique<core::TeleportTrigger>(teleport_sender, session.movement, session.collision, session.dungeon, target, dst);
}

void Factory::handle(rpg::ProjectileEvent const& event) {
	switch (event.type) {
		case rpg::ProjectileEvent::Create:
			createBullet(event.meta_data, event.spawn, event.id);
			break;

		case rpg::ProjectileEvent::Destroy:
			onBulletExploded(event.id);
			break;
	}
}

void Factory::handle(rpg::DeathEvent const& event) {
	auto id = event.actor;
	
	ASSERT(id > 0u);
	ASSERT(session.movement.has(id));
	ASSERT(session.render.has(id));
	ASSERT(session.collision.has(id));
	ASSERT(session.focus.has(id));

	// movement stop
	core::InputEvent stop;
	stop.actor = id;
	send(stop);

	// adjust object layer
	auto& r = session.render.query(id);
	r.layer = core::ObjectLayer::Bottom;

	// drop collision
	session.collision.release(id);  // can be easily recreated at respawn

	// disable focus component
	auto& f = session.focus.query(id);
	f.is_active = false;
	f.has_changed = true;

	// disable script component
	/*
	if (session.script.has(id)) {
		auto& s = session.script.query(id);
		s.is_active = false;
	}
	*/

	auto const & m = session.movement.query(id);
	rpg::SpawnMetaData spawn;
	spawn.scene = m.scene;
	spawn.pos = m.pos;
	spawn.direction = {0, 1};

	// add blood layer
	if (blood_texture != nullptr && r.blood_color != sf::Color::Transparent) {
		createAmbience(*blood_texture, spawn, r.blood_color);
	}

	// add powerup
	if (thor::random(0.f, 1.f) > 0.6f) {
		log.debug << "[Game/Factory] RNG rejected spawning a powerup :3\n";
		return;
	}
	
	if (gem_tpl == nullptr) {
		log.warning << "[Engine/Factory] " << "Cannot create powerup: no config found\n";
		return;
	}
	
	float v = thor::random(0.f, 1.f);
	PowerupType type;
	if (v < 0.5f) {
		type = PowerupType::Life;
	} else if (v > 0.6f) {
		type = PowerupType::Mana;
	} else {
		type = PowerupType::Rejuvenation;
	}
	if (!core::getFreePosition([&](sf::Vector2f const & tmp) {
		return factory_impl::canHoldPowerup(session, spawn.scene, tmp, id);
	}, spawn.pos, MAX_POWERUP_SPAWN_DRIFT)) {
		log.warning << "[Game/Factory] " << "Cannot add more powerups at this area\n";
	} else {
		// spawn powerup
		createPowerup(*gem_tpl, spawn, type);
	}
}

void Factory::handle(rpg::SpawnEvent const& event) {
	if (!event.respawn) {
		// not a respawn
		return;
	}
	
	auto id = event.actor;
	
	ASSERT(id > 0u);
	ASSERT(session.movement.has(id));
	ASSERT(session.render.has(id));
	ASSERT(session.focus.has(id));
	
	ASSERT(entity_cache[id].entity != nullptr);
	auto& entity = *entity_cache[id].entity;
	
	// create missing components
	if (entity.collide) {
		ASSERT(!session.collision.has(id));
		auto& c = session.collision.acquire(id);
		/// @TODO use template
		c.shape.is_aabb = false;
		c.shape.radius = 0.5f;
	}
	
	// re-setup components
	setupObject(id, entity);
	
	// setup object layer
	auto& r = session.render.query(id);
	r.layer = core::ObjectLayer::Top;
	
	// reinitialize script comonent
	/*
	if (session.script.has(id)) {
		// enable component
		auto& s = session.script.query(id);
		s.is_active = true;
		// determine hostility
		auto& hostile = entity_cache[id].hostile;
		if (event.causer != 0u) {
			if (session.script.has(event.causer)) {
				// an AI object revived another object
				auto& a = session.script.query(event.causer);
				//hostile = a.api->hostile;
				hostile = false;
				
			} else if (session.player.has(event.causer)) {
				// a player revives another object
				hostile = false;
			}
		}
		// initialize AI
		ASSERT(s.script != nullptr);
		//auto& script = *s.script;
		//script("onSpawn", s.api.get());
		s.api->hostile = hostile;
	}
	*/
	
	// forward respawn
	send(event);
}

void Factory::handle(ReleaseEvent const & event) {
	// release as soon as possible
	release.push(event.actor, sf::Time::Zero);
}

void Factory::update(sf::Time const& elapsed) {
	dispatch<rpg::ProjectileEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<ReleaseEvent>(*this);

	release(elapsed);
	for (auto id : release.ready) {
		destroyObject(id);
	}
	release.ready.clear();

	propagate<core::InputEvent>();
	propagate<rpg::ActionEvent>();
	propagate<rpg::ItemEvent>();
	propagate<rpg::StatsEvent>();
	propagate<rpg::SpawnEvent>();
	propagate<PowerupEvent>();
}

void Factory::reset() {
	entity_cache.clear();
	entity_cache.resize(max_num_players);
	release.reset();
}

}  // ::rage
