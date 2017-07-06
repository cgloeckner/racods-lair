#include <limits.h>
#include <boost/algorithm/string.hpp>

#include <utils/algorithm.hpp>
#include <core/algorithm.hpp>
#include <core/teleport.hpp>
#include <game/mod.hpp>
#include <state/tool/roomeditor.hpp>

namespace tool {

EngineState::EngineState(state::App& app, std::string const & mod_name)
	: app{app}
	, cache{}
	, mod{app.getContext().log, cache, mod_name}
	, engine{app.getContext().log, app.getContext().globals.max_num_objects,
		app.getWindow().getSize(), 1.f, 16u, mod, cache,
		app.getContext().locale}
	, current_room{}
	, current_name{""}
	, changed{false}
	, scene{0u}
	, viewer{0u}
	, empty_tex{}
	, sprite{} {
	engine.ui.render.setCastShadows(false);
	
	engine.generator.settings = app.getContext().globals.dungeon_gen;
	engine.generator.settings.room_density = 1.f;
	engine.generator.settings.deadend_density = 0.f;
}

rpg::TilesetTemplate const & EngineState::getTileset() {
	return *mod.getAll<rpg::TilesetTemplate>().front();
}

void EngineState::newRoom(std::string const & room_name) {
	// prepare new room file
	game::RoomTemplate empty;
	auto path = mod.get_filename<game::RoomTemplate>(room_name);
	empty.saveToFile(path);
	
	// load that room
	loadRoom(room_name);
}

void EngineState::loadRoom(std::string const & room_name) {
	auto& context = app.getContext();
	
	current_room = mod.get<game::RoomTemplate>(room_name, true);
	current_name = room_name;
	mod.prepare(current_room);
	
	rebuild();
	
	changed = false;
	
	context.log.debug << "[State/RoomEditor] " << "Loaded '" << current_name << "'\n";
}

void EngineState::saveRoom() {
	ASSERT(!current_name.empty());
	auto& context = app.getContext();
	
	auto path = mod.get_filename<game::RoomTemplate>(current_name);
	current_room.saveToFile(path);
	changed = false;
	
	context.log.debug << "[State/RoomEditor] " << "Saved '" << current_name << "'\n";
}

void EngineState::rebuild() {
	auto& context = app.getContext();
	
	// detect previous zoom and center
	float zoom{1.f};
	boost::optional<sf::Vector2u> center{boost::none};
	auto first_cam = engine.ui.camera.begin();
	if (first_cam != engine.ui.camera.end()) {
		auto const & cam = *first_cam->get();
		zoom = cam.zoom;
		center = engine.physics.movement.query(viewer).target;
	}
	
	// reset engine state
	engine.clear();
	
	// modify copy of tileset to use only one kind of floor/wall
	auto tileset = getTileset();
	tileset.floors.resize(1u);
	tileset.walls.resize(1u);
	
	// create dungeon
	engine.generator.rooms.clear();
	engine.generator.rooms.push_back(&current_room);
	thor::setRandomSeed(0u); // note: make generation predictable
	game::BuildSettings settings;
	settings.cell_size = context.globals.dungeon_gen.cell_size;
	settings.random_transform = false;
	settings.editor_mode = true;
	scene = engine.factory.createDungeon(tileset,
		{settings.cell_size, settings.cell_size}, settings,
		[&](game::DungeonBuilder& builder) {
			// note: corridor is centered with respect to its width!
			auto const pad = settings.path_width / 2;
			auto const step = settings.cell_size / 2u;
			auto const stop = settings.cell_size - (settings.path_width - pad);
			// add stub horizontal corridor
			builder.paths.emplace_back(pad, step, stop, step);
			// add stub vertical corridor
			builder.paths.emplace_back(step, pad, step, stop);
	});
	
	// prepare invisible viewer entity
	rpg::EntityTemplate entity;
	entity.sprite = &sprite;
	sprite.frameset = &empty_tex;
	for (auto& pair: sprite.torso) {
		pair.second.frames.resize(1u);
		pair.second.frames[0].duration = sf::seconds(1.f);
		pair.second.duration = sf::seconds(1.f);
	}
	
	// create viewer and camera
	rpg::SpawnMetaData spawn;
	spawn.scene = scene;
	if (center == boost::none) {
		// spawn camera at center
		spawn.pos = engine.session.dungeon[scene].getSize() / 2u;
	} else {
		// reuse previous position
		spawn.pos = *center;
	}
	viewer = engine.factory.createObject(entity, spawn);
	auto& cam = engine.session.camera.acquire();
	cam.objects.push_back(viewer);
	
	if (zoom != 1.f) {
		// reapply zoom
		cam.zoom = zoom;
		cam.scene.zoom(zoom);
	}
	
	// create mouse-attached entity
	entity.light = std::make_unique<utils::Light>();
	entity.light->radius = game::PLAYER_LIGHT_RADIUS;
	entity.light->color = game::PLAYER_LIGHT_COLOR;
	entity.light->intensity = game::PLAYER_LIGHT_INTENSITY;
	spawn.pos = {0u, 0u};
	mouse = engine.factory.createObject(entity, spawn);
	updateMouseLight();
}

void EngineState::draw(sf::Vector2u const & pen, CellHandler handle) {
	ASSERT(scene > 0u);
	auto tile_pos = sf::Vector2u{getWorldPos()};
	sf::Vector2u delta;
	auto& dungeon = engine.dungeon[scene];
	auto const size = dungeon.getSize();
	
	// update room template
	for (delta.y = 0; delta.y < pen.y; ++delta.y) {
		for (delta.x = 0; delta.x < pen.x; ++delta.x) {
			auto tmp = tile_pos + delta;
			if (tmp.x == 0u || tmp.y == 0u || tmp.x >= size.x - 1u || tmp.y >= size.y - 1u) {
				// is border or outside room
				continue;
			}
			handle(tmp);
		}
	}
	
	// rebuild entire dungeon
	rebuild();
	
	// check whether room changed compared to its file content
	changed = true;
	if (!current_name.empty()) {
		auto& prev_room = mod.query<game::RoomTemplate>(current_name, true);
		mod.prepare(prev_room);
		changed = current_room != prev_room;
	}
}

void EngineState::setTerrain(sf::Vector2u const & pen, core::Terrain terrain) {
	draw(pen, [&](sf::Vector2u const & pos) {
		bool has_cell = current_room.cells.find(pos) != current_room.cells.end();
		switch (terrain) {
			case core::Terrain::Void:
				if (has_cell) {
					current_room.destroy(pos);
				}
				break;
				
			case core::Terrain::Wall:
			case core::Terrain::Floor:
				if (!has_cell) {
					current_room.create(pos);
				}
				current_room.cells[pos].wall = terrain == core::Terrain::Wall;
				break;
		}
	});
}

void EngineState::setEntity(sf::Vector2u const & pen, std::string const & name,
	sf::Vector2i const & direction) {
	draw(pen, [&](sf::Vector2u const & pos) {
		bool has_cell = current_room.cells.find(pos) != current_room.cells.end();
		if (name.empty()) {
			if (has_cell) {
				auto& cell = current_room.cells[pos];
				// reset entity
				cell.entity = game::RoomTemplate::EntityNode{};
				if (cell.wall) {
					// remove entire cell
					current_room.destroy(pos);
				}
			}
		} else {
			if (!has_cell) {
				// create wall tile
				current_room.create(pos);
				current_room.cells[pos].wall = true;
			}
			auto& cell = current_room.cells[pos];
			cell.entity.name = name;
			cell.entity.direction = direction;
			cell.entity.ptr = &engine.mod.query<rpg::EntityTemplate>(name);
		}
	});
}

void EngineState::setLighting(bool lighting) {
	if (lighting) {
		engine.ui.lighting.setLevelOfDetails(std::numeric_limits<std::size_t>::max());
	} else {
		engine.ui.lighting.setLevelOfDetails(0u);
	}
}

void EngineState::setShowGrid(bool show) {
	if (show) {
		engine.ui.render.setGridColor({255u, 255u, 0u, 32u});
	} else {
		engine.ui.render.setGridColor(sf::Color::Transparent);
	}
}

void EngineState::updateMouseLight() {
	auto world_pos = getWorldPos();
	auto tile_pos = sf::Vector2u{world_pos};
	auto& dungeon = engine.dungeon[scene];
	if (dungeon.has(tile_pos)) {
		auto& move = engine.session.movement.query(mouse);
		if (move.target != tile_pos) {
			core::vanish(dungeon, move);
			core::spawn(dungeon, move, tile_pos);
		}
		move.pos = world_pos;
	}
}

sf::Vector2f EngineState::getWorldPos() const {
	return engine.getWorldPos(sf::Vector2f{sf::Mouse::getPosition(app.getWindow())});
}

void EngineState::scroll(sf::Vector2i const & delta) {
	auto& context = app.getContext();
	
	if (scene == 0u) {
		return;
	}
	
	auto& cam = **engine.ui.camera.begin();
	auto id = cam.objects.front();
	auto& m = engine.session.movement.query(id);
	auto& d = engine.dungeon[scene];
	auto pos = sf::Vector2u{sf::Vector2i{m.target} + delta};
	if (!d.has(pos)) {
		context.log.debug << "[State/RoomEditor] "
			<< "Cannot scroll to " << pos << "\n";
		return;
	}
	
	core::vanish(d, m);
	core::spawn(d, m, pos);
}

// --------------------------------------------------------------------

RoomEditorState::RoomEditorState(state::App& app)
	: state::State{app}
	, engine{nullptr}
	, next_popup{""}
	, new_modpath{"./data"}
	, new_filename{}
	, load_modpath{"./data"}
	, load_filename_index{-1}
	, entity_filename_index{-1}
	, load_filename{}
	, entity_filename{}
	, edit_mode{0}
	, edit_pen{1, 1}
	, entity_direction{0, 1}
	, pen{}
	, last_pos{boost::none}
	, lighting{false}
	, show_grid{true} {
	onLoadMod(load_modpath);
	
	pen.setFillColor({255u, 255u, 0u, 32u});
}

bool RoomEditorState::hasMap() const {
	return engine != nullptr && !engine->current_name.empty();
}

void RoomEditorState::onLoadMod(std::string const & mod_name) {
	if (!utils::file_exists(mod_name)) {
		// mod doesn't exist
		return;
	}
	
	if (engine == nullptr || engine->mod.name != mod_name) {
		// create new engine
		engine = std::make_unique<EngineState>(getApplication(), mod_name);
	}
	
	// query all room and entity files
	try {
		engine->mod.getAllFiles<game::RoomTemplate>(load_filename);
		engine->mod.getAllFiles<rpg::EntityTemplate>(entity_filename);
		utils::pop(entity_filename, std::string{"stairs"});
		std::sort(load_filename.begin(), load_filename.end());
		std::sort(entity_filename.begin(), entity_filename.end());
	} catch (...) {
		engine = nullptr;
	}
}

void RoomEditorState::onPenResize() {
	if (engine == nullptr || engine->scene == 0u) {
		return;
	}
	
	// resize pen using tilesize
	auto const & tileset = engine->getTileset();
	auto size = tileset.tilesize;
	if (edit_mode == 0) {
		size.x *= edit_pen[0];
		size.y *= edit_pen[1];
	}
	pen.setSize(sf::Vector2f{size});
}

void RoomEditorState::onPenMove() {
	if (engine == nullptr || engine->scene == 0u) {
		return;
	}
	
	// move pen to mouse position
	sf::Vector2f pos{sf::Mouse::getPosition(getApplication().getWindow())};
	engine->engine.snapGrid(pos);
	pen.setPosition(pos);
	
	// move light to mouse position
	engine->updateMouseLight();
}

void RoomEditorState::onModeSet() {
	onPenResize();
}

void RoomEditorState::onEntitySelect() {
	ASSERT(engine != nullptr);
	
	if (entity_filename_index >= 0) {
		auto& res = engine->engine.mod.query<rpg::EntityTemplate>(entity_filename[entity_filename_index]);
		engine->engine.mod.prepare(res);
	}
}

void RoomEditorState::onNewClick() {
	auto& context = getContext();
	
	if (new_modpath.empty() || new_filename.empty()) {
		context.log.error << "[State/RoomEditor] "
			<< "Need both: modpath and filename\n";
		return;
	}
	
	onLoadMod(new_modpath);
	if (engine == nullptr) {
		context.log.error << "[State/RoomEditor] "
			<< "Failed to load mod '" << new_modpath << "'\n";
		return;
	}
	
	engine->newRoom(new_filename);
	engine->setLighting(lighting);
	engine->setShowGrid(show_grid);
	onPenResize();
	
	// reload mod to refresh room filename list
	onLoadMod(new_modpath);
}

void RoomEditorState::onLoadClick() {
	ASSERT(engine != nullptr);
	if (load_filename_index == -1) {
		return;
	}
	
	engine->loadRoom(load_filename[load_filename_index]);
	engine->setLighting(lighting);
	engine->setShowGrid(show_grid);
	onPenResize();
}

void RoomEditorState::onSaveClick() {
	ASSERT(engine != nullptr);
	
	engine->saveRoom();
}

void RoomEditorState::onQuitClick() {
	quit();
}

void RoomEditorState::onMouseClick(sf::Mouse::Button button) {
	switch (button) {
		case sf::Mouse::Button::Left:
			switch (edit_mode) {
				case 0:
					onPlaceTileClick();
					break;
					
				case 1:
					onPlaceEntityClick();
					break;
					
				case 2:
					break;
					
				default:
					break;
			}
			break;
			
		case sf::Mouse::Button::Right:
			switch (edit_mode) {
				case 0:
					onRemoveTileClick();
					break;
					
				case 1:
					onRemoveEntityClick();
					break;
					
				case 2:
					//
					break;
					
				default:
				break;
			}
			break;
		
		default:
			break;
	}
}

void RoomEditorState::onPlaceTileClick() {
	if (hasMap()) {
		engine->setTerrain(sf::Vector2u(edit_pen[0], edit_pen[1]), core::Terrain::Floor);
	}
}

void RoomEditorState::onRemoveTileClick() {
	if (hasMap()) {
		engine->setTerrain(sf::Vector2u(edit_pen[0], edit_pen[1]), core::Terrain::Void);
	}
}

void RoomEditorState::onPlaceEntityClick() {
	if (hasMap() && entity_filename_index >= 0) {
		engine->setEntity({1, 1}, entity_filename[entity_filename_index],
			{entity_direction[0], entity_direction[1]});
	}
}

void RoomEditorState::onRemoveEntityClick() {
	if (hasMap()) {
		engine->setEntity({1, 1}, "", {0, 1});
	}
}

void RoomEditorState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (hasMap()) {
		auto view = target.getView();
		
		engine->engine.ui.render.cull();
		target.draw(engine->engine.ui.render, states);
		
		auto& cam = **engine->engine.ui.camera.begin();
		target.setView(cam.scene);
		target.draw(pen);
		
		target.setView(view);
	}
	
