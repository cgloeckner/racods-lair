#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace utils {

struct Edge {
	sf::Vector2f u, v;

	Edge();
};

// ---------------------------------------------------------------------------

struct Light {
	sf::Vector2f pos;
	sf::Color color;
	sf::Uint8 intensity;
	float radius;
	bool cast_shadow;
	std::size_t lod;

	Light();
};

// ---------------------------------------------------------------------------

extern float const MAX_LIGHT_RADIUS;

/// Calculate a "far" point outside the box
/**
 *	The far point is located at the ray from origin through pos outside the
 *	given box. In most cases the resulting point is not directly located at the
 *	border. But that's not necessary here.
 *
 *	@pre origin != pos
 *	@param origin origin of the ray
 *	@param pos position which the ray runs through
 *	@return far point
 */
sf::Vector2f getFarPoint(
	sf::Vector2f const& origin, sf::Vector2f pos, sf::FloatRect const& box);

sf::Texture createLightmap(float radius, sf::Shader& shader);

// ---------------------------------------------------------------------------

/**
 *	@note This class is NOT unit-tested because it is coupled to the rendering
 *	system too much.
 *
 *	@note: Only axis-aligned edges are allowed
 */
class LightingSystem {
  private:
	sf::RenderTexture shadow_buffer, light_buffer, fog_buffer, tmp_buffer;
	sf::View default_view;
	sf::Color shadow;
	sf::Sprite light_sprite;
	std::size_t lod, num_drawn_lights, num_drawn_shadows;

	void prepareLight(Light const& light);

  public:
	LightingSystem(sf::Vector2u const& size, sf::Texture const& lightmap);

	void setShadowColor(sf::Color const& color);
	void setLevelOfDetails(std::size_t lod);
	std::size_t getLevelOfDetails() const;

	sf::Texture const & getLightmap() const;

	void resize(sf::Vector2u const& size);

	void clear();

	/// scene and screen are necessary due to splitscreen
	void update(sf::View const& scene, sf::View const& screen,
		std::vector<Edge> const& edges, std::vector<Light> const& lights);

	void saveShadowMap(std::string const& filename) const;
	void saveLightMap(std::string const& filename) const;
	void saveFogMap(std::string const& filename) const;

	void renderShadow(sf::RenderTarget& target) const;
	void renderLight(sf::RenderTarget& target) const;
	void renderFog(sf::RenderTarget& target) const;
};

}  // ::utils
