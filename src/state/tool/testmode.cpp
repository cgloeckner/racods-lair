#include <core/teleport.hpp>
#include <state/tool/inspector.hpp>
#include <state/tool/testmode.hpp>

namespace tool {

namespace monitor_impl {

Memory::Memory()
	: used{0u}
	, alloc{0u} {
}

Memory get(core::DungeonSystem const & system) {
	Memory mem;
	for (auto const & uptr: system) {
		auto size = uptr->getSize();
		mem.alloc += sizeof(core::Dungeon);
		mem.alloc += size.x * size.y * sizeof(core::BaseCell);
	}
	mem.used = mem.alloc;
	return mem;
}

Memory get(engine::PhysicsSystem const & system) {
	Memory mem;
	mem.used += system.movement.size() * sizeof(core::MovementData);
	mem.used += system.collision.size() * sizeof(core::CollisionData);
	mem.used += system.focus.size() * sizeof(core::FocusData);
	mem.used += system.projectile.size() * sizeof(rpg::ProjectileData);
	
	mem.alloc += system.movement.capacity() * sizeof(core::MovementData);
	mem.alloc += system.collision.capacity() * sizeof(core::CollisionData);
	mem.alloc += system.focus.capacity() * sizeof(core::FocusData);
	mem.alloc += system.projectile.capacity() * sizeof(rpg::ProjectileData);
	return mem;
}

Memory get(engine::AvatarSystem const & system) {
	Memory mem;
	mem.used += system.stats.size() * sizeof(rpg::StatsData);
	mem.used += system.effect.size() * sizeof(rpg::EffectData);
	mem.used += system.item.size() * sizeof(rpg::ItemData);
	mem.used += system.perk.size() * sizeof(rpg::PerkData);
	mem.used += system.quickslot.size() * sizeof(rpg::QuickslotData);
	mem.used += system.player.size() * sizeof(rpg::PlayerData);
	
	mem.alloc += system.stats.capacity() * sizeof(rpg::StatsData);
	mem.alloc += system.effect.capacity() * sizeof(rpg::EffectData);
	mem.alloc += system.item.capacity() * sizeof(rpg::ItemData);
	mem.alloc += system.perk.capacity() * sizeof(rpg::PerkData);
	mem.alloc += system.quickslot.capacity() * sizeof(rpg::QuickslotData);
	mem.alloc += system.player.capacity() * sizeof(rpg::PlayerData);
	return mem;
}

Memory get(engine::BehaviorSystem const & system) {
	Memory mem;
	mem.used += system.input.size() * sizeof(rpg::InputData);
	mem.used += system.action.size() * sizeof(rpg::ActionData);
	mem.used += system.interact.size() * sizeof(rpg::InteractData);
	
	mem.alloc += system.input.capacity() * sizeof(rpg::InputData);
	mem.alloc += system.action.capacity() * sizeof(rpg::ActionData);
	mem.alloc += system.interact.capacity() * sizeof(rpg::InteractData);
	
	return mem;
}


Memory get(engine::AiSystem const & system) {
	Memory mem;
	//mem.used += system.script.size() * sizeof(game::ScriptData);
	
	//mem.alloc += system.script.capacity() * sizeof(game::ScriptData);
	
	return mem;
}

Memory get(engine::UiSystem const & system) {
	Memory mem;
	mem.used += system.audio.size() * sizeof(core::SoundData);
	mem.used += system.render.size() * sizeof(core::RenderData);
	mem.used += system.animation.size() * sizeof(core::AnimationData);
	mem.used += system.hud.size() * sizeof(game::HudData);
	
	mem.alloc += system.audio.capacity() * sizeof(core::SoundData);
	mem.alloc += system.render.capacity() * sizeof(core::RenderData);
	mem.alloc += system.animation.capacity() * sizeof(core::AnimationData);
	mem.alloc += system.hud.capacity() * sizeof(game::HudData);
	
	return mem;
}

} // ::monitor_impl

// --------------------------------------------------------------------

BaseInspector::BaseInspector(core::LogContext& log, engine::Engine& engine, core::ObjectID id)
	: log{log}
	, engine{engine}
	, id{id} {
}

void BaseInspector::refresh() {
}

// --------------------------------------------------------------------

MemoryMonitor::MemoryMonitor(engine::Engine const & engine)
	: engine{engine}
	, data{} {
}

void MemoryMonitor::update() {
	data["Dungeon Tiles"] = monitor_impl::get(engine.dungeon);
	data["Physics Components"] = monitor_impl::get(engine.physics);
	data["Avatar Components"] = monitor_impl::get(engine.avatar);
	data["Behavior Components"] = monitor_impl::get(engine.behavior);
	data["Ai Components"] = monitor_impl::get(engine.ai);
	data["Ui Components"] = monitor_impl::get(engine.ui);
}

// --------------------------------------------------------------------

TestMode::TestMode(state::GameState& parent)
	: state::SubState{}
	, parent{parent}
	, freeze{false}
	, show_monitor{false}
	, show_inspector{false}
	, show_spawner{false}
	, show_teleporter{false}
	, show_log{false}
	, show_event{false}
	, render_debug{false}
	, memory{parent.getContext().game->engine}
	, scene{0u}
	, tile_pos{-1.f, -1.f}
	, target_pos{-1.f, -1.f}
	, scenes_index{-1}
	, cell_entities_index{-1}
	, scenes{}
	, cell_entities{}
	, object{0u}
	, inspectors{}
	, spawn_mode{0}
	, entity_tpls_index{-1}
	, bot_tpls_index{-1}
	, ai_scripts_index{-1}
	, entity_tpls{}
	, bot_tpls{}
	, ai_scripts{}
	, spawn_near{true}
	, hostile{true}
	, level{1u}
	, debug_stream{}
	, warning_stream{}
	, error_stream{}
	, debug_log{}
	, warning_log{}
	, error_log{}
	, event_log{}
	, event_logger{} {
	// register testmode stream
	auto& log = parent.getContext().log;
	log.debug.add(debug_stream);
	log.warning.add(warning_stream);
	log.error.add(error_stream);
	
	// setup parent's time monitor
	parent.time_monitor.setInterval(sf::milliseconds(100u));
	parent.time_monitor.setNumRecords(100u);
	parent.time_monitor.setMaxValue(200u);
	parent.time_monitor.setSize({500u, 200u});
	parent.time_monitor.setFillColor(sf::Color{0u, 0u, 0u, 64u});
	parent.time_monitor.init("combat", {128u, 255u, 0u});
	parent.time_monitor.init("ui", {255u, 128u, 0u});
	parent.time_monitor.init("avatar", sf::Color::Cyan);
	parent.time_monitor.init("physics", sf::Color::Magenta);
	parent.time_monitor.init("behavior", sf::Color::Yellow);
	parent.time_monitor.init("ai", sf::Color::Blue);
	parent.time_monitor.init("cleanup", sf::Color::Green);
	parent.time_monitor.init("draw", sf::Color::Red);
	parent.time_monitor.init("save", {0u, 255u, 128u});
	
	// connect event logger
	parent.getContext().game->engine.connect(event_logger);
	event_logger.setEnabled<core::MoveEvent>(false);
	event_logger.setEnabled<core::CollisionEvent>(false);
	event_logger.setEnabled<core::InputEvent>(false);
	event_logger.setEnabled<rpg::ActionEvent>(false);
	
	onSpawnModeChanged();
}

TestMode::~TestMode() {
	auto& log = parent.getContext().log;
	log.debug.remove(debug_stream);
	log.warning.remove(warning_stream);
	log.error.remove(error_stream);
	
	// disconnect event logger
	parent.getContext().game->engine.disconnect(event_logger);
}

void TestMode::updateMonitor() {
	memory.update();
	
	// draw elapsed time per system
	ImGui::Text("Elapsed time per system:");
	for (auto const & pair: parent.time_monitor) {
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(pair.second.color), "%s", pair.first.c_str());
	}
	auto draw_list = ImGui::GetWindowDrawList();
	sf::Vector2f topleft = ImGui::GetWindowPos();
	for (auto const & pair: parent.time_monitor) {
		auto lines = parent.time_monitor.getLines(pair.first);
		auto color = IM_COLOR(pair.second.color);
		for (auto const & pair: lines) {
			draw_list->AddLine(ImVec2(pair.first + topleft), ImVec2(pair.second + topleft), color, 1.f);
		}
	}
	ImGui::Dummy(ImVec2(parent.time_monitor.getSize()));
	
