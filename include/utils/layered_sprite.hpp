#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <utils/enum_map.hpp>

namespace utils {

/**
 *	@note This class is NOT unit-tested because it is coupled to the rendering
 *	system too much.
 */
template <typename Layer>
class LayeredSprite : public sf::Transformable {

  private:
	using container = EnumMap<Layer, sf::Sprite>;
	using iterator = typename container::iterator;
	using const_iterator = typename container::const_iterator;
	
	container layers;
	float brightness, min_saturation, max_saturation;

  public:
	LayeredSprite();

	sf::Sprite const& operator[](Layer layer) const;
	sf::Sprite& operator[](Layer layer);

	void setBrightness(float brightness);
	void setMinSaturation(float saturation);
	void setMaxSaturation(float saturation);

	void render(sf::RenderTarget& target, sf::Transform const& matrix,
		sf::Shader& shader) const;
	
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
};

template <typename Layer>
typename LayeredSprite<Layer>::iterator begin(LayeredSprite<Layer>& s);

template <typename Layer>
typename LayeredSprite<Layer>::iterator end(LayeredSprite<Layer>& s);

template <typename Layer>
typename LayeredSprite<Layer>::const_iterator begin(LayeredSprite<Layer> const & s);

template <typename Layer>
typename LayeredSprite<Layer>::const_iterator end(LayeredSprite<Layer> const & s);

}  // ::utils

// include implementation details
#include <utils/layered_sprite.inl>
