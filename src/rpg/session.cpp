#include <rpg/session.hpp>

namespace rpg {

Session::Session(core::IdManager& id_manager, core::DungeonSystem& dungeon,
	core::CameraSystem& camera, core::MovementManager& movement,
	core::CollisionManager& collision, core::FocusManager& focus,
	core::AnimationManager& animation, core::RenderManager& render,
	core::SoundManager& sound, rpg::StatsManager& stats,
	rpg::EffectManager& effect, rpg::ItemManager& item, rpg::PerkManager& perk,
	rpg::PlayerManager& player, rpg::ProjectileManager& projectile,
	rpg::ActionManager& action, rpg::InputManager& input,
	rpg::InteractManager& interact, rpg::QuickslotManager& quickslot)
	: id_manager{id_manager}
	, dungeon{dungeon}
	, camera{camera}
	, movement{movement}
	, collision{collision}
	, focus{focus}
	, animation{animation}
	, render{render}
	, sound{sound}
	, stats{stats}
	, effect{effect}
	, item{item}
	, perk{perk}
	, player{player}
	, projectile{projectile}
	, action{action}
	, input{input}
	, interact{interact}
	, quickslot{quickslot} {}

}  // ::game