	// show memory usage per system
	ImGui::Text("Memory usage per system:");
	ImGui::Columns(4, "memory-columns");
	ImGui::Separator();
	ImGui::Text("System"); ImGui::NextColumn();
	ImGui::Text("Bytes used"); ImGui::NextColumn();
	ImGui::Text("Bytes allocated"); ImGui::NextColumn();
	ImGui::Text("Workload"); ImGui::NextColumn();
	ImGui::Separator();
	for (auto const & pair: memory.data) {
		auto perc = static_cast<unsigned int>((10000.f * pair.second.used) / pair.second.alloc) / 100.f;
		ImGui::Text("%s", pair.first.c_str()); ImGui::NextColumn();
		ImGui::Text("%'lu", pair.second.used); ImGui::NextColumn();
		ImGui::Text("%'lu", pair.second.alloc); ImGui::NextColumn();
		auto color = sf::Color::White;
		if (perc > 90) {
			color = sf::Color::Red;
		} else if (perc > 75) {
			color = {255u, 128u, 0u};
		} else if (perc > 50) {
			color = sf::Color::Yellow;
		}
		ImGui::TextColored(ImVec4(color), "%.2f %%", perc); ImGui::NextColumn();
	}
	ImGui::Columns(1);
	ImGui::Separator();
}

