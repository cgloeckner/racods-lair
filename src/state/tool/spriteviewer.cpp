#include <sstream>
#include <utils/algorithm.hpp>
#include <utils/filesystem.hpp>

#include <core/teleport.hpp>

#include <rpg/resources.hpp>
#include <ui/imgui.hpp>
#include <state/tool/spriteviewer.hpp>

namespace tool {

SpriteViewerState::SpriteViewerState(state::App& app)
	: state::State{app}
	, modname{"./data"}
	, log{}
	, cache{}
	, mod{nullptr}
	, sprite_index{-1}
	, action_index{-1}
	, sprite{}
	, action{}
	, equip{}
	, moving{false}
	, ready{false}
	, dummy{}
	, dungeon{}
	, camera{{128u, 128u}, 1.f}
	, lighting{{128u, 128u}, dummy}
	, movement{log, 10u, dungeon}
	, focus{log, 10u, dungeon, movement}
	, animation{log, 10u}
	, render{log, 10u, animation, movement, dungeon, camera, lighting} {
	// setup dummy dungeon
	auto scene = dungeon.create(dummy, sf::Vector2u{3u, 3u}, sf::Vector2f{32.f, 32.f});
	ASSERT(scene == 1u);
	// spawn object
	camera.acquire().objects.push_back(1u);
	core::spawn(dungeon[1u], movement.acquire(1u), {1u, 1u});
	animation.acquire(1u);
	render.acquire(1u);
	
	for (auto v: utils::EnumRange<core::AnimationAction>{}) {
		action.emplace_back(core::to_string(v));
	}
	
	onModType();
}

void SpriteViewerState::onModType() {
	sprite.clear();
	sprite_index = -1;
	for (auto& pair: equip) {
		pair.second.data.clear();
		pair.second.index = -1;
	}
	
	if (!utils::file_exists(modname)) {
		return;
	}
	
	mod = std::make_unique<game::Mod>(log, cache, std::string{modname});
	
	auto loader = [&](std::vector<std::string>& target, std::string const & path) {
		target.clear();
		utils::for_each_file(path, ".xml", [&](std::string const & p, std::string const & key) {
			target.push_back(game::mod_impl::concat(p, key));
		});
	};
	
	std::vector<std::string> items;
	loader(sprite, mod->get_path<rpg::EntityTemplate>());
	loader(items, mod->get_path<rpg::ItemTemplate>());
	for (auto const & name: items) {
		auto const & tpl = mod->get<rpg::ItemTemplate>(name);
		equip[tpl.slot].data.push_back(name);
	}
}

void SpriteViewerState::onUpdate(rpg::EquipmentSlot slot) {
	if (mod == nullptr) {
		return;
	}
	
	switch (slot) {
		case rpg::EquipmentSlot::None:
			onUpdateSprite();
			break;
			
		case rpg::EquipmentSlot::Weapon:
			onUpdateEquipment(slot, core::SpriteTorsoLayer::Weapon);
			break;
			
		case rpg::EquipmentSlot::Body:
			onUpdateEquipment(slot, core::SpriteTorsoLayer::Armor, core::SpriteLegLayer::Armor);
			break;
			
		case rpg::EquipmentSlot::Extension:
			onUpdateEquipment(slot, core::SpriteTorsoLayer::Shield);
			break;
			
		case rpg::EquipmentSlot::Head:
			onUpdateEquipment(slot, core::SpriteTorsoLayer::Helmet);
			break;
	}
}

void SpriteViewerState::onAnimate() {
	if (!ready) {
		return;
	}
	
	core::AnimationEvent event;
	event.actor = 1u;
	event.type = core::AnimationEvent::Action;
	event.action = core::AnimationAction::Idle;
	if (action_index > -1) {
		event.action = core::from_string<core::AnimationAction>(action[action_index]);
	}
	animation.handle(event);
	
	if (event.action == core::AnimationAction::Die) {
		moving = false;
	}
}

void SpriteViewerState::onSetMove() {
	if (!ready) {
		return;
	}
	
	core::AnimationEvent event;
	event.actor = 1u;
	event.type = core::AnimationEvent::Move;
	event.move = moving;
	animation.handle(event);
}

void SpriteViewerState::onClearClick() {
	sprite_index = -1;
	action_index = -1;
	moving = false;
	onUpdate(rpg::EquipmentSlot::None);
	onAnimate();
	onSetMove();
}

void SpriteViewerState::onBackClick() {
	quit();
}

void SpriteViewerState::updateSprite(core::SpriteTorsoLayer layer, core::AnimationEvent::TorsoAnimation const * ptr, sf::Texture const * tex) {
	if (ptr == nullptr && layer == core::SpriteTorsoLayer::Base) {
		return;
	}
	
	core::AnimationEvent ani_event;
	ani_event.actor = 1u;
	ani_event.type = core::AnimationEvent::Torso;
	
	core::SpriteEvent spr_event;
	spr_event.actor = 1u;
	spr_event.type = core::SpriteEvent::Torso;
	
	if (ptr != nullptr) {
		// reset other torso layers if frames count mismatches
		auto const & ani_data = animation.query(1u);
		for (auto v: utils::EnumRange<core::SpriteTorsoLayer>{}) {
			if (v == layer || ani_data.tpl.torso[v] == nullptr) {
				continue;
			}
			for (auto& pair: *ani_data.tpl.torso[v]) {
				if (pair.first == core::AnimationAction::Die) {
					// ignore dying
					continue;
				}
				auto& prev = pair.second;
				auto& next = (*ptr)[pair.first];
				if (prev.frames.size() != next.frames.size()) {
					if (v == core::SpriteTorsoLayer::Base) {
						std::cout << "doesn't fit torso base: base has " << prev.frames.size() << " frames at "
							<< core::to_string(pair.first) << " vs. " << next.frames.size() << " by given animation\n";
						return;
					}
					// reset this layer
					ani_event.torso_layer = v;
					ani_event.torso = nullptr;
					animation.handle(ani_event);
					spr_event.torso_layer = v;
					spr_event.texture = nullptr;
					render.handle(spr_event);
					break;
				}
			}
		}
	}
	
	ani_event.torso_layer = layer;
	ani_event.torso = ptr;
	animation.handle(ani_event);
	
	spr_event.torso_layer = layer;
	spr_event.texture = tex;
	render.handle(spr_event);
}

void SpriteViewerState::updateSprite(core::SpriteLegLayer layer, core::AnimationEvent::LegAnimation const * ptr, sf::Texture const * tex) {
	core::AnimationEvent ani_event;
	ani_event.actor = 1u;
	ani_event.type = core::AnimationEvent::Legs;
	
	core::SpriteEvent spr_event;
	spr_event.actor = 1u;
	spr_event.type = core::SpriteEvent::Legs;
	
	if (ptr != nullptr) {
		// reset other torso layers if frames count mismatches
		auto const & ani_data = animation.query(1u);
		for (auto v: utils::EnumRange<core::SpriteLegLayer>{}) {
			if (v == layer || ani_data.tpl.legs[v] == nullptr) {
				continue;
			}
			auto& prev = *ani_data.tpl.legs[v];
			auto& next = *ptr;
			if (prev.frames.size() != next.frames.size()) {
				// reset this layer
				ani_event.leg_layer = v;
				ani_event.torso = nullptr;
				animation.handle(ani_event);
				spr_event.leg_layer = v;
				spr_event.texture = nullptr;
				render.handle(spr_event);
				break;
			}
		}
	}
	
	ani_event.leg_layer = layer;
	ani_event.legs = ptr;
	animation.handle(ani_event);
	
	spr_event.leg_layer = layer;
	spr_event.texture = tex;
	render.handle(spr_event);
}

void SpriteViewerState::onUpdateSprite() {
	if (sprite_index > -1) {
		// update sprite using selected animation
		auto name = sprite[sprite_index];
		auto& entity = mod->query<rpg::EntityTemplate>(name);
		auto& sprite = mod->query<rpg::SpriteTemplate>(entity.sprite_name);
		mod->prepare(sprite);
		
		updateSprite(core::SpriteTorsoLayer::Base, &sprite.torso, sprite.frameset);
		for (auto v: utils::EnumRange<core::SpriteTorsoLayer>{}) {
			if (v != core::SpriteTorsoLayer::Base) {
				updateSprite(v, nullptr, nullptr);
			}
		}
		if (!sprite.legs.frames.empty()) {
			updateSprite(core::SpriteLegLayer::Base, &sprite.legs, sprite.frameset);
		} else {
			for (auto v: utils::EnumRange<core::SpriteLegLayer>{}) {
				updateSprite(v, nullptr, nullptr);
			}
		}
		
		ready = true;
		
	} else {
		// reset entire sprite
		for (auto v: utils::EnumRange<core::SpriteTorsoLayer>{}) {
			updateSprite(v, nullptr, nullptr);
		}
		for (auto v: utils::EnumRange<core::SpriteLegLayer>{}) {
			updateSprite(v, nullptr, nullptr);
		}
		
		ready = false;
	}
}

void SpriteViewerState::onUpdateEquipment(rpg::EquipmentSlot slot, core::SpriteTorsoLayer torso, boost::optional<core::SpriteLegLayer> leg) {
	if (equip[slot].index > -1) {
		// update layer using select animation
		auto name = equip[slot].data[equip[slot].index];
		auto& item = mod->query<rpg::ItemTemplate>(name);
		auto& sprite = mod->query<rpg::SpriteTemplate>(item.sprite_name);
		mod->prepare(sprite);
		
		updateSprite(torso, &sprite.torso, sprite.frameset);
		if (!sprite.legs.frames.empty() && leg != boost::none) {
			updateSprite(core::SpriteLegLayer::Base, &sprite.legs, sprite.frameset);
		}
		
	} else {
		// reset layer
		updateSprite(torso, nullptr, nullptr);
		if (leg != boost::none) {
			updateSprite(*leg, nullptr, nullptr);
		}
	}
}

void SpriteViewerState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (ready) {
		auto prev = target.getDefaultView();
		sf::View view{{0.f, 0.f, 128.f, 128.f}};
		target.setView(view);
		target.draw(render);
		target.setView(prev);
	}
	
