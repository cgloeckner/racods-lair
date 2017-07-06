#pragma once
#include <string>
#include <memory>
#include <map>

#include <engine/engine.hpp>
#include <state/tool/testmode.hpp>

namespace tool {

template <typename T>
bool createInspector(InspectorMap& map, core::ComponentManager<T>& system, core::LogContext& log, engine::Engine& engine, core::ObjectID id);

// --------------------------------------------------------------------

template <typename T>
struct ComponentInspector: BaseInspector {
	using BaseInspector::BaseInspector;
	
	void update() override;
};

template <>
struct ComponentInspector<core::AnimationData>: BaseInspector {
	using BaseInspector::BaseInspector;
	
	int action_index{-1};
	std::vector<std::string> actions;
	
	void update() override;
};

template <>
struct ComponentInspector<core::RenderData>: BaseInspector {
	using BaseInspector::BaseInspector;
	
	int layer_index{-1};
	std::vector<std::string> layers;
	
	void update() override;
};

template <>
struct ComponentInspector<rpg::ItemData>: BaseInspector {
	using BaseInspector::BaseInspector;
	
	int quantity, equip_index{-1}, names_index{-1}, slots_index{-1};
	std::vector<std::string> all_names, slots;
	std::vector<rpg::ItemTemplate const *> all_items;
	
	void update() override;
};

template <>
struct ComponentInspector<rpg::PerkData>: BaseInspector {
	using BaseInspector::BaseInspector;
	
	int level, names_index{-1};
	std::vector<std::string> all_names;
	std::vector<rpg::PerkTemplate const *> all_perks;
	
	void update() override;
};

template <>
struct ComponentInspector<rpg::EffectData>: BaseInspector {
	using BaseInspector::BaseInspector;
	
	int names_index{-1};
	std::vector<std::string> all_names;
	std::vector<rpg::EffectTemplate const *> all_effects;
	
	void update() override;
};

template <>
struct ComponentInspector<rpg::QuickslotData>: BaseInspector {
	using BaseInspector::BaseInspector;
	
	std::size_t slot;
	int names_index{-1};
	std::vector<std::string> all_names;
	std::vector<rpg::ItemTemplate const *> items;
	std::vector<rpg::PerkTemplate const *> perks;
	
	void refresh() override;
	void update() override;
};

} // ::tool

// include implementation details
#include <state/tool/inspector.inl>