void TestMode::updateInspector() {
	auto& engine = parent.getContext().game->engine;
	
	if (scene == 0u || tile_pos == sf::Vector2f{-1.f, -1.f}) {
		ImGui::Text("Select cell position with left mouse click");
		return;
	}
	auto pos_dump = thor::toString(tile_pos);
	
	if (cell_entities.empty()) {
		ImGui::Text("No entities found on %s", pos_dump.c_str());
		return;
	}
	
	ImGui::Text("Entities at %s", pos_dump.c_str());
	if (ui::Combo("ID", cell_entities_index, cell_entities)) {
		// parse id
		auto item = cell_entities[cell_entities_index];
		auto pos = item.find(" ");
		ASSERT(pos != std::string::npos);
		object = std::stoul(item.substr(1, pos-1));
		ASSERT(object > 0u);
		
		onSelectObject();
	}
	if (object == 0u) {
		return;
	}
	
	if (engine.session.stats.has(object)) {
		auto& data = engine.session.stats.query(object);
		if (engine.session.action.query(object).dead) {
			if (ImGui::Button("Spawn")) {
				rpg::SpawnEvent event;
				event.actor = object;
				event.respawn = true;
				engine.factory.handle(event);
				data.stats[rpg::Stat::Life] = data.properties[rpg::Property::MaxLife];
			}
		} else {
			if (ImGui::Button("Kill")) {
				rpg::StatsEvent event;
				event.actor = object;
				event.delta[rpg::Stat::Life] = -data.stats[rpg::Stat::Life];
				engine.avatar.handle(event);
			}
		}
	}
	
	if (ImGui::Button("Remove object")) {
		// allow removing only if not focusable!
		if (engine.session.stats.has(object) && !engine.session.action.query(object).dead) {
			auto& data = engine.session.stats.query(object);
			// first kill it!
			rpg::StatsEvent event;
			event.actor = object;
			event.delta[rpg::Stat::Life] = -data.stats[rpg::Stat::Life];
			engine.avatar.handle(event);
		}
		parent.getContext().game->engine.factory.destroyObject(object); 
	}
	
	ImGui::Separator();
	
	// show object inspectors
	for (auto const & pair: inspectors) {
		auto ptr = pair.second.get();
		if (ImGui::CollapsingHeader(pair.first.c_str())) {
			if (!ptr->open) {
				ptr->open = true;
				ptr->refresh();
			}
			ptr->update();
			ImGui::PushItemWidth(-1);
		} else {
			ptr->open = false;
		}
		ImGui::NextColumn();
	}
}

