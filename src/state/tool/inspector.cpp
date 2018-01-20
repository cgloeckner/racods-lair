#include <ui/imgui.hpp>

#include <core/teleport.hpp>
#include <rpg/combat.hpp>
#include <state/tool/inspector.hpp>

namespace tool {

void showSprite(std::string const & key, sf::Sprite const & sprite) {
	ImGui::Text("%s", key.c_str());
	ImGui::NextColumn();
	ImGui::Text("Color: %s", thor::toString(sprite.getColor()).c_str());
	ImGui::Text("Texture: %p", sprite.getTexture());
	ImGui::Text("Clipping Rectangle: %s", thor::toString(sprite.getTextureRect()).c_str());
	ImGui::NextColumn();
	ImGui::Separator();
}

void editInterval(std::string const & key, utils::IntervalState& state, float min, float max, float max_speed) {
	ImGui::Text("%s", key.c_str()); ImGui::NextColumn();
	ImGui::Checkbox(("Rise##Rise" + key).c_str(), &state.rise);
	ui::InputNumber("Repeat##Repeat" + key, state.repeat, -1, 10);
	ui::SliderNumber("Speed##Speed" + key, state.speed, 0.00001f, max_speed);
	state.speed = std::max(0.00001f, state.speed);
	ui::SliderNumber("Minimum##Minimum" + key, state.min, min, max);
	ui::SliderNumber("Maximum##Maximum" + key, state.max, min, max);
	ui::SliderNumber("Current##Current" + key, state.current, state.min, state.max);
	if (state.max <= state.min) {
		state.max = state.min + state.speed;
	}
	state.current = std::max(state.min, state.current);
	state.current = std::min(state.max, state.current);
	ImGui::NextColumn();
	ImGui::Separator();
};

template <typename Key, typename Value>
void showEnumMap(utils::EnumMap<Key, Value> const & map, std::string const & prefix) {
	ImGui::Text("%s", prefix.c_str());
	ImGui::NextColumn();
	for (auto const & pair: map) {
		ImGui::BulletText("%s: %s", rpg::to_string(pair.first).c_str(), std::to_string(pair.second).c_str());
	}
	ImGui::NextColumn();
	ImGui::Separator();
}

template <typename T>
std::string concat(std::vector<T> const & data) {
	std::string dump{"{ "};
	for (auto const & v: data) {
		dump += std::to_string(v) + " ";
	}
	dump += "}";
	return dump;
}

// --------------------------------------------------------------------

template <>
void ComponentInspector<core::MovementData>::update() {
	auto& data = engine.session.movement.query(id);
	
	ImGui::Columns(2, "movement-columns");
		ImGui::Separator();
		ui::showPair("World Position", thor::toString(data.pos));
		ui::showPair("Movement Target", thor::toString(data.target));
		ui::showPair("Scene ID", std::to_string(data.scene));
		ui::showPair("Move Vector", thor::toString(data.move));
		ui::showPair("Look Vector", thor::toString(data.look));
	ImGui::Columns();
}

template <>
void ComponentInspector<core::FocusData>::update() {
	auto& data = engine.session.focus.query(id);
	
	ImGui::Columns(2, "focus-columns");
	ImGui::Separator();
	
	ui::editString("Display Name", data.display_name);
	ui::showPair("Sight Radius", std::to_string(data.sight));
	ui::showPair("FoV Angle", std::to_string(data.fov));
	
	ImGui::Columns();
}

template <>
void ComponentInspector<core::CollisionData>::update() {
	auto& data = engine.session.collision.query(id);
	
	ImGui::Columns(2, "collision-columns");
	ImGui::Separator();
	
	if (ui::editBool("Is Projectile", &data.is_projectile)) {
		if (!data.is_projectile) {
			data.radius = 0.f;
		}
	}
	if (data.is_projectile) {
		ui::editFloat("Collision Radius", data.radius, 0.f, core::collision_impl::MAX_PROJECTILE_RADIUS);
	} else {
		ui::showPair("Collision Radius", std::to_string(data.radius));
	}
	ui::showPair("Ignored Objects", concat(data.ignore));
	
	ImGui::Columns();
}

void ComponentInspector<core::AnimationData>::update() {
	auto& data = engine.session.animation.query(id);
	
	ImGui::Columns(2, "animation-columns");
	ImGui::Separator();
	
	// initialize
	if (actions.empty()) {
		for (auto action: utils::EnumRange<core::AnimationAction>{}) {
			actions.push_back(core::to_string(action));
		}
	}
	
	if (ui::editSelect("Animation Action", action_index, actions)) {
		if (action_index >= 0u) {
			data.current = core::from_string<core::AnimationAction>(actions[action_index]);
			data.torso.index = 0u;
		}
	}
	if (ui::editBool("Move Animation", &data.is_moving)) {
		data.legs.index = 0u;
	}
	ui::showPair("Torso Animation Index", std::to_string(data.torso.index));
	ui::showPair("Leg Animation Index", std::to_string(data.legs.index));
	editInterval("Brightness", data.brightness, 0.f, 1.f, 0.01f);
	editInterval("Alpha", data.alpha, 0.f, 1.f, 0.01f);
	editInterval("Min Saturation", data.min_saturation, 0.f, 1.f, 0.01f);
	editInterval("Max Saturation", data.max_saturation, 0.f, 1.f, 0.01f);
	editInterval("Light Intensity", data.light_intensity, 0.f, 1.f, 0.01f);
	editInterval("Light Radius", data.light_radius, 0.f, utils::MAX_LIGHT_RADIUS, 0.2f);
	for (auto const & pair: data.tpl.torso) {
		ui::showPair("TorsoLayer " + core::to_string(pair.first), pair.second != nullptr ? "0x" + std::to_string((size_t)pair.second) : "None");
	}
	for (auto const & pair: data.tpl.legs) {
		ui::showPair("LegLayer " + core::to_string(pair.first), pair.second != nullptr ? "0x" + std::to_string((size_t)pair.second) : "None");
	}
	
	ImGui::Columns();
}

void ComponentInspector<core::RenderData>::update() {
	auto& data = engine.session.render.query(id);
	
	ImGui::Columns(2, "render-columns");
	ImGui::Separator();
	
	// initialize
	if (layers.empty()) {
		int i{0};
		for (auto layer: utils::EnumRange<core::ObjectLayer>{}) {
			layers.push_back(core::to_string(layer));
			if (data.layer == layer) {
				layer_index = i;
			} else {
				++i;
			}
		}
	}
	
	ui::showPair("Highlight Color", data.highlight != nullptr ? thor::toString(data.highlight->getColor()) : "None");
	if (data.light == nullptr) {
		ui::showPair("Lighting", "None");
	} else {
		ui::showPair("Lighting Color", thor::toString(data.light->color));
		ui::showPair("Lighting Radius", std::to_string(data.light->radius));
		ui::showPair("Lighting Intensity", std::to_string(data.light->intensity));
	}
	if (ui::editSelect("Object Layer", layer_index, layers)) {
		if (layer_index >= 0u) {
			data.layer = core::from_string<core::ObjectLayer>(layers[layer_index]);
		}
	}
	for (auto layer: utils::EnumRange<core::SpriteLegLayer>{}) {
		showSprite("Leg " + core::to_string(layer), data.legs[layer]);
	}
	for (auto layer: utils::EnumRange<core::SpriteTorsoLayer>{}) {
		showSprite("Torso " + core::to_string(layer), data.torso[layer]);
	}
	
	ImGui::Columns();
}

template <>
void ComponentInspector<core::SoundData>::update() {
	auto& data = engine.session.sound.query(id);
	
	ImGui::Columns(2, "sound-columns");
	ImGui::Separator();
	
	for (auto const & pair: data.sfx) {
		ImGui::Text("%s", core::to_string(pair.first).c_str());
		ImGui::NextColumn();
		for (auto ptr: pair.second) {
			ImGui::BulletText("%p", ptr);
		}
		ImGui::NextColumn();
		ImGui::Separator();
	}
	
	ImGui::Columns(1);
}

template <>
void ComponentInspector<rpg::InputData>::update() {
	auto& data = engine.session.input.query(id);
	
	ImGui::Columns(2, "input-columns");
	ImGui::Separator();
	
	ui::showPair("Auto Look", data.auto_look ? "true" : "false");
	ui::showPair("Action Cooldown", utils::to_string(data.cooldown));
	for (auto value: utils::EnumRange<rpg::PlayerAction>{}) {
		auto action = data.keys.get(value);
		ui::showPair("Binding " + rpg::to_string(value), action.toString());
	}
	
	ImGui::Columns();
}

template <>
void ComponentInspector<rpg::ActionData>::update() {
	auto& data = engine.session.action.query(id);
	
	ImGui::Columns(2, "action-columns");
	ImGui::Separator();
	
	ui::showPair("Idle", data.idle ? "true" : "false");
	ui::showPair("Moving", data.moving ? "true" : "false");
	ui::showPair("Dead", data.dead ? "true" : "false");
	
	ImGui::Columns();
}

void ComponentInspector<rpg::ItemData>::update() {
	auto& data = engine.session.item.query(id);
	
	ImGui::Columns(2, "item-columns");
	ImGui::Separator();
	
	// initialize
	if (slots.empty()) {
		for (auto value: utils::EnumRange<rpg::EquipmentSlot>{}) {
			slots.push_back(rpg::to_string(value));
		}
		quantity = 1;
		equip_index = -1;
		all_items = engine.mod.getAll<rpg::ItemTemplate>();
		all_names.reserve(all_items.size());
		for (auto ptr: all_items) {
			all_names.push_back(ptr->display_name);
		}
	}
	
	// show inventory with incr/decr ui
	ImGui::Text("Inventory");
	ImGui::NextColumn();
	for (auto const & pair: data.inventory) {
		auto section = rpg::to_string(pair.first);
		std::size_t i{0u};
		for (auto const & node: pair.second) {
			ASSERT(node.item != nullptr);
			ImGui::BulletText("%s (%s): x%lu", node.item->display_name.c_str(), section.c_str(), node.quantity);
			ImGui::SameLine();
			if (ImGui::Button(("-##DecrItemQuant" + std::to_string(i)).c_str())) {
				// decrease item quantity
				rpg::ItemEvent event;
				event.actor = id;
				event.item = node.item;
				event.quantity = 1u;
				event.type = rpg::ItemEvent::Remove;
				engine.avatar.handle(event);
			}
			ImGui::SameLine();
			if (ImGui::Button(("+##IncrItemQuant" + std::to_string(i)).c_str())) {
				// increase item quantity
				rpg::ItemEvent event;
				event.actor = id;
				event.item = node.item;
				event.quantity = 1u;
				event.type = rpg::ItemEvent::Add;
				engine.avatar.handle(event);
			}
			++i;
		}
	}
	
	// show item add ui
	ui::Combo("Item##AddItemName", names_index, all_names);
	ui::InputNumber("Quantity##AddItemQuant", quantity, 1, 1000);
	if (ImGui::Button("Add Item") && names_index >= 0) {
		// add item
		rpg::ItemEvent event;
		event.actor = id;
		event.item = all_items[names_index];
		event.quantity = static_cast<unsigned int>(quantity);
		event.type = rpg::ItemEvent::Add;
		engine.avatar.handle(event);
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	// show equipment
	ImGui::Text("Equipment");
	ImGui::NextColumn();
	for (auto const & pair: data.equipment) {
		if (pair.first == rpg::EquipmentSlot::None) {
			continue;
		}
		auto slot = rpg::to_string(pair.first);
		if (pair.second != nullptr) {
			ImGui::BulletText("%s: %s", slot.c_str(), pair.second->display_name.c_str());
			ImGui::SameLine();
			if (ImGui::Button(("Unequip##UnequipSlot" + slot).c_str())) {
				// unequip slot
				rpg::ItemEvent event;
				event.actor = id;
				event.item = pair.second;
				event.type = rpg::ItemEvent::Use;
				event.slot = pair.first;
				engine.avatar.handle(event);
			}
		}
	}
	
	// show change equipment ui
	if (ui::Combo("Item##EquipItemName", equip_index, all_names)) {
		// suggest slot
		auto slot = rpg::to_string(all_items[equip_index]->slot);
		slots_index = utils::find_index(slots, slot);
	}
	ui::Combo("Slot##EquipSlotName", slots_index, slots);
	if (ImGui::Button("Equip Item") && equip_index >= 0 && slots_index >= 0) {
		auto slot = rpg::from_string<rpg::EquipmentSlot>(slots[slots_index]);
		if (slot != rpg::EquipmentSlot::None) {
			rpg::ItemEvent event;
			event.actor = id;
			event.item = all_items[equip_index];
			if (!rpg::hasItem(data, *event.item)) {
				// add item
				event.type = rpg::ItemEvent::Add;
				event.quantity = 1u;
				engine.avatar.handle(event);
			}
			// equip item to slot
			event.type = rpg::ItemEvent::Use;
			event.slot = slot;
			engine.avatar.handle(event);
		}
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Columns();
}

void ComponentInspector<rpg::PerkData>::update() {
	auto& data = engine.session.perk.query(id);
	
	ImGui::Columns(2, "perk-columns");
	ImGui::Separator();
	
	// initialize
	if (all_perks.empty()) {
		level = 1;
		all_perks = engine.mod.getAll<rpg::PerkTemplate>();
		all_names.reserve(all_perks.size());
		for (auto ptr: all_perks) {
			all_names.push_back(ptr->display_name);
		}
	}
	
	// show perk ui
	ImGui::Text("Perks");
	ImGui::NextColumn();
	std::size_t i{0u};
	for (auto& node: data.perks) {
		ASSERT(node.perk != nullptr);
		ImGui::BulletText("%s Lvl. %lu", node.perk->display_name.c_str(), node.level);
		ImGui::SameLine();
		if (ImGui::Button(("-##DecrPerkLvl" + std::to_string(i)).c_str())) {
			// decrease perk level
			rpg::PerkEvent event;
			event.actor = id;
			event.perk = node.perk;
			event.type = rpg::PerkEvent::Set;
			event.level = node.level - 1u;
			engine.avatar.handle(event);
		}
		ImGui::SameLine();
		if (ImGui::Button(("+##IncrPerkLvl" + std::to_string(i)).c_str())) {
			// increase perk level
			rpg::PerkEvent event;
			event.actor = id;
			event.perk = node.perk;
			event.type = rpg::PerkEvent::Set;
			event.level = node.level + 1u;
			engine.avatar.handle(event);
		}
		++i;
	}
	
	// show perk add ui
	ui::Combo("Perk##AddPerkName", names_index, all_names);
	ui::InputNumber("Level##AddPerkLevel", level, 1, 1000);
	if (ImGui::Button("Add Perk") && names_index >= 0) {
		// add perk
		rpg::PerkEvent event;
		event.actor = id;
		event.perk = all_perks[names_index];
		event.level = static_cast<unsigned int>(level);
		event.type = rpg::PerkEvent::Set;
		engine.avatar.handle(event);
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Columns();
}

template <>
void ComponentInspector<rpg::StatsData>::update() {
	auto& data = engine.session.stats.query(id);
	auto& item_data = engine.session.item.query(id);
	
	ImGui::Columns(2, "stats-columns");
	ImGui::Separator();
	
	ui::showPair("Level", std::to_string(data.level));
	ui::editBool("God Mode", &data.godmode);
	
	// show stat ui
	ImGui::Text("Attributes");
	ImGui::NextColumn();
	for (auto& pair: data.attributes) {
		if (ui::InputNumber(rpg::to_string(pair.first).c_str(), pair.second)) {
			rpg::stats_impl::refresh(data);
		}
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Text("Stats");
	ImGui::NextColumn();
	for (auto& pair: data.stats) {
		ui::InputNumber(rpg::to_string(pair.first).c_str(), pair.second);
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	showEnumMap(data.properties, "Property");
	showEnumMap(rpg::combat_impl::getDefense(data), "Defense");
	showEnumMap(rpg::combat_impl::getWeaponDamage(data, item_data.equipment[rpg::EquipmentSlot::Weapon], nullptr), "Weapon Damage");
	
	//showEnumMap(data.base_props, "Base Property");
	//showEnumMap(data.prop_boni, "Property Bonus");
	
	ImGui::Columns();
}

void ComponentInspector<rpg::EffectData>::update() {
	auto& data = engine.session.effect.query(id);
	
	ImGui::Columns(2, "effect-columns");
	ImGui::Separator();
	
	// initialize
	if (all_effects.empty()) {
		all_effects = engine.mod.getAll<rpg::EffectTemplate>();
		all_names.reserve(all_effects.size());
		for (auto ptr: all_effects) {
			all_names.push_back(ptr->display_name);
		}
	}
	
	ui::showPair("Update Cooldown", utils::to_string(data.cooldown));
	
	// show effects and remove ui
	ImGui::Text("Active Effects");
	ImGui::NextColumn();
	std::size_t i{0u};
	for (auto const & node: data.effects) {
		ASSERT(node.effect != nullptr);
		if (node.remain == sf::Time::Zero) {
			ImGui::BulletText("%s (permanent)", node.effect->display_name.c_str());
		} else {
			ImGui::BulletText("%s (remaining %ums)", node.effect->display_name.c_str(), node.remain.asMilliseconds());
		}
		ImGui::SameLine();
		if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) {
			// remove effect
			rpg::EffectEvent event;
			event.actor = id;
			event.effect = node.effect;
			event.type = rpg::EffectEvent::Remove;
			engine.avatar.handle(event);
		}
	}
	
	// show effect add ui
	ui::Combo("Effect###EffectAddName", names_index, all_names);
	if (ImGui::Button("Add Effect") && names_index >= 0) {
		// add effect
		rpg::EffectEvent event;
		event.actor = id;
		event.effect = all_effects[names_index];
		event.type = rpg::EffectEvent::Add;
		engine.avatar.handle(event);
	};
	
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Columns();
}

void ComponentInspector<rpg::QuickslotData>::refresh() {
	all_names.clear();
	items.clear();
	perks.clear();
	slot = 0;
	auto const & item_data = engine.session.item.query(id);
	auto const & perk_data = engine.session.perk.query(id);
	std::size_t n{0u};
	for (auto const & pair: item_data.inventory) {
		n += pair.second.size();
	}
	items.reserve(n);
	perks.reserve(perk_data.perks.size());
	all_names.reserve(n + perk_data.perks.size());
	for (auto const & pair: item_data.inventory) {
		for (auto const & node: pair.second) {
			items.push_back(node.item);
			all_names.push_back(node.item->display_name + " (item)");
		}
	}
	for (auto const & node: perk_data.perks) {
		perks.push_back(node.perk);
		all_names.push_back(node.perk->display_name + " (perk)");
	}
}

void ComponentInspector<rpg::QuickslotData>::update() {
	auto& data = engine.session.quickslot.query(id);
	
	ImGui::Columns(2, "quickslot-columns");
	ImGui::Separator();
	
	// show quickslots and release/use ui
	ImGui::Text("Quickslots");
	ImGui::NextColumn();
	ui::InputNumber("Current #", data.slot_id, (std::size_t)0u, (std::size_t)(rpg::MAX_QUICKSLOTS-1u));
	for (auto i = 0u; i < rpg::MAX_QUICKSLOTS; ++i) {
		std::string slot;
		auto const & node = data.slots[i];
		if (node.perk != nullptr) {
			slot = node.perk->display_name;
		} else if (node.item != nullptr) {
			slot = node.item->display_name;
		}
		if (slot.empty()) {
			ImGui::BulletText("Slot #%u: empty", i);
		} else {
			ImGui::BulletText("Slot #%u: %s", i, slot.c_str());
			ImGui::SameLine();
			if (ImGui::Button(("Use##UseSlot" + std::to_string(i)).c_str())) {
				if (node.perk != nullptr) {
					// use slot's perk
					rpg::PerkEvent event;
					event.actor = id;
					event.perk = node.perk;
					event.type = rpg::PerkEvent::Use;
					engine.avatar.handle(event);
				} else {
					// use slot's item
					rpg::ItemEvent event;
					event.actor = id;
					event.item = node.item;
					event.type = rpg::ItemEvent::Use;
					engine.avatar.handle(event);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(("Release##ReleSlot" + std::to_string(i)).c_str())) {
				// release slot
				rpg::QuickslotEvent event;
				event.actor = id;
				event.type = rpg::QuickslotEvent::Release;
				event.item = node.item;
				event.perk = node.perk;
				engine.avatar.handle(event);
			}
		}
	}
	
	// show quickslot assign ui
	ui::Combo("Item/Perk##AssignSlotName", names_index, all_names);
	ui::SliderNumber("Slot #", slot, (std::size_t)0u, (std::size_t)(rpg::MAX_QUICKSLOTS-1lu));
	if (ImGui::Button("Assign Slot") && names_index >= 0) {
		// assign slot
		rpg::QuickslotEvent event;
		event.actor = id;
		event.type = rpg::QuickslotEvent::Assign;
		event.slot_id = static_cast<unsigned int>(slot);
		if (names_index < items.size()) {
			event.item = items[names_index];
		} else {
			event.perk = perks[items.size() - names_index];
		}
		engine.avatar.handle(event);
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Columns();
}

template <>
void ComponentInspector<rpg::PlayerData>::update() {
	auto& data = engine.session.player.query(id);
	
	ImGui::Columns(2, "player-columns");
	ImGui::Separator();
	
	ui::showPair("Player ID", std::to_string(data.player_id));
	ui::showPair("Minions", concat(data.minions));
	
	ImGui::Text("Experience");
	ImGui::NextColumn();
	ui::InputNumber("##Experience", data.exp);
	if (ImGui::Button("Level up")) {
		// trigger levelup
		rpg::ExpEvent event;
		event.actor = data.id;
		event.exp = data.exp < data.next_exp ? data.next_exp - data.exp : 0;
		engine.avatar.player.handle(event);
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	data.exp = std::min(data.exp, data.next_exp);
	ui::showPair("Required Experience", std::to_string(data.next_exp));
	ui::editInt("Stacked Experience", data.stacked_exp, "PlayerStackedExp");
	ui::editInt("Attrib Points", data.attrib_points, "PlayerAttribPoints");
	ui::editInt("Perk Points", data.perk_points, "PlayerPerkPoints");
	
	ImGui::Columns();
}

template <>
void ComponentInspector<rpg::ProjectileData>::update() {
	auto& data = engine.session.projectile.query(id);
	
	ImGui::Columns(2, "projectile-columns");
	ImGui::Separator();
	
	ui::showPair("Owner Object", std::to_string(data.owner));
	ui::showPair("Ignored Objects", concat(data.ignore));
	ui::showPair("Bullet Template", data.bullet != nullptr ? data.bullet->entity_name : "None");
	ui::showPair("Emitter Type", rpg::to_string(data.meta_data.emitter));
	ui::showPair("Combat Meta Data: Primary Weapon", data.meta_data.primary != nullptr ? data.meta_data.primary->display_name : "None");
	ui::showPair("Combat Meta Data: Scondary Weapon", data.meta_data.secondary != nullptr ? data.meta_data.secondary->display_name : "None");
	ui::showPair("Combat Meta Data: Perk", data.meta_data.perk != nullptr ? data.meta_data.perk->display_name : "None");
	ui::showPair("Combat Meta Data: Effect", data.meta_data.effect != nullptr ? data.meta_data.effect->display_name : "None");
	ui::showPair("Combat Meta Data: Trap", data.meta_data.trap != nullptr ? data.meta_data.trap->internal_name : "None");
	
	ImGui::Columns();
}

template <>
void ComponentInspector<rpg::InteractData>::update() {
	auto& data = engine.session.interact.query(id);
	
	ImGui::Columns(2, "interact-columns");
	ImGui::Separator();
	
	ui::showPair("Interact Type", rpg::to_string(data.type));
	switch (data.type) {
		case rpg::InteractType::Barrier:
			ui::showPair("Moving", data.moving ? "true" : "false");
			break;
			
		case rpg::InteractType::Corpse:
			for (auto i = 0u; i < data.loot.size(); ++i) {
				std::string loot;
				for (auto const & node: data.loot[i]) {
					ASSERT(node.item != nullptr);
					loot += node.item->display_name + " x" + std::to_string(node.quantity) + "\n";
				}
				ui::showPair("Loot for Player #" + std::to_string(i), loot);
			}
			break;
	}
	
	ImGui::Columns();
}

template <>
void ComponentInspector<game::ScriptData>::update() {
	auto& data = engine.session.script.query(id);
	
	ImGui::Columns(2, "script-columns");
	ImGui::Separator();
	
	ASSERT(data.script != nullptr);
	ASSERT(data.api != nullptr);
	ui::showPair("Script Filename", data.script->getFilename());
	ui::showPair("Hostile AI", data.api->hostile ? "true" : "false");
	
	ImGui::Columns();
}

} // ::tool
