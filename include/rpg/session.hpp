#pragma once
#include <core/dungeon.hpp>
#include <core/entity.hpp>
#include <rpg/entity.hpp>

namespace rpg {

struct Session {
	core::IdManager& id_manager;
	core::DungeonSystem& dungeon;
	core::CameraSystem& camera;
	core::MovementManager& movement;
	core::CollisionManager& collision;
	core::FocusManager& focus;
	core::AnimationManager& animation;
	core::RenderManager& render;
	core::SoundManager& sound;

	rpg::StatsManager& stats;
	rpg::EffectManager& effect;
	rpg::ItemManager& item;
	rpg::PerkManager& perk;
	rpg::PlayerManager& player;
	rpg::ProjectileManager& projectile;
	rpg::ActionManager& action;
	rpg::InputManager& input;
	rpg::InteractManager& interact;
	rpg::QuickslotManager& quickslot;

	Session(core::IdManager& id_manager, core::DungeonSystem& dungeon,
		core::CameraSystem& camera, core::MovementManager& movement,
		core::CollisionManager& collision, core::FocusManager& focus,
		core::AnimationManager& animation, core::RenderManager& render,
		core::SoundManager& sound, rpg::StatsManager& stats,
		rpg::EffectManager& effect, rpg::ItemManager& item,
		rpg::PerkManager& perk, rpg::PlayerManager& player,
		rpg::ProjectileManager& projectile, rpg::ActionManager& action,
		rpg::InputManager& input, rpg::InteractManager& interact,
		rpg::QuickslotManager& quickslot);
};

}  // ::game