void TestMode::updateSpawner() {
	auto& engine = parent.getContext().game->engine;
	
	ImGui::Text("Mode:");
	bool changed = ImGui::RadioButton("Entity", &spawn_mode, 0);
	changed = changed | ImGui::RadioButton("Bot", &spawn_mode, 1);
	if (changed) {
		onSpawnModeChanged();
	}
	
	if (spawn_mode == 0) {
		// show entity spawn ui
		ui::Combo("Entity", entity_tpls_index, entity_tpls);
	} else if (spawn_mode == 1) {
		// show bot spawn ui
		ui::Combo("Bots", bot_tpls_index, bot_tpls);
		ui::Combo("Script", ai_scripts_index, ai_scripts);
		ImGui::Checkbox("Hostile", &hostile);
		ui::InputNumber("Level", level, (std::size_t)0, (std::size_t)1000);
	}
	
	// show general spawn ui
	ui::Combo("Scene ID", scenes_index, scenes);
	if (scenes_index >= 0) {
		auto& dungeon = engine.dungeon[scenes_index+1u];
		auto size = dungeon.getSize();
		ui::InputNumber("x", target_pos.x, 0.f, size.x-1.f);
		ui::InputNumber("y", target_pos.y, 0.f, size.y-1.f);
		ImGui::Checkbox("Spawn near by", &spawn_near);
		if (ImGui::Button("Spawn")) {
			rpg::SpawnMetaData spawn;
			spawn.scene = dungeon.id;
			spawn.pos = target_pos;
			if (spawn_near) {
				// spawn at near position
				core::getFreePosition([&](sf::Vector2f const & p) {
					if (dungeon.has(sf::Vector2u{p})) {
						auto const & cell = dungeon.getCell(sf::Vector2u{p});
						return cell.terrain == core::Terrain::Floor
							&& cell.entities.empty();
					}
					return false;
				}, spawn.pos, 1000u);
			}
			
			if (spawn_mode == 0 && entity_tpls_index >= 0) {
				auto const & entity = engine.mod.get<rpg::EntityTemplate>(entity_tpls[entity_tpls_index]);
				engine.factory.createObject(entity, spawn);
				
			} else if (spawn_mode == 1 && bot_tpls_index >= 0 && ai_scripts_index >= 0) {
				auto const & bot = engine.mod.get<game::BotTemplate>(bot_tpls[bot_tpls_index]);
				auto const & script = engine.mod.createScript(ai_scripts[ai_scripts_index]);
				engine.factory.createBot(bot, spawn, level, /*script,*/ hostile);
			}
		}
	}
}

void TestMode::updateTeleporter() {
	auto& engine = parent.getContext().game->engine;
	
	if (scene == 0u || tile_pos == sf::Vector2f{-1.f, -1.f}) {
		ImGui::Text("Select cell position with left or right mouse click");
		return;
	}
	auto pos_dump = thor::toString(tile_pos);
	
	if (cell_entities.empty()) {
		ImGui::Text("No entities found on %s", pos_dump.c_str());
		return;
	}
	
	ImGui::Text("Entities at %s", pos_dump.c_str());
	if (ui::Combo("ID", cell_entities_index, cell_entities)) {
		// parse id
		auto item = cell_entities[cell_entities_index];
		auto pos = item.find(" ");
		ASSERT(pos != std::string::npos);
		object = std::stoul(item.substr(1, pos-1));
		ASSERT(object > 0u);
		
		onSelectObject();
	}
	if (object == 0u) {
		return;
	}
	
	// show teleport ui
	ui::Combo("Scene ID", scenes_index, scenes);
	if (scenes_index >= 0) {
		auto& dungeon = engine.dungeon[scenes_index+1];
		auto size = dungeon.getSize();
		ui::InputNumber("x", target_pos.x, 0.f, size.x-1.f);
		ui::InputNumber("y", target_pos.y, 0.f, size.y-1.f);
		if (ImGui::Button("Teleport")) {
			auto& data = engine.session.movement.query(object);
			core::vanish(engine.dungeon[data.scene], data);
			core::spawn(dungeon, data, target_pos);
		}
	}
}