	ImGui::Render();
}

void RoomEditorState::handle(sf::Event const & event) {
	ImGui::SFML::ProcessEvent(event);
	ImGuiIO& io = ImGui::GetIO();
	bool handle_mouse = !io.WantCaptureMouse;
	bool handle_keyboard = !io.WantCaptureKeyboard;
	
	if (engine != nullptr) {
		engine->engine.ui.handle(event);
	}
	
	switch (event.type) {
		case sf::Event::Closed:
			onQuitClick();
			break;
		
		case sf::Event::Resized:
			State::onResize({event.size.width, event.size.height});
			break;
		
		case sf::Event::KeyPressed:
			if (handle_keyboard) {
				// handle camera movement and menu shortcuts
				switch (event.key.code) {
					case sf::Keyboard::W:
					case sf::Keyboard::Up:
						if (hasMap()) {
							engine->scroll({0, -1});
						}
						break;
					
					case sf::Keyboard::A:
					case sf::Keyboard::Left:
						if (hasMap()) {
							engine->scroll({-1, 0});
						}
						break;
					
					case sf::Keyboard::S:
						if (event.key.control) {
							onSaveClick();
							break;
						} // else:
					case sf::Keyboard::Down:
						if (hasMap()) {
							engine->scroll({0, 1});
						}
						break;
					
					case sf::Keyboard::D:
					case sf::Keyboard::Right:
						if (hasMap()) {
							engine->scroll({1, 0});
						}
						break;
					
					case sf::Keyboard::N:
						if (event.key.control) {
							next_popup = "new";
						}
						break;
					
					case sf::Keyboard::O:
						if (event.key.control) {
							next_popup = "load";
						}
						break;
					
					case sf::Keyboard::Q:
						if (event.key.control) {
							next_popup = "quit";
						}
						break;
						
					case sf::Keyboard::L:
						if (event.key.control && hasMap()) {
							lighting = !lighting;
							engine->setLighting(lighting);
						}
						break;
						
					case sf::Keyboard::G:
						if (event.key.control) {
							show_grid = !show_grid;
							engine->setShowGrid(show_grid);
						}
						break;
						
					case sf::Keyboard::F1:
						// switch to tile mode
						edit_mode = 0;
						onModeSet();
						break;
					
					case sf::Keyboard::F2:
						// switch to entity mode
						edit_mode = 1;
						onModeSet();
						break;
						
					default:
						break;
				}
			}
			
			break;
		
		case sf::Event::MouseWheelMoved:
			if (event.mouseWheel.delta > 0) {
				auto const max = getContext().globals.dungeon_gen.cell_size / 2;
				for (auto i = 0u; i < 2; ++i) {
					if (edit_pen[i] < max) {
						++edit_pen[i];
					}
				}
			} else if (event.mouseWheel.delta < 0) {
				for (auto i = 0u; i < 2; ++i) {
					if (edit_pen[i] > 1) {
						--edit_pen[i];
					}
				}
			}
			if (event.mouseWheel.delta != 0) {
				onPenResize();
			}
			break;
		
		case sf::Event::MouseMoved:
			onPenMove();
			if (hasMap()) {
				if (last_pos != boost::none) {
					auto current = sf::Vector2u{engine->getWorldPos()};
					if (last_pos != current) {
						last_pos = current;
						for (int i = 0u; i < sf::Mouse::Button::ButtonCount; ++i) {
							auto btn = static_cast<sf::Mouse::Button>(i);
							if (sf::Mouse::isButtonPressed(btn)) {
								onMouseClick(btn);
							}
						}
					}
				}
			}
			break;
		
		case sf::Event::MouseButtonPressed:
			if (handle_mouse) {
				if (hasMap()) {
					last_pos = sf::Vector2u{engine->getWorldPos()};
				}
				onMouseClick(event.mouseButton.button);
			}
			break;
		
		case sf::Event::MouseButtonReleased:
			last_pos = boost::none;
			break;
		
		default:
			break;
	}
}

