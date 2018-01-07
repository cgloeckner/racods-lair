#include <utils/assert.hpp>

namespace utils {

template <typename Layer>
LayeredSprite<Layer>::LayeredSprite()
	: sf::Transformable{}
	, layers{}
	, brightness{1.f}
	, min_saturation{0.f}
	, max_saturation{1.f} {}

template <typename Layer>
void LayeredSprite<Layer>::render(sf::RenderTarget& target,
	sf::Transform const& matrix, sf::Shader& shader) const {
	// apply transformations and shader
	shader.setUniform("brightness", brightness);
	shader.setUniform("min_saturation", min_saturation);
	shader.setUniform("max_saturation", max_saturation);
	shader.setUniform("texture", sf::Shader::CurrentTexture);
	// create render states
	sf::RenderStates states;
	states.transform = matrix;
	// states.transform.combine(getTransform());
	states.shader = &shader;
	// draw layers
	for (auto const& pair : layers) {
		target.draw(pair.second, states);
	}
}

template <typename Layer>
sf::Sprite const& LayeredSprite<Layer>::operator[](Layer layer) const {
	return layers[layer];
}

template <typename Layer>
sf::Sprite& LayeredSprite<Layer>::operator[](Layer layer) {
	return layers[layer];
}

template <typename Layer>
void LayeredSprite<Layer>::setBrightness(float brightness) {
	ASSERT(brightness >= 0.f);
	ASSERT(brightness <= 1.f);
	this->brightness = brightness;
}

template <typename Layer>
void LayeredSprite<Layer>::setMinSaturation(float saturation) {
	ASSERT(saturation >= 0.f);
	ASSERT(saturation <= 1.f);
	min_saturation = saturation;
}

template <typename Layer>
void LayeredSprite<Layer>::setMaxSaturation(float saturation) {
	ASSERT(saturation >= 0.f);
	ASSERT(saturation <= 1.f);
	max_saturation = saturation;
}


template <typename Layer>
typename LayeredSprite<Layer>::iterator LayeredSprite<Layer>::begin() {
	return layers.begin();
}

template <typename Layer>
typename LayeredSprite<Layer>::iterator LayeredSprite<Layer>::end() {
	return layers.end();
}

template <typename Layer>
typename LayeredSprite<Layer>::const_iterator LayeredSprite<Layer>::begin() const {
	return layers.begin();
}

template <typename Layer>
typename LayeredSprite<Layer>::const_iterator LayeredSprite<Layer>::end() const {
	return layers.end();
}

template <typename Layer>
typename LayeredSprite<Layer>::iterator begin(LayeredSprite<Layer>& s) {
	return s.begin();
}

template <typename Layer>
typename LayeredSprite<Layer>::iterator end(LayeredSprite<Layer>& s) {
	return s.end();
}

template <typename Layer>
typename LayeredSprite<Layer>::const_iterator begin(LayeredSprite<Layer> const & s) {
	return s.begin();
}

template <typename Layer>
typename LayeredSprite<Layer>::const_iterator end(LayeredSprite<Layer> const & s) {
	return s.end();
}

}  // ::utils