void TestMode::updateSystemLog() {
	bool scroll_debug = ui::forwardStream(debug_stream, debug_log);
	bool scroll_warning = ui::forwardStream(warning_stream, warning_log);
	bool scroll_error = ui::forwardStream(error_stream, error_log);
	
	ImGui::Begin("Debug Log");
		if (debug_log.empty()) {
			ImGui::Text("Empty log");
		} else {
			ImGui::TextUnformatted(debug_log.begin());
			if (scroll_debug) {
				ImGui::SetScrollHere(1.f);
			}
		}
	ImGui::End();
	
	if (!warning_log.empty()) {
		ImGui::Begin("Warning Log");
			ImGui::TextUnformatted(warning_log.begin());
			if (scroll_warning) {
				ImGui::SetScrollHere(1.f);
			}
		ImGui::End();
	}
	
	if (!error_log.empty()) {
		ImGui::Begin("Error Log");
			ImGui::TextUnformatted(error_log.begin());
			if (scroll_error) {
				ImGui::SetScrollHere(1.f);
			}
		ImGui::End();
	}
}

void TestMode::updateEventLog() {
	bool scroll{false};
	for (auto& pair: event_logger) {
		scroll = scroll | ui::forwardStream(pair.second.stream, event_log);
	}
	ImGui::TextUnformatted(event_log.begin());
	if (scroll) {
		ImGui::SetScrollHere(1.f);
	}
}

void TestMode::reloadScripts() {
	auto& context = parent.getContext();
	auto& engine = context.game->engine;
	std::size_t n{0u}, m{0u};
	// reload scripts
	for (auto& script: context.mod.getAllScripts()) {
		auto fname = script.getFilename();
		ASSERT(!fname.empty());
		script.loadFromFile(fname);
		++n;
	}
	// reinit AIs
	/*
	for (auto& data: engine.session.script) {
		auto const & script = *data.script;
		script("onInit", data.api.get());
		++m;
	}
	*/
	context.log.debug << "[State/TestMode] Reloaded " << n
		<< " Scripts for " << m << " AI Components\n";
}

void TestMode::toggleRenderDebug() {
	auto& context = parent.getContext();
	auto& engine = context.game->engine;
	render_debug = !render_debug;
	
	if (render_debug) {
		engine.ui.render.setGridColor(sf::Color::Yellow);
		engine.ui.render.setShowFov(true);
		engine.ui.render.setShowShape(true);
	} else {
		engine.ui.render.setGridColor(sf::Color::Transparent);
		engine.ui.render.setShowFov(false);
		engine.ui.render.setShowShape(false);
	}
}

