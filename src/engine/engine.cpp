#include <utils/filesystem.hpp>
#include <engine/engine.hpp>

namespace engine {

std::string get_preference_dir(std::string const& app_name) {
	std::string pref_dir;
// determine preferences base path
#if defined(SFML_SYSTEM_WINDOWS)
	pref_dir = std::getenv("APPDATA");
	pref_dir += "\\";
#elif defined(SFML_SYSTEM_LINUX)
	pref_dir = std::getenv("HOME");
	pref_dir += "/.local/share/";
#else
#error This operation system is not supported, yet
#endif
	return pref_dir + app_name + "/";
}

std::string get_lightmap_filename() {
	return get_preference_dir() + "cache/lightmap.png";
}

bool load_lightmap(core::LogContext& log, game::ResourceCache& cache) {
	auto lightmap_file = get_lightmap_filename();
	if (utils::file_exists(lightmap_file)) {
		// try to load and verify lightmap
		try {
			auto& lightmap = cache.get<sf::Texture>(lightmap_file);
			auto estimated =
				static_cast<unsigned int>(utils::MAX_LIGHT_RADIUS * 1.5f);
			auto size = lightmap.getSize();
			if (size.x != estimated || size.y != estimated) {
				log.error << "[ngine] " << "Corrupted Lightmap\n";
				log.error.flush();
				return false;
			}
		} catch (...) {
			// cannot load lightmap
			return false;
		}
		// lightmap is ok
		return true;
	} else {
		// lightmap not found
		return false;
	}
}

bool create_lightmap(core::LogContext& log, game::ResourceCache& cache) {
	if (!sf::Shader::isAvailable()) {
		log.error << "[Engine/Engine] " << "Cannot render Lightmap: No shaders available\n";
		log.error.flush();
		return false;
	}
	// load fragment shader for light calculation
	sf::Shader shader;
	shader.loadFromMemory(
		"uniform vec2 center;"
		"uniform float radius;"
		"uniform float intensity;"
		"void main() {"
		"	float dist = distance(gl_TexCoord[0].xy, center);"
		"	float color = exp(-9.0*dist/radius);"
		"	gl_FragColor = vec4(gl_Color.xyz * color, 1.0);"
		"}",
		sf::Shader::Fragment);
	auto lightmap =
		utils::createLightmap(utils::MAX_LIGHT_RADIUS, shader).copyToImage();
	auto lightmap_file = get_lightmap_filename();
	bool exists = utils::file_exists(lightmap_file);
	if (!lightmap.saveToFile(lightmap_file)) {
		log.error << "[Engine/Engine] " << "Cannot save Lightmap\n";
		log.error.flush();
		return false;
	}
	if (exists) {
		// force cache to reload
		cache.get<sf::Texture>(lightmap_file, true);
	}
	return true;
}

sf::Texture const& get_lightmap(
	core::LogContext& log, game::ResourceCache& cache) {
	if (!load_lightmap(log, cache) && !create_lightmap(log, cache)) {
		std::abort();
	}
	return cache.get<sf::Texture>(get_lightmap_filename());
}

// ---------------------------------------------------------------------------

Engine::Engine(core::LogContext& log, std::size_t max_objects,
	sf::Vector2u const& screen_size, float zoom, unsigned int poolsize,
	game::Mod& mod, game::ResourceCache& cache, game::Localization& locale)
	: id_manager{max_objects}
	, dungeon{}
	, physics{log, max_objects, dungeon}
	, avatar{log, max_objects}
	, ui{log, max_objects, screen_size, get_lightmap(log, cache), zoom, poolsize,
		  physics.movement, physics.focus, dungeon, avatar.stats,
		  avatar.item, avatar.player, locale, mod.get_path<sf::Music>(),
		  mod.get_ext<sf::Music>()}
	, behavior{log, max_objects, dungeon, physics.movement, physics.focus, ui.animation,
		  avatar.item, avatar.stats, avatar.player}
	, ai{log, max_objects}
	, combat{log, physics.movement, physics.projectile, avatar.perk,
		  avatar.stats, behavior.interact, 0.f}
	, generator{log}
	, session{id_manager, dungeon, ui.camera, physics.movement,
		  physics.collision, physics.focus, ui.animation, ui.render,
		  avatar.stats, avatar.effect, avatar.item, avatar.perk, avatar.player,
		  physics.projectile, behavior.action, behavior.input,
		  behavior.interact, avatar.quickslot, ui.audio, generator,
		  ai.navigation, ai.script, ui.hud, ai.path}
	, mod{mod}
	, factory{log, session, mod} {
	log.debug << "[Engine/Engine] Initialized with max_objects="
		<< max_objects << "\n";
	// propagate available rooms to dungeon generator
	generator.rooms = mod.getAll<game::RoomTemplate>();

	// setup behavior notifications
	behavior.bind<core::InputEvent>(physics);	  // move request
	behavior.bind<core::AnimationEvent>(ui);	   // action/move request
	behavior.bind<rpg::ActionEvent>(avatar);	   // action request
	behavior.bind<rpg::CombatEvent>(combat);	   // regular combat request
	behavior.bind<rpg::ActionEvent>(ui);		   // ui response
	behavior.bind<rpg::ProjectileEvent>(factory);  // projectile creation
	behavior.bind<rpg::ItemEvent>(avatar);		   // looting

	// setup physics notifications
	physics.bind<core::MoveEvent>(behavior);	   // move response
	physics.bind<core::MoveEvent>(ai);			   // move response
	physics.bind<core::MoveEvent>(ui);			   // move response to audio (walk sfx)
	physics.bind<core::CollisionEvent>(behavior);  // collision response
	physics.bind<core::CollisionEvent>(ai);		   // collision response
	physics.bind<core::TeleportEvent>(ai);		   // ai response
	physics.bind<core::TeleportEvent>(ui);		   // ui response
	physics.bind<core::FocusEvent>(ai);			   // focus response
	physics.bind<core::FocusEvent>(ui);			   // focus response
	physics.bind<rpg::CombatEvent>(combat);		   // combat request by projectiles
	physics.bind<rpg::ProjectileEvent>(factory);   // projectile destruction
	physics.bind<rpg::ProjectileEvent>(ui);		   // projectile destruction

	// setup avatar notifications
	avatar.bind<core::AnimationEvent>(ui);   // equipment change
	avatar.bind<core::SpriteEvent>(ui);		 // equipment change
	avatar.bind<rpg::StatsEvent>(ai);		 // stats notification
	avatar.bind<rpg::StatsEvent>(ui);		 // stats notification
	avatar.bind<rpg::ExpEvent>(ui);			 // exp notification
	avatar.bind<rpg::FeedbackEvent>(ai);	 // feedback notification
	avatar.bind<rpg::FeedbackEvent>(ui);	 // feedback notification
	avatar.bind<rpg::FeedbackEvent>(behavior);	 // feedback notification
	avatar.bind<rpg::DeathEvent>(behavior);  // death notification
	avatar.bind<rpg::DeathEvent>(ai);		 // death notification
	avatar.bind<rpg::DeathEvent>(ui);		 // death notification
	avatar.bind<rpg::DeathEvent>(factory);   // death notification
	avatar.bind<rpg::EffectEvent>(avatar);   // effect faded notification
	avatar.bind<rpg::EffectEvent>(ai);		 // effect faded notification
	avatar.bind<rpg::CombatEvent>(combat);   // combat request by effects
	avatar.bind<rpg::PerkEvent>(behavior);   // perk usage (after mana consume)
	avatar.bind<rpg::PerkEvent>(ui);   		 // audio notification
	avatar.bind<rpg::ItemEvent>(ui);   		 // audio notification

	// setup combat notifications
	combat.bind<rpg::StatsEvent>(avatar);		 // stats response
	combat.bind<rpg::ExpEvent>(avatar);			 // exp response
	combat.bind<rpg::EffectEvent>(avatar);		 // effect response
	combat.bind<rpg::ProjectileEvent>(factory);  // projectile destruction
	combat.bind<rpg::ProjectileEvent>(ui);		 // projectile destruction
	combat.bind<rpg::SpawnEvent>(behavior);	// respawn notification

	// setup ui notifications
	ui.bind<core::AnimationEvent>(behavior);  // action update

	// setup factory notifications
	factory.bind<core::InputEvent>(physics);	// projectile & ai movement
	factory.bind<rpg::ActionEvent>(behavior);	// ai actions
	factory.bind<rpg::ItemEvent>(avatar);		// player/ai equipment
	factory.bind<rpg::StatsEvent>(avatar);		// regeneration
	factory.bind<rpg::SpawnEvent>(avatar);	// respawn handling
	factory.bind<rpg::SpawnEvent>(behavior);	// respawn handling
	factory.bind<rpg::SpawnEvent>(ai);		// respawn handling
	factory.bind<rpg::SpawnEvent>(ui);		// respawn handling
	factory.bind<game::PowerupEvent>(ui);		// ui response
}

void Engine::connect(MultiEventListener& listener) {
	behavior.bind<core::InputEvent>(listener);
	behavior.bind<rpg::ActionEvent>(listener);
	behavior.bind<rpg::CombatEvent>(listener);
	behavior.bind<rpg::ProjectileEvent>(listener);
	behavior.bind<rpg::ItemEvent>(listener);
	behavior.connect(listener);
	
	physics.bind<core::MoveEvent>(listener);
	physics.bind<core::CollisionEvent>(listener);
	physics.bind<core::TeleportEvent>(listener);
	physics.bind<core::FocusEvent>(listener);
	physics.bind<rpg::CombatEvent>(listener);
	physics.bind<rpg::ProjectileEvent>(listener);
	physics.connect(listener);
	
	avatar.bind<core::SpriteEvent>(listener);
	avatar.bind<rpg::StatsEvent>(listener);
	avatar.bind<rpg::ExpEvent>(listener);
	avatar.bind<rpg::FeedbackEvent>(listener);
	avatar.bind<rpg::DeathEvent>(listener);
	avatar.bind<rpg::EffectEvent>(listener);
	avatar.bind<rpg::PerkEvent>(listener);
	avatar.connect(listener);
	
	combat.bind<rpg::StatsEvent>(listener);
	combat.bind<rpg::ExpEvent>(listener);
	combat.bind<rpg::EffectEvent>(listener);
	combat.bind<rpg::ProjectileEvent>(listener);
	combat.bind<rpg::SpawnEvent>(listener);
	// note: combat is no engine system, thus not connectable

	ui.bind<core::AnimationEvent>(listener);
	ui.connect(listener);
	
	factory.bind<core::InputEvent>(listener);
	factory.bind<rpg::ActionEvent>(listener);
	factory.bind<rpg::ItemEvent>(listener);
	factory.bind<rpg::StatsEvent>(listener);
	factory.bind<game::PowerupEvent>(listener);
	// note: factory is no engine system, thus not connectable
}

void Engine::disconnect(MultiEventListener& listener) {
	behavior.unbind<core::InputEvent>(listener);
	behavior.unbind<core::AnimationEvent>(listener);
	behavior.unbind<rpg::ActionEvent>(listener);
	behavior.unbind<rpg::CombatEvent>(listener);
	behavior.unbind<rpg::ProjectileEvent>(listener);
	behavior.unbind<rpg::ItemEvent>(listener);
	behavior.disconnect(listener);
	
	physics.unbind<core::MoveEvent>(listener);
	physics.unbind<core::CollisionEvent>(listener);
	physics.unbind<core::TeleportEvent>(listener);
	physics.unbind<core::FocusEvent>(listener);
	physics.unbind<rpg::CombatEvent>(listener);
	physics.unbind<rpg::ProjectileEvent>(listener);
	physics.disconnect(listener);

	avatar.unbind<core::AnimationEvent>(listener);
	avatar.unbind<core::SpriteEvent>(listener);
	avatar.unbind<rpg::StatsEvent>(listener);
	avatar.unbind<rpg::ExpEvent>(listener);
	avatar.unbind<rpg::FeedbackEvent>(listener);
	avatar.unbind<rpg::DeathEvent>(listener);
	avatar.unbind<rpg::EffectEvent>(listener);
	avatar.unbind<rpg::PerkEvent>(listener);
	avatar.disconnect(listener);

	combat.unbind<rpg::StatsEvent>(listener);
	combat.unbind<rpg::ExpEvent>(listener);
	combat.unbind<rpg::EffectEvent>(listener);
	combat.unbind<rpg::ProjectileEvent>(listener);
	combat.unbind<rpg::SpawnEvent>(listener);
	// note: combat is no engine system, thus not connectable

	ui.unbind<core::AnimationEvent>(listener);
	ui.disconnect(listener);

	factory.unbind<core::InputEvent>(listener);
	factory.unbind<rpg::ActionEvent>(listener);
	factory.unbind<rpg::ItemEvent>(listener);
	factory.unbind<rpg::StatsEvent>(listener);
	factory.unbind<game::PowerupEvent>(listener);
	// note: factory is no engine system, thus not connectable
}

core::CameraData const * Engine::getCamera(sf::Vector2f const & screen_pos) const {
	// determine camera
	core::CameraData const * cam{nullptr};
	auto window_size = session.camera.getWindowSize();
	for (auto const & uptr: session.camera) {
		auto box = uptr->screen.getViewport();
		box.left *= window_size.x;
		box.width *= window_size.x;
		box.top *= window_size.y;
		box.height *= window_size.y;
		if (box.contains(screen_pos)) {
			cam = uptr.get();
			break;
		}
	}
	return cam;
}

core::Dungeon const & Engine::getDungeon(core::CameraData const & cam) const {
	ASSERT(!cam.objects.empty());
	return dungeon[session.movement.query(cam.objects.front()).scene];
}

sf::Vector2f Engine::getWorldPos(sf::Vector2f const & screen_pos) const {
	auto const ptr = getCamera(screen_pos);
	if (ptr == nullptr) {
		// outside screen
		return sf::Vector2f(-1.f, -1.f);
	}
	auto const & cam = *ptr;
	auto const & dungeon = getDungeon(cam);
	// make screen position relative to camera
	auto const window_size = session.camera.getWindowSize();
	auto const box = cam.screen.getViewport();
	auto spos = screen_pos;
	spos.x -= box.left * window_size.x;
	spos.y -= box.top * window_size.y;
	// determine camera position
	auto const cam_pos = cam.bary_center;
	auto const cam_size = sf::Vector2f{cam.screen.getSize()} * cam.zoom;
	auto const world_pos = spos * cam.zoom + cam_pos - cam_size / 2.f;
	auto tmp = dungeon.fromScreen(world_pos);
	tmp.x += 0.5f; // tiles are rendered centered
	tmp.y += 0.5f;
	return tmp;
}

void Engine::snapGrid(sf::Vector2f& screen_pos) const {
	auto const ptr = getCamera(screen_pos);
	if (ptr == nullptr) {
		// outside screen
		return;
	}
	auto const & cam = *ptr;
	auto const & dungeon = getDungeon(cam);
	auto const cam_pos = cam.bary_center;
	auto const cam_size = sf::Vector2f{cam.screen.getSize()} * cam.zoom;
	auto const world_pos = screen_pos * cam.zoom + cam_pos - cam_size / 2.f;
	auto tmp = dungeon.fromScreen(world_pos);
	tmp.x -= 0.5f; // tiles are rendered centered!
	tmp.y -= 0.5f;
	tmp = sf::Vector2f{sf::Vector2u{tmp}}; // snap to grid!
	tmp.x += 0.5f; // tiles are rendered centered!
	tmp.y += 0.5f;
	screen_pos = dungeon.toScreen(tmp);
}

void Engine::clear() {
	// query and release all entities
	for (auto const & d: dungeon) {
		ASSERT(d != nullptr);
		sf::Vector2u pos;
		auto size = d->getSize();
		for (pos.y = 0; pos.y < size.y; ++pos.y) {
			for (pos.x = 0; pos.x < size.x; ++pos.x) {
				if (!d->has(pos)) {
					continue;
				}
				auto& cell = d->getCell(pos);
				for (auto id: cell.entities) {
					for (auto ptr: session.systems) {
						ptr->tryRelease(id);
					}
				}
			}
		}
	}
	// reset systems
	for (auto ptr: session.systems) {
		ptr->cleanup();
	}
	id_manager.reset();
	dungeon.clear();
	generator.clear();
	physics.clear();
	avatar.clear();
	ui.clear();
	behavior.clear();
	ai.clear();
	combat.clear();
	factory.reset();
}

}  // ::state
