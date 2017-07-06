#pragma once
#include <map>

#include <ui/imgui.hpp>
#include <engine/event.hpp>
#include <state/common.hpp>
#include <state/game.hpp>

namespace tool {

namespace monitor_impl {

struct Memory {
	std::size_t used, alloc;
	
	Memory();
};

Memory get(core::DungeonSystem const & system);

Memory get(engine::PhysicsSystem const & system);
Memory get(engine::AvatarSystem const & system);
Memory get(engine::BehaviorSystem const & system);
Memory get(engine::AiSystem const & system);
Memory get(engine::UiSystem const & system);

} // ::monitor_impl

// --------------------------------------------------------------------

struct BaseInspector {
	core::LogContext& log;
	engine::Engine& engine;
	core::ObjectID id;
	bool open;
	
	BaseInspector(core::LogContext& log, engine::Engine& engine, core::ObjectID id);
	virtual void refresh();
	virtual void update() = 0;
};

using InspectorMap = std::map<std::string, std::unique_ptr<BaseInspector>>;

// --------------------------------------------------------------------

struct MemoryMonitor {
	engine::Engine const & engine;
	
	std::map<std::string, monitor_impl::Memory> data;
	
	MemoryMonitor(engine::Engine const & engine);
	
	void update();
};

// --------------------------------------------------------------------

class TestMode
	: public state::SubState {
  private:
	state::GameState& parent;
	bool freeze, show_monitor, show_inspector, show_spawner,
		show_teleporter, show_log, show_event;
	
	// system monitor
	MemoryMonitor memory;
	
	// object inspector / spawner
	utils::SceneID scene;
	
	// object inspector / spawner / teleporter
	sf::Vector2u tile_pos, target_pos;
	
	// object spawner / teleporter
	int scenes_index, cell_entities_index;
	std::vector<std::string> scenes, cell_entities;
	
	// object inspector
	core::ObjectID object;
	InspectorMap inspectors;
	
	// object spawner
	int spawn_mode, entity_tpls_index, bot_tpls_index, ai_scripts_index;
	std::vector<std::string> entity_tpls, bot_tpls, ai_scripts;
	bool spawn_near, hostile;
	std::size_t level;
	
	// system log
	std::stringstream debug_stream, warning_stream, error_stream;
	ImGuiTextBuffer debug_log, warning_log, error_log, event_log;
	// std::unordered_map<std::type_index, ImGuiTextBuffer> event_log;
	
	// event monitor
	engine::EventLogger event_logger;
	
	void updateMonitor();
	void updateInspector();
	void updateSpawner();
	void updateTeleporter();
	void updateSystemLog();
	void updateEventLog();
	void reloadScripts();
	
	void onLeftClick();
	void onRightClick();
	void onSelectObject();
	void onSpawnModeChanged();
	
	void tryLoadScenes();
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
  public:
	TestMode(state::GameState& parent);
	~TestMode();
	
	bool handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::tool

