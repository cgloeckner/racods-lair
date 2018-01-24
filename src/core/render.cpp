#include <stdexcept>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

#include <core/render.hpp>

namespace core {

RenderSystem::RenderSystem(LogContext& log, std::size_t max_objects,
	AnimationManager const& animation_manager,
	MovementManager const& movement_manager, FocusManager const& focus_manager,
	DungeonSystem& dungeon_system, CameraSystem& camera_system,
	utils::LightingSystem& lighting_system)
	: sf::Drawable{}					   // Event API
	, utils::EventListener<SpriteEvent>{}  // Component API
	, RenderManager{max_objects}
	, context{log, *this, animation_manager, movement_manager, focus_manager,
		dungeon_system, camera_system, lighting_system} {}

void RenderSystem::draw(
	sf::RenderTarget& target, sf::RenderStates states) const {
	render_impl::drawScenes(context, target);
}

void RenderSystem::setCastShadows(bool flag) {
	context.cast_shadows = flag;
}

void RenderSystem::setGridColor(sf::Color color) {
	context.grid_color = color;
}

void RenderSystem::setShowFov(bool show) {
	context.show_fov = show;
}

void RenderSystem::handle(SpriteEvent const& event) {
	auto& data = query(event.actor);
	switch (event.type) {
		case SpriteEvent::Legs:
			render_impl::updateTexture(
				context, data, event.leg_layer, event.texture);
			break;

		case SpriteEvent::Torso:
			render_impl::updateTexture(
				context, data, event.torso_layer, event.texture);
			break;
	}
}

void RenderSystem::update(sf::Time const& elapsed) {
	dispatch<SpriteEvent>(*this);

	// component update is done on demand (while culling) to avoid processing
	// all dirtyflags each time

	render_impl::updateCameras(context, elapsed);
}

void RenderSystem::cull() { render_impl::cullScenes(context); }

// ---------------------------------------------------------------------------

namespace render_impl {

CullingBuffer::CullingBuffer()
	: terrain{}
	, objects{}
	, ambiences{}
	, edges{}
	, lights{}
	, highlights{}
	, grid{sf::Lines} {
	for (auto& pair : terrain) {
		pair.second.setPrimitiveType(sf::Triangles);
	}
}

Context::Context(LogContext& log, RenderManager& render_manager,
	AnimationManager const& animation_manager,
	MovementManager const& movement_manager, FocusManager const& focus_manager,
	DungeonSystem& dungeon_system,
	CameraSystem& camera_system, utils::LightingSystem& lighting_system)
	: log{log}
	, render_manager{render_manager}
	, animation_manager{animation_manager}
	, movement_manager{movement_manager}
	, focus_manager{focus_manager}
	, dungeon_system{dungeon_system}
	, camera_system{camera_system}
	, lighting_system{lighting_system}
	, buffers{}
	, grid_color{sf::Color::Transparent}
	, show_fov{false}
	, cast_shadows{true}
	, sprite_shader{} {
	// setup sprite shader
	sprite_shader.loadFromMemory(
		"uniform float brightness, min_saturation, max_saturation;"
		"uniform sampler2D texture;"
		"void main() {"
		"	vec4 pixel = gl_Color * texture2D(texture, gl_TexCoord[0].xy);"
		"	pixel = vec4(clamp(pixel.rgb, min_saturation, max_saturation), "
		"pixel.a);"
		"	gl_FragColor = vec4(pixel.rgb * brightness, pixel.a);"
		"}",
		sf::Shader::Fragment);
}

// ---------------------------------------------------------------------------

float getRotation(sf::Vector2i const& dir) {
	if (dir == sf::Vector2i{0, -1}) {
		return 180.f;
	} else if (dir == sf::Vector2i{1, -1}) {
		return -135.f;
	} else if (dir == sf::Vector2i{1, 0}) {
		return -90.f;
	} else if (dir == sf::Vector2i{1, 1}) {
		return -45.f;
	} else if (dir == sf::Vector2i{0, 1}) {
		return 0.f;
	} else if (dir == sf::Vector2i{-1, 1}) {
		return 45.f;
	} else if (dir == sf::Vector2i{-1, 0}) {
		return 90.f;
	} else if (dir == sf::Vector2i{-1, -1}) {
		return 135.f;
	}
	throw std::out_of_range{"Invalid direction vector <" +
							std::to_string(dir.x) + "," +
							std::to_string(dir.y) + ">"};
}

void updateTexture(Context& context, RenderData& data, SpriteLegLayer layer,
	sf::Texture const* texture) {
	if (texture == nullptr) {
		data.legs[layer].setTextureRect({});
	} else {
		data.legs[layer].setTexture(*texture);
	}
}

void updateTexture(Context& context, RenderData& data, SpriteTorsoLayer layer,
	sf::Texture const* texture) {
	if (texture == nullptr) {
		data.torso[layer].setTextureRect({});
	} else {
		data.torso[layer].setTexture(*texture);
	}
}

void applyFrame(utils::Frame const& frame, sf::Sprite& sprite) {
	sprite.setTextureRect(frame.clip);
	sprite.setOrigin(frame.origin);
}

void applyAnimation(AnimationData const& ani_data, RenderData& data) {
	auto alpha = static_cast<sf::Uint8>(ani_data.alpha.current * 255);

	// update legs
	data.legs.setBrightness(ani_data.brightness.current);
	data.legs.setMinSaturation(ani_data.min_saturation.current);
	data.legs.setMaxSaturation(ani_data.max_saturation.current);
	for (auto& pair : ani_data.tpl.legs) {
		if (pair.second == nullptr) {
			continue;
		}
		auto& frames = pair.second->frames;
		auto& sprite = data.legs[pair.first];
		ASSERT(frames.size() > ani_data.legs.index);
		applyFrame(frames[ani_data.legs.index], sprite);
		thor::setAlpha(sprite, alpha);
	}

	// update torso
	data.torso.setBrightness(ani_data.brightness.current);
	data.torso.setMinSaturation(ani_data.min_saturation.current);
	data.torso.setMaxSaturation(ani_data.max_saturation.current);
	for (auto& pair : ani_data.tpl.torso) {
		if (pair.second == nullptr) {
			continue;
		}
		auto& frames = (*pair.second)[ani_data.current].frames;
		auto& sprite = data.torso[pair.first];
		if (!frames.empty()) {
			ASSERT(frames.size() > ani_data.torso.index);
			applyFrame(frames[ani_data.torso.index], sprite);
		} else {
			// use idle frame
			auto& tmp = (*pair.second)[core::AnimationAction::Idle].frames;
			auto index = std::min(ani_data.torso.index, tmp.size() - 1u);
			applyFrame(tmp[index], sprite);
		}

		thor::setAlpha(sprite, alpha);
	}
}

void updateObject(Context& context, RenderData& data) {
	auto const& move_data = context.movement_manager.query(data.id);
	ASSERT(move_data.scene > 0u);
	auto const& dungeon = context.dungeon_system[move_data.scene];
	// update transformation if necessary
	if (move_data.has_changed) {
		float angle = getRotation(move_data.look);
		auto screen_pos = dungeon.toScreen(move_data.pos);
		// modify transformation matrices
		auto matrix = sf::Transform::Identity;
		matrix.translate(screen_pos);
		matrix.rotate(angle);
		data.legs_matrix = matrix;
		data.torso_matrix = matrix;
		move_data.has_changed = false;
		// update object light
		if (data.light != nullptr) {
			data.light->pos = screen_pos;
		}
		// update highlight sprite
		if (data.highlight != nullptr) {
			data.highlight->setPosition(screen_pos);
		}
		// update fov shape
		// note: fov direction is skipped, because the sprite's transformation
		// matrix (including its rotation) is used
		//data.fov.setDirection(sf::Vector2f{move_data.look});
	}
	// update fov shape if necessary
	if (context.focus_manager.has(data.id)) {
		auto const & focus_data = context.focus_manager.query(data.id);
		if (focus_data.has_changed) {
			focus_data.has_changed = false;
			// update fov shape
			auto radius = focus_data.sight * dungeon.getTileSize().x;
			if (!focus_data.is_active) {
				radius = 0.f;
			}
			data.fov.setOrigin({radius, radius});
			data.fov.setRadius(radius);
			data.fov.setAngle(focus_data.fov);
			data.fov.setPointCount(static_cast<std::size_t>(focus_data.sight * 20));
		}
	}
	// update animation if necessary
	if (context.animation_manager.has(data.id)) {
		auto const& ani_data = context.animation_manager.query(data.id);
		if (ani_data.has_changed) {
			ani_data.has_changed = false;
			applyAnimation(ani_data, data);
			if (data.light != nullptr) {
				// update light settings
				data.light->intensity = static_cast<sf::Uint8>(
					255u * ani_data.light_intensity.current);
				data.light->radius = ani_data.light_radius.current;
			}
		}
	}
}

void updateCameras(Context& context, sf::Time const& elapsed) {
	// guarantee correct number of culling buffers
	if (context.buffers.size() < context.camera_system.size()) {
		context.buffers.resize(context.camera_system.size());
	}
	// update each camera's position
	for (auto& unique_ptr : context.camera_system) {
		auto& camera = *unique_ptr;
		ASSERT(!camera.objects.empty());
		// collect object positions
		std::vector<sf::Vector2f> positions;
		for (auto id : camera.objects) {
			auto const& move_data = context.movement_manager.query(id);
			ASSERT(move_data.scene > 0u);
			auto const& dungeon = context.dungeon_system[move_data.scene];
			positions.push_back(dungeon.toScreen(move_data.pos));
		}
		// update camera system
		context.camera_system.update(camera, elapsed, positions);
	}
}

// ---------------------------------------------------------------------------

void cullAmbiences(CullingBuffer& buffer, DungeonCell const & cell) {
	for (auto const & sprite: cell.ambiences) {
		buffer.ambiences.push_back(&sprite);
	}
}

void addEdges(Context const & context, RenderData const & data, std::vector<utils::Edge>& edges) {
	// determine position
	auto const& move_data = context.movement_manager.query(data.id);
	ASSERT(move_data.scene > 0u);
	auto const& dungeon = context.dungeon_system[move_data.scene];
	auto pos = dungeon.toScreen(move_data.pos);
	
	for (auto cpy: data.edges) {
		cpy.u += pos;
		cpy.v += pos;
		edges.push_back(cpy);
	}
}

void cullScene(Context& context, CullingBuffer& buffer, CameraData const& cam,
	Dungeon& dungeon) {
	// reset state of the buffer
	for (auto& pair : buffer.terrain) {
		pair.second.clear();
	}
	for (auto& pair : buffer.objects) {
		pair.second.clear();
	}
	buffer.ambiences.clear();
	buffer.edges.clear();
	buffer.lights.clear();
	buffer.highlights.clear();
	buffer.grid.clear();

	auto tile_size = dungeon.getTileSize();
	float scale = std::max(tile_size.x, tile_size.y);
	dungeon.setView(cam.scene);

	// cull visible scene
	dungeon.setPadding({1u, 1u});
	for (auto const& pos : dungeon) {
		if (!dungeon.has(pos)) {
			continue;
		}
		auto const& cell = dungeon.getCell(pos);
		if (cell.terrain != core::Terrain::Void) {
			// cull terrain
			cell.tile.fetchTile(buffer.terrain[cell.terrain]);
			// cull ambiences
			cullAmbiences(buffer, cell);
		}
		// cull objects and highlights
		for (auto id : cell.entities) {
			auto& data = context.render_manager.query(id);
			// update renderable's representation if necessary
			updateObject(context, data);
			// save renderable for drawing
			buffer.objects[data.layer].push_back(&data);
			// cull highlighting sprite
			if (data.highlight != nullptr) {
				auto ptr = data.highlight->getTexture();
				// apply lightmap if not yet applied
				if (ptr == nullptr) {
					auto const & lightmap = context.lighting_system.getLightmap();
					auto size = lightmap.getSize();
					data.highlight->setTexture(lightmap);
					data.highlight->setOrigin(sf::Vector2f{size} / 2.f);
				}
				buffer.highlights.push_back(data.highlight.get());
			}
		}
		// cull debug grid
		if (context.grid_color.a != sf::Color::Transparent.a) {
			cell.tile.fetchGrid(context.grid_color, buffer.grid);
		}
	}

	if (context.lighting_system.getLevelOfDetails() > 0u) {
		// set padding to fetch all visible lights and shadows
		dungeon.setPadding({static_cast<unsigned int>(std::ceil(
								utils::MAX_LIGHT_RADIUS / tile_size.x)),
			static_cast<unsigned int>(std::ceil(
				utils::MAX_LIGHT_RADIUS / tile_size.y))});
		// cull lights and edges for shadowcasting
		for (auto const& pos : dungeon) {
			if (!dungeon.has(pos)) {
				continue;
			}
			auto const& cell = dungeon.getCell(pos);
			// cull edges if shadowing enabled
			if (context.cast_shadows) {
				for (auto const& edge : cell.tile.edges) {
					buffer.edges.push_back(edge);
				}
			}
			// cull lights
			for (auto id : cell.entities) {
				auto const& data = context.render_manager.query(id);
				if (data.light != nullptr) {
					// copy and scale light
					utils::Light light = *data.light;
					light.radius *= scale;
					buffer.lights.push_back(std::move(light));
				}
				// cull edges if shadowing enabled
				if (context.cast_shadows && !data.edges.empty()) {
					render_impl::addEdges(context, data, buffer.edges);
				}
			}
		}
		// reset padding
		dungeon.setPadding({0u, 0u});
	}
}

void cullScenes(Context& context) {
	// guarantee correct number of culling buffers
	if (context.buffers.size() < context.camera_system.size()) {
		context.buffers.resize(context.camera_system.size());
	}
	std::size_t i = 0u;
	for (auto& unique_ptr : context.camera_system) {
		auto& camera = *unique_ptr;
		ASSERT(!camera.objects.empty());
		// query one of the focused objects to grab the scene
		auto const& move_data =
			context.movement_manager.query(camera.objects.front());
		ASSERT(move_data.scene > 0u);
		auto& dungeon = context.dungeon_system[move_data.scene];
		// cull scene
		render_impl::cullScene(context, context.buffers[i++], camera, dungeon);
	}
}

// ---------------------------------------------------------------------------

void drawAmbiences(CullingBuffer const & buffer, sf::RenderTarget& target) {
	for (auto ptr: buffer.ambiences) {
		target.draw(*ptr);
	}
}

void drawHighlightings(CullingBuffer const & buffer, sf::RenderTarget& target) {
	for (auto ptr: buffer.highlights) {
		target.draw(*ptr, sf::BlendAdd);
	}
}

void drawSprites(Context const& context, Renderables const& objects,
	sf::RenderTarget& target) {
	for (auto const& ptr : objects) {
		ptr->legs.render(target, ptr->legs_matrix, context.sprite_shader);
		ptr->torso.render(target, ptr->torso_matrix, context.sprite_shader);
	}
}

void drawFovs(Context const& context, Renderables const& objects,
	sf::RenderTarget& target) {
	for (auto const & ptr: objects) {
		target.draw(ptr->fov, ptr->torso_matrix);
	}
}

void drawScene(Context const& context, CullingBuffer& buffer,
	sf::RenderTarget& target, CameraData const& cam, Dungeon& dungeon) {
	// draw floor tiles
	target.setView(cam.scene);
	target.draw(buffer.terrain[Terrain::Floor], &dungeon.tileset);
	// colorize floor with light
	target.setView(cam.screen);
	context.lighting_system.renderLight(target);
	// draw highlighting sprites
	target.setView(cam.scene);
	drawHighlightings(buffer, target);
	// draw ambiences
	target.setView(cam.scene);
	drawAmbiences(buffer, target);
	// hide floor behind obstacles
	target.setView(cam.screen);
	context.lighting_system.renderShadow(target);
	// draw walls and objects
	target.setView(cam.scene);
	target.draw(buffer.terrain[Terrain::Wall], &dungeon.tileset);
	for (auto& pair: buffer.objects) {
		drawSprites(context, pair.second, target);
	}
	// hide far walls and objects with fog
	target.setView(cam.screen);
	context.lighting_system.renderFog(target);
	// draw debug grid
	if (context.grid_color.a != sf::Color::Transparent.a) {
		target.setView(cam.scene);
		target.draw(buffer.grid);
	}
	// draw debug fov
	if (context.show_fov) {
		target.setView(cam.scene);
		for (auto& pair: buffer. objects) {
			drawFovs(context, pair.second, target);
		}
	}
}

void drawScenes(Context const& context, sf::RenderTarget& target) {
	// reset state of lighting system
	context.lighting_system.clear();
	// calculate lighting per camera
	std::size_t i = 0u;
	for (auto& unique_ptr: context.camera_system) {
		auto& buffer = context.buffers[i++];
		auto& camera = *unique_ptr;
		context.lighting_system.update(
			camera.scene, camera.screen, buffer.edges, buffer.lights);
	}
	// draw cameras
	i = 0u;
	for (auto& unique_ptr : context.camera_system) {
		auto& camera = *unique_ptr;
		ASSERT(!camera.objects.empty());
		// query one of the focused objects to grab the scene
		auto const& move_data =
			context.movement_manager.query(camera.objects.front());
		ASSERT(move_data.scene > 0u);
		auto& dungeon = context.dungeon_system[move_data.scene];
		// cull scene
		render_impl::drawScene(
			context, context.buffers[i++], target, camera, dungeon);
	}
}

}  // ::render_impl

}  // ::core
