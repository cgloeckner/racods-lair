#pragma once
#include <rpg/session.hpp>
#include <game/audio.hpp>
#include <game/common.hpp>
#include <game/entity.hpp>
#include <game/generator.hpp>
#include <game/navigator.hpp>
#include <game/path.hpp>

namespace game {

struct Session : rpg::Session {

	std::vector<utils::BaseSystem<core::ObjectID>*> systems;
	
	AudioSystem& audio;
	DungeonGenerator& generator;
	NavigationSystem& navigation;
	//ScriptManager& script;
	HudManager& hud;
	PathSystem& path;

	Session(core::IdManager& id_manager, core::DungeonSystem& dungeon,
		core::CameraSystem& camera, core::MovementManager& movement,
		core::CollisionManager& collision, core::FocusManager& focus,
		core::AnimationManager& animation, core::RenderManager& render,
		rpg::StatsManager& stats, rpg::EffectManager& effect,
		rpg::ItemManager& item, rpg::PerkManager& perk, rpg::PlayerManager& player,
		rpg::ProjectileManager& projectile, rpg::ActionManager& action,
		rpg::InputManager& input, rpg::InteractManager& interact,
		rpg::QuickslotManager& quickslot, AudioSystem& audio,
		DungeonGenerator& generator, NavigationSystem& navigation,
		/*ScriptManager& script,*/ HudManager& hud, PathSystem& path);
};

}  // ::rage
