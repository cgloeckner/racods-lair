#pragma once
#include <core/dungeon.hpp>
#include <rpg/combat.hpp>
#include <game/mod.hpp>
#include <game/session.hpp>
#include <game/factory.hpp>
#include <game/path.hpp>

#include <engine/common.hpp>
#include <engine/ai.hpp>
#include <engine/avatar.hpp>
#include <engine/event.hpp>
#include <engine/behavior.hpp>
#include <engine/physics.hpp>
#include <engine/ui.hpp>

namespace engine {

std::string get_preference_dir(std::string const& app_name=PROJECT);
std::string get_lightmap_filename();
bool load_lightmap(core::LogContext& log, game::ResourceCache& cache);
bool create_lightmap(core::LogContext& log, game::ResourceCache& cache);
sf::Texture const& get_lightmap(
	core::LogContext& log, game::ResourceCache& cache);

// ---------------------------------------------------------------------------

struct Engine {
	core::IdManager id_manager;
	core::DungeonSystem dungeon;

	engine::PhysicsSystem physics;
	engine::AvatarSystem avatar;
	engine::UiSystem ui;
	engine::BehaviorSystem behavior;
	engine::AiSystem ai;

	rpg::CombatSystem combat;

	game::DungeonGenerator generator;
	game::Session session;
	game::Mod& mod;
	game::Factory factory;

	Engine(core::LogContext& log, std::size_t max_objects, sf::Vector2u const& screen_size,
		float zoom, unsigned int poolsize, game::Mod& mod,
		game::ResourceCache& cache, game::Localization& locale);
	
	void connect(MultiEventListener& listener);
	void disconnect(MultiEventListener& listener);
	
	core::CameraData const * getCamera(sf::Vector2f const & screen_pos) const;
	core::Dungeon const & getDungeon(core::CameraData const & cam) const;
	
	sf::Vector2f getWorldPos(sf::Vector2f const & screen_pos) const;
	void snapGrid(sf::Vector2f& screen_pos) const;
	
	void clear();
};

}  // ::state
