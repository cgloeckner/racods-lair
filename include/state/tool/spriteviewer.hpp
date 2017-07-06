#pragma once
#include <boost/optional/optional.hpp>
#include <utils/enum_map.hpp>

#include <core/dungeon.hpp>
#include <core/movement.hpp>
#include <core/focus.hpp>
#include <core/animation.hpp>
#include <core/render.hpp>

#include <rpg/common.hpp>
#include <game/mod.hpp>
#include <ui/imgui.hpp>
#include <state/common.hpp>

namespace tool {

class SpriteViewerState
	: public state::State {
  protected:
	std::string modname;
	core::LogContext log;
	game::ResourceCache cache;
	std::unique_ptr<game::Mod> mod;
	
	int sprite_index, action_index;
	std::vector<std::string> sprite, action;
	
	struct EquipData {
		int index{-1};
		std::vector<std::string> data;
	};
	
	utils::EnumMap<rpg::EquipmentSlot, EquipData> equip;
	bool moving;
	
	bool ready;
	sf::Texture dummy;
	core::DungeonSystem dungeon;
	core::CameraSystem camera;
	utils::LightingSystem lighting;
	core::MovementSystem movement;
	core::FocusSystem focus;
	core::AnimationSystem animation;
	core::RenderSystem render;
	
	void onModType();
	
	void onUpdate(rpg::EquipmentSlot slot);
	void onAnimate();
	void onSetMove();
	void onClearClick();
	void onBackClick();
	
	void updateSprite(core::SpriteTorsoLayer layer, core::AnimationEvent::TorsoAnimation const * ptr, sf::Texture const * tex);
	void updateSprite(core::SpriteLegLayer layer, core::AnimationEvent::LegAnimation const * ptr, sf::Texture const * tex);
	void onUpdateSprite();
	void onUpdateEquipment(rpg::EquipmentSlot slot, core::SpriteTorsoLayer torso, boost::optional<core::SpriteLegLayer> leg=boost::none);
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
  public:
	SpriteViewerState(state::App& app);
	
	void handle(sf::Event const & event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::tool