void TestMode::onLeftClick() {
	// update select pos
	sf::Vector2f mouse_pos{sf::Mouse::getPosition(parent.getApplication().getWindow())};
	auto& context = parent.getContext();
	auto const & engine = context.game->engine;
	auto cam_ptr = engine.getCamera(mouse_pos);
	if (cam_ptr == nullptr) {
		return;
	}
	auto const & dungeon = engine.getDungeon(*cam_ptr);
	ASSERT(dungeon.id > 0u);
	scene = dungeon.id;
	tile_pos = engine.getWorldPos(mouse_pos);
	
	if (tile_pos == sf::Vector2f{-1.f, -1.f}) {
		return;
	} if (target_pos == sf::Vector2f{-1.f, -1.f}) {
		target_pos = tile_pos;
	}
	
	// load entities
	auto const & focus = engine.session.focus;
	// query entities
	cell_entities.clear();
	cell_entities_index = -1;
	if (!dungeon.has(sf::Vector2u{tile_pos})) {
		return;
	}
	auto const & cell = dungeon.getCell(sf::Vector2u{tile_pos});
	object = 0u;
	for (auto id: cell.entities) {
		// create entry
		std::string item = "#" + std::to_string(id) + " ";
		if (focus.has(id)) {
			item += focus.query(id).display_name;
		} else {
			item += "<Unknown>";
		}
		cell_entities.push_back(item);
		// select if first entry
		if (object == 0u) {
			object = id;
			cell_entities_index = 0;
			onSelectObject();
		}
	}
}

void TestMode::onRightClick() {
	// update select pos
	sf::Vector2f mouse_pos{sf::Mouse::getPosition(parent.getApplication().getWindow())};
	auto& context = parent.getContext();
	auto const & engine = context.game->engine;
	auto cam_ptr = engine.getCamera(mouse_pos);
	if (cam_ptr == nullptr) {
		return;
	}
	auto const & dungeon = engine.getDungeon(*cam_ptr);
	ASSERT(dungeon.id > 0u);
	scenes_index = dungeon.id - 1u;
	target_pos = engine.getWorldPos(mouse_pos);
}

void TestMode::onSelectObject() {
	inspectors.clear();
	auto& context = parent.getContext();
	auto& engine = context.game->engine;
	auto& session = engine.session;
	createInspector(inspectors, session.movement, context.log, engine, object);
	createInspector(inspectors, session.focus, context.log, engine, object);
	createInspector(inspectors, session.collision, context.log, engine, object);
	createInspector(inspectors, session.animation, context.log, engine, object);
	createInspector(inspectors, session.render, context.log, engine, object);
	createInspector(inspectors, session.sound, context.log, engine, object);
	createInspector(inspectors, session.stats, context.log, engine, object);
	createInspector(inspectors, session.effect, context.log, engine, object);
	createInspector(inspectors, session.item, context.log, engine, object);
	createInspector(inspectors, session.perk, context.log, engine, object);
	createInspector(inspectors, session.player, context.log, engine, object);
	createInspector(inspectors, session.projectile, context.log, engine, object);
	createInspector(inspectors, session.action, context.log, engine, object);
	createInspector(inspectors, session.input, context.log, engine, object);
	createInspector(inspectors, session.interact, context.log, engine, object);
	createInspector(inspectors, session.quickslot, context.log, engine, object);
	//createInspector(inspectors, session.script, context.log, engine, object);
	
	// note: hud is not inspected here
}

void TestMode::onSpawnModeChanged() {
	auto& engine = parent.getContext().game->engine;
	
	// query all entity templates, bot templates and ai scripts
	entity_tpls.clear();
	entity_tpls_index = -1;
	bot_tpls.clear();
	bot_tpls_index = -1;
	ai_scripts.clear();
	ai_scripts_index = -1;
	engine.mod.getAllFiles<rpg::EntityTemplate>(entity_tpls);
	engine.mod.getAllFiles<game::BotTemplate>(bot_tpls);
	engine.mod.getAllFiles<game::AiScript>(ai_scripts);
}

void TestMode::tryLoadScenes() {
	if (scenes.empty()) {
		// initialize scenes
		auto const & engine = parent.getContext().game->engine;
		for (auto const & ptr: engine.dungeon) {
			scenes.push_back("#" + std::to_string(ptr->id));
		}
		scenes_index = -1;
	}
}

void TestMode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	ImGui::Render();
}

