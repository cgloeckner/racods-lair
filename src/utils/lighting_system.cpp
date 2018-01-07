#include <cmath>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <utils/assert.hpp>
#include <utils/lighting_system.hpp>

namespace utils {

float const MAX_LIGHT_RADIUS = 320.f;

Edge::Edge() : u{}, v{} {}

// ---------------------------------------------------------------------------

Light::Light()
	: pos{}
	, color{sf::Color::White}
	, intensity{255u}
	, radius{1.f}
	, cast_shadow{false}
	, lod{1u} {}

// ---------------------------------------------------------------------------

sf::Vector2f getFarPoint(
	sf::Vector2f const& origin, sf::Vector2f pos, sf::FloatRect const& box) {
	ASSERT(origin != pos);
	// calculate normalized direction
	auto direction = pos - origin;
	auto norm =
		std::sqrt(direction.x * direction.x + direction.y * direction.y);
	direction /= norm;
	// calculate suitable distance
	auto dist = box.width * box.height;
	// create far point
	return pos + dist * direction;
}

// ---------------------------------------------------------------------------

sf::Texture createLightmap(float radius, sf::Shader& shader) {
	float size = radius * 1.5f;

	// prepare buffer
	sf::RenderTexture buffer;
	if (!buffer.create(size, size)) {
		// core::debug << "Cannot render lightmap: Cannot create buffer\n";
		return sf::Texture{};
	}

	// prepare shadeable object
	sf::VertexArray array{sf::Quads, 4u};
	array[0].position = {0.f, 0.f};
	array[1].position = {size, 0.f};
	array[2].position = {size, size};
	array[3].position = {0.f, size};
	for (std::size_t i = 0u; i < 4u; ++i) {
		array[i].texCoords = array[i].position;
		array[i].color = sf::Color::White;
	}

	// render lightmap
	buffer.clear(sf::Color::Black);
	shader.setUniform("radius", radius);
	shader.setUniform("center", sf::Glsl::Vec2{size / 2.f, size / 2.f});
	buffer.draw(array, &shader);
	buffer.display();

	return buffer.getTexture();
}

// ---------------------------------------------------------------------------

LightingSystem::LightingSystem(
	sf::Vector2u const& size, sf::Texture const& lightmap)
	: shadow_buffer{}
	, light_buffer{}
	, fog_buffer{}
	, tmp_buffer{}
	, default_view{{0.f, 0.f, 1.f, 1.f}}
	, shadow{sf::Color::Black}
	, light_sprite{}
	, lod{0u}
	, num_drawn_lights{0u}
	, num_drawn_shadows{0u} {
	resize(size);
	// prepare light sprite
	auto map_size = sf::Vector2f{lightmap.getSize()};
	light_sprite.setTexture(lightmap);
	light_sprite.setOrigin(map_size / 2.f);
}

void LightingSystem::prepareLight(Light const& light) {
	float scale = 3.f * light.radius / MAX_LIGHT_RADIUS;
	auto color = light.color;
	color.a = light.intensity;

	// customize lightmap for this light
	light_sprite.setPosition(light.pos);
	light_sprite.setColor(color);
	light_sprite.setScale({scale, scale});
}

void LightingSystem::setShadowColor(sf::Color const& color) { shadow = color; }

void LightingSystem::setLevelOfDetails(std::size_t lod) { this->lod = lod; }

std::size_t LightingSystem::getLevelOfDetails() const { return lod; }

sf::Texture const & LightingSystem::getLightmap() const {
	return *light_sprite.getTexture();
}

void LightingSystem::resize(sf::Vector2u const& size) {
	// resize default view
	auto window_size = sf::Vector2f{size};
	default_view.setCenter(window_size / 2.f);
	default_view.setSize(window_size);
	// resize offscreen buffers
	shadow_buffer.create(size.x, size.y);
	light_buffer.create(size.x, size.y);
	fog_buffer.create(size.x, size.y);
	tmp_buffer.create(size.x, size.y);
}

void LightingSystem::clear() {
	shadow_buffer.clear(shadow);
	light_buffer.clear(shadow);
	fog_buffer.clear(shadow);
}

void LightingSystem::update(sf::View const& scene, sf::View const& screen,
	std::vector<Edge> const& edges, std::vector<Light> const& lights) {
	// tba: ASSERT buffers to be created

	if (lod == 0u) {
		// lighting is disabled!
		return;
	}

	// prepare rendering
	sf::VertexArray gloom{sf::Quads};
	auto size = scene.getSize();
	sf::FloatRect box{scene.getCenter() - size / 2.f, size};

	num_drawn_shadows = 0u;
	if (!edges.empty()) {
		// create shadowmaps
		shadow_buffer.setView(screen);
		for (auto const& light : lights) {
			if (light.lod > lod) {
				// this shadow is not drawn at this low level of details
				continue;
			}

			// limit shadows to the light source
			tmp_buffer.clear(sf::Color::Black);
			tmp_buffer.setView(scene);
			prepareLight(light);
			float scale = 2.f * light.radius / MAX_LIGHT_RADIUS;  // shorter shadows
			light_sprite.setScale({scale, scale});
			light_sprite.setColor(sf::Color::White);
			tmp_buffer.draw(light_sprite);
			
			if (light.cast_shadow) {
				// cast shadows
				gloom.clear();
				for (auto const& edge : edges) {
					auto p = getFarPoint(light.pos, edge.u, box);
					auto q = getFarPoint(light.pos, edge.v, box);
					gloom.append({edge.u, shadow});
					gloom.append({edge.v, shadow});
					gloom.append({q, shadow});
					gloom.append({p, shadow});
				}
				tmp_buffer.draw(gloom, sf::BlendMultiply);
			}

			// apply this shadowmap to primary buffer
			tmp_buffer.display();
			sf::Sprite tmp{tmp_buffer.getTexture()};
			shadow_buffer.draw(tmp, sf::BlendAdd);
			++num_drawn_shadows;
		}
		if (num_drawn_shadows > 0u) {
			shadow_buffer.display();
		}
	}

	light_buffer.setView(scene);
	fog_buffer.setView(scene);
	num_drawn_lights = 0u;
	for (auto const& light : lights) {
		if (light.lod > lod) {
			// this light is not drawn at this low level of details
			continue;
		}
		// render light
		prepareLight(light);
		light_buffer.draw(light_sprite, sf::BlendAdd);
		// render fog
		float scale = 2.5f * light.radius / MAX_LIGHT_RADIUS;  // larger fog
		light_sprite.setColor(sf::Color::White);
		light_sprite.setScale({scale, scale});
		fog_buffer.draw(light_sprite, sf::BlendAdd);
		++num_drawn_lights;
	}
	if (num_drawn_lights > 0u) {
		light_buffer.display();
		fog_buffer.display();
	}

}

void LightingSystem::saveShadowMap(std::string const& filename) const {
	shadow_buffer.getTexture().copyToImage().saveToFile(filename);
}

void LightingSystem::saveLightMap(std::string const& filename) const {
	light_buffer.getTexture().copyToImage().saveToFile(filename);
}

void LightingSystem::saveFogMap(std::string const& filename) const {
	fog_buffer.getTexture().copyToImage().saveToFile(filename);
}

void LightingSystem::renderShadow(sf::RenderTarget& target) const {
	// tba: ASSERT buffers to be created

	if (lod == 0u) {
		// lighting is disabled!
		return;
	}
	if (num_drawn_lights == 0u || num_drawn_shadows == 0u) {
		// no lights, no shadows
		return;
	}

	// apply this shadow map
	sf::Sprite tmp{shadow_buffer.getTexture()};
	target.draw(tmp, sf::BlendMultiply);
}

void LightingSystem::renderLight(sf::RenderTarget& target) const {
	// tba: ASSERT buffers to be created

	if (lod == 0u) {
		// lighting is disabled!
		return;
	}
	if (num_drawn_lights == 0u) {
		// nothing to enlight
		return;
	}

	// apply this shadow map
	sf::Sprite tmp{light_buffer.getTexture()};
	target.draw(tmp, sf::BlendMultiply);
}

void LightingSystem::renderFog(sf::RenderTarget& target) const {
	// tba: ASSERT buffers to be created
	if (lod == 0u) {
		// lighting is disabled!
		return;
	}
	if (num_drawn_lights == 0u) {
		// nothing to enlight
		return;
	}

	// apply this shadow map
	sf::Sprite tmp{fog_buffer.getTexture()};
	target.draw(tmp, sf::BlendMultiply);
}

}  // ::utils
