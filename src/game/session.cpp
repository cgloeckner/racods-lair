#include <game/session.hpp>

namespace game {

Session::Session(core::IdManager& id_manager, core::DungeonSystem& dungeon,
	core::CameraSystem& camera, core::MovementManager& movement,
	core::CollisionManager& collision, core::FocusManager& focus,
	core::AnimationManager& animation, core::RenderManager& render,
	rpg::StatsManager& stats, rpg::EffectManager& effect, rpg::ItemManager& item,
	rpg::PerkManager& perk, rpg::PlayerManager& player, rpg::ProjectileManager& projectile,
	rpg::ActionManager& action, rpg::InputManager& input, rpg::InteractManager& interact,
	rpg::QuickslotManager& quickslot, AudioSystem& audio, DungeonGenerator& generator,
	NavigationSystem& navigation, /*ScriptManager& script,*/ HudManager& hud,
	PathSystem& path, TracerSystem& tracer)
	: rpg::Session{id_manager, dungeon, camera, movement, collision, focus,
		  animation, render, audio, stats, effect, item, perk, player,
		  projectile, action, input, interact, quickslot}
	, systems{}
	, audio{audio}
	, generator{generator}
	, navigation{navigation}
	//, script{script}
	, hud{hud}
	, path{path}
	, tracer{tracer} {
	systems.push_back(&movement);
	systems.push_back(&collision);
	systems.push_back(&focus);
	systems.push_back(&animation);
	systems.push_back(&render);
	systems.push_back(&audio);
	systems.push_back(&stats);
	systems.push_back(&effect);
	systems.push_back(&item);
	systems.push_back(&perk);
	systems.push_back(&player);
	systems.push_back(&projectile);
	systems.push_back(&action);
	systems.push_back(&input);
	systems.push_back(&interact);
	systems.push_back(&quickslot);
	//systems.push_back(&script);
	systems.push_back(&hud);
	systems.push_back(&tracer);
}

}  // ::rage