bool TestMode::handle(sf::Event const& event) {
	ImGui::SFML::ProcessEvent(event);
	
	ImGuiIO& io = ImGui::GetIO();
	bool handled = io.WantCaptureKeyboard;
	
	switch (event.type) {
		case sf::Event::KeyPressed:
			// handle shortcuts
			switch (event.key.code) {
				case sf::Keyboard::F1:
					show_monitor = !show_monitor;
					break;
					
				case sf::Keyboard::F2:
					show_inspector = !show_inspector;
					break;
					
				case sf::Keyboard::F3:
					show_spawner = !show_spawner;
					break;
					
				case sf::Keyboard::F4:
					show_teleporter = !show_teleporter;
					break;
					
				case sf::Keyboard::F5:
					show_log = !show_log;
					break;
					
				case sf::Keyboard::F6:
					show_event = !show_event;
					break;
					
				case sf::Keyboard::F7:
					reloadScripts();
					break;
					
				case sf::Keyboard::F8:
					toggleRenderDebug();
					break;
					
				default:
					handled = false;
					break;
			}
			break;
			
		case sf::Event::MouseButtonPressed:
			if (io.WantCaptureMouse) {
				return true;
			}
			switch (event.mouseButton.button) {
				case sf::Mouse::Button::Left:
					onLeftClick();
					break;
					
				case sf::Mouse::Button::Right:
					onRightClick();
					break;
					
				default:
					handled = false;
					break;
			}
			break;
			
		default:
			handled = false;
			break;
	}
	
	return handled;
}

void TestMode::update(sf::Time const& elapsed) {
	auto& window = parent.getApplication().getWindow();
	
	ImGui::SFML::Update(window, elapsed);
	
	event_logger.update();
	
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Engine")) {
			if (ImGui::MenuItem((freeze ? "Resume" : "Freeze"))) {
				freeze = !freeze;
				parent.onSetFreeze(freeze);
			}
			if (ImGui::MenuItem("Pause")) {
				parent.onPause();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Shutdown")) {
				parent.quit();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem("System Monitor", "F1", show_monitor)) {
				show_monitor = !show_monitor;
			}
			if (ImGui::MenuItem("Object Inspector", "F2", show_inspector)) {
				show_inspector = !show_inspector;
			}
			if (ImGui::MenuItem("Object Spawner", "F3", show_spawner)) {
				show_spawner = !show_spawner;
			}
			if (ImGui::MenuItem("Object Teleport", "F4", show_teleporter)) {
				show_teleporter = !show_teleporter;
			}
			if (ImGui::MenuItem("System Log", "F5", show_log)) {
				show_log = !show_log;
			}
			if (ImGui::MenuItem("Event Log", "F6", show_event)) {
				show_event = !show_event;
			}
			if (ImGui::MenuItem("Reload all scripts", "F7")) {
				reloadScripts();
			}
			if (ImGui::MenuItem("Toggle Debug Rendering", "F8")) {
				toggleRenderDebug();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Hide all", nullptr, false, show_monitor
				|| show_inspector || show_spawner || show_teleporter
				|| show_log || show_event)) {
				show_monitor = false;
				show_inspector = false;
				show_spawner = false;
				show_teleporter = false;
				show_log = false;
				show_event = false;
			}
			if (ImGui::MenuItem("Clear logs")) {
				debug_log.clear();
				warning_log.clear();
				error_log.clear();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	
	if (show_monitor) {
		ImGui::Begin("System Monitor");
		updateMonitor();
		ImGui::End();
	}
	if (show_inspector) {
		ImGui::Begin("Object Inspector");
		updateInspector();
		ImGui::End();
	}
	if (show_spawner) {
		tryLoadScenes();
		ImGui::Begin("Object Spawner");
		updateSpawner();
		ImGui::End();
	}
	if (show_teleporter) {
		tryLoadScenes();
		ImGui::Begin("Object Teleporter");
		updateTeleporter();
		ImGui::End();
	}
	if (show_log) {
		updateSystemLog();
	}
	if (show_event) {
		ImGui::Begin("Event Log");
		updateEventLog();
		ImGui::End();
	}
}

} // ::tool
