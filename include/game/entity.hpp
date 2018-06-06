#pragma once
#include <memory>

#include <utils/lua_utils.hpp>
#include <core/common.hpp>
#include <core/entity.hpp>
#include <rpg/common.hpp>
#include <ui/playerhud.hpp>
#include <game/lua.hpp>

namespace game {

struct TracerData : core::ComponentData {
	std::future<Path> request;
	Path path; /// @note path is traverse backwards for better performance
	bool is_enabled;
	
	TracerData();
};

struct HudData : core::ComponentData {
	std::unique_ptr<ui::PlayerHud> hud;

	HudData();
};

struct ScriptData : core::ComponentData {
	bool is_active;
	//std::unique_ptr<LuaApi> api;
	utils::Script const* script;

	ScriptData();
};

// ---------------------------------------------------------------------------
// enhanced component managers

using TracerManager = core::ComponentManager<TracerData>;
using HudManager = core::ComponentManager<HudData>;
using ScriptManager = core::ComponentManager<ScriptData>;

} // ::game