void RoomEditorState::update(sf::Time const & elapsed) {
	ImGui::SFML::Update();
	
	auto const & context = getContext();
	
	// setup menu bar
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New room", "CTRL+N")) {
				next_popup = "new";
			}
			if (ImGui::MenuItem("Open room", "CTRL+O")) {
				next_popup = "load";
			}
			if (ImGui::MenuItem("Save room", "CTRL+S", false, (engine != nullptr) && (engine->changed))) {
				onSaveClick();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "CTRL+Q")) {
				next_popup = "quit";
			}
			
			ImGui::EndMenu();
		}
		if (hasMap()) {
			if (ImGui::BeginMenu("Mode")) {
				if (ImGui::MenuItem("Tile", "F1")) {
					edit_mode = 0;
					onModeSet();
				}
				if (ImGui::MenuItem("Entity", "F2")) {
					edit_mode = 1;
					onModeSet();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Settings")) {
				if (ImGui::MenuItem((lighting ? "Disable lighting" : "Enable lighting"), "CTRL+L")) {
					lighting = !lighting;
					engine->setLighting(lighting);
				}
				if (ImGui::MenuItem((show_grid ? "Hide grid" : "Show grid"), "CTRL+G")) {
					show_grid = !show_grid;
					engine->setShowGrid(show_grid);
				}
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}
	
	// setup popups
	if (!next_popup.empty()) {
		ImGui::SetNextWindowPosCenter();
		ImGui::OpenPopup(next_popup.c_str());
		next_popup.clear();
	}
	
	// setup popup: new file dialog
	if (ImGui::BeginPopup("new")) {
		ImGui::TextWrapped("In order to create a new room, please select"
			" the target mod directory and specify the room's filename.");
		if (engine != nullptr && engine->changed) {
			ImGui::TextColored(ImVec4(sf::Color::Red), "Warning: Unsaved changes will be lost!");
		}
		ui::InputText("Mod path", new_modpath);
		ui::InputText("Room file", new_filename);
		if (ImGui::Button("Create")) {
			onNewClick();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	
	if (ImGui::BeginPopup("load")) {
		ImGui::TextWrapped("In order to load an existing room, please "
			" select the mod's directory and select a room.");
		if (engine != nullptr && engine->changed) {
			ImGui::TextColored(ImVec4(sf::Color::Red), "Warning: Unsaved changes will be lost!");
		}
		if (ui::InputText("Mod path", load_modpath)) {
			onLoadMod(load_modpath);
		}
		ui::Combo("Room file", load_filename_index, load_filename);
		if (ImGui::Button("Load")) {
			onLoadClick();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	
	if (ImGui::BeginPopup("quit")) {
		ImGui::TextWrapped("Do you really want to quit the editor?");
		if (engine != nullptr && engine->changed) {
			ImGui::TextColored(ImVec4(sf::Color::Red), "Warning: Unsaved changes will be lost!");
		}
		if (ImGui::Button("Quit")) {
			onQuitClick();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stay")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	
	if (hasMap()) {
		ImGui::Begin("Editor");
			ImGui::Text("Mod: %s", engine->mod.name.c_str());
			ImGui::Text("Room: %s", engine->current_name.c_str());
			if (engine->changed) {
				ImGui::TextColored(ImVec4(sf::Color::Red), "Unsaved changes!");
			} else {
				ImGui::TextColored(ImVec4(sf::Color::Green), "All changes are saved");
			}
			if (engine->scene > 0u) {
				auto tile_pos = sf::Vector2u{engine->getWorldPos()};
				if (engine->engine.dungeon[engine->scene].has(tile_pos)) {
					ImGui::TextWrapped("Cell %s", thor::toString(tile_pos).c_str());
				} else {
					ImGui::TextWrapped("Outside");
				}
			}
			
			ImGui::Text("Mode:");
			auto set_mode = ImGui::RadioButton("Tile", &edit_mode, 0);
			set_mode = set_mode | ImGui::RadioButton("Entity", &edit_mode, 1);
			if (set_mode) {
				onModeSet();
			}
			
			if (ImGui::Checkbox("Lighting", &lighting)) {
				engine->setLighting(lighting);
			}
			if (ImGui::Checkbox("Show grid", &show_grid)) {
				engine->setShowGrid(show_grid);
			}
			
			if (edit_mode == 0) {
				if (ImGui::SliderInt2("Pen size", edit_pen, 1, context.globals.dungeon_gen.cell_size / 2)) {
					onPenResize();
				}
			} else if (edit_mode == 1) {
				if (ui::Combo("Entity", entity_filename_index, entity_filename)) {
					onEntitySelect();
				}
				if (ImGui::SliderInt2("Direction", entity_direction, -1, 1)) {
					if (entity_direction[0] == 0 && entity_direction[1] == 0) {
						entity_direction[1] = 1;
					}
				}
			}
		ImGui::End();
		
		engine->engine.factory.update(elapsed);
		engine->engine.ui.visuals.update(elapsed);
		engine->engine.ui.animation.update(elapsed);
		engine->engine.ui.render.update(elapsed);
	}
}

} // ::tool
