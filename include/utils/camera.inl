#include <cmath>
#include <stdexcept>
#include <SFML/Graphics/RenderTarget.hpp>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>
#include <utils/camera.hpp>

namespace utils {

template <typename Entity>
CameraData<Entity>::CameraData(float zoom)
	: scene{}
	, hud{}
	, zoom{zoom}
	, objects{}
	, bary_center{} {
	ASSERT(zoom > 0.f);
}

template <typename Entity>
sf::Vector2f CameraData<Entity>::hudify(
	sf::RenderTarget const& target, sf::Vector2f const& pos) const {
	return target.mapPixelToCoords(target.mapCoordsToPixel(pos, scene), hud);
}

// ---------------------------------------------------------------------------

template <typename Entity>
CameraSystem<Entity>::CameraSystem(sf::Vector2u const& size, float default_zoom)
	: cams{}
	, window_size{}
	, default_zoom{default_zoom} {
	resize(size);
	ASSERT(default_zoom > 0.f);
}

template <typename Entity>
void CameraSystem<Entity>::resize(sf::Vector2u const& size) {
	// update internal state
	window_size = size;

	// determine camera grid
	unsigned int num_cams = cams.size(), num_cols = 1u, num_rows = 1u;
	bool vertical_first = (size.x > size.y);
	while (num_cols * num_rows < num_cams) {
		if (vertical_first) {
			if (num_rows > num_cols) {
				++num_cols;
			} else {
				++num_rows;
			}
		} else {
			if (num_cols > num_rows) {
				++num_rows;
			} else {
				++num_cols;
			}
		}
	}

	// calculate size of viewport
	auto screen_size = sf::Vector2f{size};
	auto rel_size = screen_size;
	screen_size.x /= static_cast<float>(num_rows);
	screen_size.y /= static_cast<float>(num_cols);
	rel_size.x /= static_cast<float>(num_rows) * rel_size.x;
	rel_size.y /= static_cast<float>(num_cols) * rel_size.y;

	// adjust cameras
	sf::Vector2u offset;
	for (auto& ptr : cams) {
		auto& cam = *ptr;
		// set viewport
		sf::Vector2f pos;
		pos.x = offset.x / static_cast<float>(num_rows);
		pos.y = offset.y / static_cast<float>(num_cols);
		sf::FloatRect rect{pos.x, pos.y, rel_size.x, rel_size.y};
		// zoomed scene
		cam.scene.setViewport(rect);
		cam.scene.setSize(screen_size);
		cam.scene.zoom(cam.zoom);
		// centered equivalent of default view
		cam.screen.setViewport(rect);
		cam.screen.setSize(screen_size);
		pos.x *= static_cast<float>(window_size.x);
		pos.y *= static_cast<float>(window_size.y);
		cam.screen.setCenter(pos + screen_size / 2.f);
		// topleft-aligned gui-view
		cam.hud.setViewport(rect);
		cam.hud.setSize(screen_size);
		cam.hud.setCenter(screen_size / 2.f);
		// pick next position
		++offset.x;
		if (offset.x >= num_rows) {
			offset.x = 0u;
			++offset.y;
		}
	}

	// enlarge last camera
	if (!cams.empty()) {
		auto& cam = *(cams.back());
		auto rect = cam.hud.getViewport();
		sf::Vector2f pos{rect.left, rect.top};
		auto size = screen_size;
		rect.width = 1.f - rect.left;
		size.x = window_size.x * rect.width;
		// zoomed scene
		cam.scene.setViewport(rect);
		cam.scene.setSize(size);
		cam.scene.zoom(cam.zoom);
		cam.screen.setViewport(rect);
		cam.screen.setSize(size);
		// centered equivalent of default view
		pos.x *= static_cast<float>(window_size.x);
		pos.y *= static_cast<float>(window_size.y);
		pos.x += size.x / 2;
		pos.y += screen_size.y / 2.f;
		cam.screen.setCenter(pos);
		// topleft-aligned gui-view
		cam.hud.setViewport(rect);
		cam.hud.setSize(size);
		cam.hud.setCenter(size / 2.f);
	}
}

template <typename Entity>
std::size_t CameraSystem<Entity>::size() const {
	return cams.size();
}

template <typename Entity>
sf::Vector2u CameraSystem<Entity>::getWindowSize() const {
	return window_size;
}

template <typename Entity>
CameraData<Entity>& CameraSystem<Entity>::acquire() {
	cams.push_back(std::make_unique<cam>(default_zoom));
	resize(window_size);
	return *(cams.back());
}

template <typename Entity>
void CameraSystem<Entity>::release(CameraData<Entity> const& cam) {
	// search target iterator
	auto iterator =
		find_if(cams, [&](std::unique_ptr<CameraData<Entity>> const& ptr) {
			return (ptr.get() == &cam);
		});
	ASSERT(iterator != cams.end());
	// remove camera
	bool success = pop(cams, iterator);
	ASSERT(success);
	// resize entire system
	resize(window_size);
}

template <typename Entity>
void CameraSystem<Entity>::clear() {
	cams.clear();
}

template <typename Entity>
void CameraSystem<Entity>::update(CameraData<Entity>& camera,
	sf::Time const& elapsed, std::vector<sf::Vector2f> const& positions) {
	// update position
	camera.bary_center = camera_impl::getBaryCenter(positions);
	camera.scene.setCenter(camera.bary_center);

	// update zoom
	auto factor = camera_impl::getZoomFactor(
		elapsed, positions, camera.scene, camera.zoom, default_zoom);
	if (factor != 1.f) {
		camera.scene.zoom(factor);
		camera.zoom *= factor;
	}
}

template <typename Entity>
void CameraSystem<Entity>::leave(cam& cam, Entity id) {
	if (cam.objects.size() == 1u) {
		release(cam);
	} else {
		auto moved = utils::pop(cam.objects, id);
		ASSERT(moved);
	}
}

/*
template <typename Entity>
CameraData<Entity>& CameraSystem<Entity>::split(
	CameraData<Entity>& cam, Entity id) {
	ASSERT(cam.objects.size() > 1u);
	bool moved = pop(cam.objects, id);
	ASSERT(moved);
	auto& new_cam = acquire();
	new_cam.objects.push_back(id);
	return new_cam;
}

template <typename Entity>
void CameraSystem<Entity>::join(CameraData<Entity>& cam, Entity id) {
	auto& old_cam = query(id);
	auto result = pop(old_cam.objects, id);
	ASSERT(result);
	cam.objects.push_back(id);
	if (old_cam.objects.empty()) {
		release(old_cam);
	}
}
*/

template <typename Entity>
CameraData<Entity> const& CameraSystem<Entity>::query(Entity id) const {
	cam const* ptr = nullptr;
	for (auto const& tmp : cams) {
		if (contains(tmp->objects, id)) {
			ptr = tmp.get();
			break;
		}
	}
	if (ptr == nullptr) {
		throw std::out_of_range(
			"No camera assigned to object #" + std::to_string(id));
	}
	return *ptr;
}

template <typename Entity>
CameraData<Entity>& CameraSystem<Entity>::query(Entity id) {
	cam* ptr = nullptr;
	for (auto& tmp : cams) {
		if (contains(tmp->objects, id)) {
			ptr = tmp.get();
			break;
		}
	}
	if (ptr == nullptr) {
		throw std::out_of_range(
			"No camera assigned to object #" + std::to_string(id));
	}
	return *ptr;
}

template <typename Entity>
bool CameraSystem<Entity>::has(Entity id) const {
	for (auto const & ptr: cams) {
		if (contains(ptr->objects, id)) {
			return true;
		}
	}
	return false;
}

template <typename Entity>
typename CameraSystem<Entity>::container::iterator
	CameraSystem<Entity>::begin() {
	return cams.begin();
}

template <typename Entity>
typename CameraSystem<Entity>::container::iterator CameraSystem<Entity>::end() {
	return cams.end();
}

template <typename Entity>
typename CameraSystem<Entity>::container::const_iterator
	CameraSystem<Entity>::begin() const {
	return cams.begin();
}

template <typename Entity>
typename CameraSystem<Entity>::container::const_iterator
	CameraSystem<Entity>::end() const {
	return cams.end();
}

}  // ::utils
