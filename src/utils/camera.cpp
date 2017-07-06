#include <utils/camera.hpp>

namespace utils {

namespace camera_impl {

float const ZOOM_IN_THRESHOLD = 0.5f;
float const ZOOM_OUT_THRESHOLD = 0.6f;
float const ZOOM_SPEED = 1.001f;
float const MIN_ZOOM_LEVEL = 0.5f;

sf::Vector2f getBaryCenter(std::vector<sf::Vector2f> const& positions) {
	sf::Vector2f center;
	for (auto const& vertex : positions) {
		center += vertex;
	}
	center /= static_cast<float>(positions.size());
	return center;
}

float getZoomFactor(sf::Time const& elapsed,
	std::vector<sf::Vector2f> const& positions, sf::View const& scene,
	float zoom, float default_zoom) {
	ASSERT(default_zoom != 0.f);
	float max_dist = 0.f;
	for (auto i = 0u; i < positions.size(); ++i) {
		for (auto j = i + 1u; j < positions.size(); ++j) {
			float d = distance(positions[i], positions[j]);
			if (d > max_dist) {
				max_dist = d;
			}
		}
	}
	auto size = sf::Vector2f{scene.getSize()};
	float pivot = std::min(size.x, size.y);
	max_dist = std::sqrt(max_dist) / zoom;
	float zoom_delta = 1.f + elapsed.asSeconds() * camera_impl::ZOOM_SPEED;
	float factor{1.f};
	if (max_dist > pivot * camera_impl::ZOOM_OUT_THRESHOLD * default_zoom) {
		factor *= zoom_delta;
	} else if (max_dist <
			   pivot * camera_impl::ZOOM_IN_THRESHOLD * default_zoom) {
		factor /= zoom_delta;
	}
	auto new_zoom = zoom * factor;
	if (new_zoom < camera_impl::MIN_ZOOM_LEVEL) {
		return 1.f;
	}
	return factor;
}

}  // ::camera_impl

}  // ::utils