	ImGui::Render();
}

void SpriteViewerState::handle(sf::Event const & event) {
	ImGui::SFML::ProcessEvent(event);
	
	switch (event.type) {
		case sf::Event::Closed:
			onBackClick();
			break;
			
		case sf::Event::Resized: {
			sf::Vector2u size{event.size.width, event.size.height};
			camera.resize(size);
			lighting.resize(size);
		} break;
		
		default:
			break;
	}
}

void SpriteViewerState::update(sf::Time const & elapsed) {
	auto& window = getApplication().getWindow();
	
	ImGui::SFML::Update(window, elapsed);
	
	ImGui::Begin("Base Sprite");
		if (ui::InputText("Mod path", modname)) {
			onModType();
		}
		
		if (ui::ListBox("Sprite", sprite_index, sprite)) {
			onUpdate(rpg::EquipmentSlot::None);
		}
		
		if (ImGui::Button("Back")) {
			onBackClick();
		}
	ImGui::End();
	
	if (ready) {
		ImGui::Begin("Equipment Layers");
			for (auto& pair: equip) {
				if (pair.second.data.empty()) {
					continue;
				}
				if (pair.first != rpg::EquipmentSlot::None) {
					if (ui::Combo(rpg::to_string(pair.first), pair.second.index, pair.second.data)) {
						onUpdate(pair.first);
					}
				}
			}
		ImGui::End();
		
		ImGui::Begin("Animation");
			if (ImGui::Checkbox("Move animation", &moving)) {
				onSetMove();
			}
			if (ui::Combo("Action animation", action_index, action)) {
				onAnimate();
			}
			
			if (ImGui::Button("Clear")) {
				onClearClick();
			}
		ImGui::End();
		
		animation.update(elapsed);
		render.update(elapsed);
		render.cull();
	}
}

} // ::tool
