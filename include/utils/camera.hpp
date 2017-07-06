#pragma once
#include <vector>
#include <memory>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace utils {

namespace camera_impl {

extern float const ZOOM_IN_THRESHOLD;
extern float const ZOOM_OUT_THRESHOLD;
extern float const ZOOM_SPEED;
extern float const MIN_ZOOM_LEVEL;

sf::Vector2f getBaryCenter(std::vector<sf::Vector2f> const& positions);
float getZoomFactor(sf::Time const& elapsed,
	std::vector<sf::Vector2f> const& positions, sf::View const& scene,
	float zoom, float default_zoom);

}  // ::camera_impl

// ---------------------------------------------------------------------------

template <typename Entity>
struct CameraData {
	sf::View scene;   // zoomed scene
	sf::View screen;  // centered equivalent of default view (see lighting)
	sf::View hud;	 // topleft-aligned gui view
	float zoom;
	std::vector<Entity> objects;
	sf::Vector2f bary_center;

	CameraData(float zoom);

	sf::Vector2f hudify(
		sf::RenderTarget const& target, sf::Vector2f const& pos) const;
};

// ---------------------------------------------------------------------------

template <typename Entity>
class CameraSystem {
  private:
	using cam = CameraData<Entity>;
	using container = std::vector<std::unique_ptr<cam>>;

	container cams;
	sf::Vector2u window_size;

	float const default_zoom;

  public:
	/// Create Camera System
	/**
	 *	The current window's size is obtained to be able to resize all
	 *	cameras and filling the screen exactly.
	 *	Using default_zoom=2.f will cause Pixel Doubeling, which looks
	 *	like zooming in. So if a camera would display 640x480 pixels,
	 *	it will only show 320x240 pixels but each one doubled.
	 *	d
	 *	@param size of current window
	 *	@param default_zoom for each camera
	 */
	CameraSystem(sf::Vector2u const& size, float default_zoom = 1.f);

	/// Resize entire camera system
	void resize(sf::Vector2u const& size);

	std::size_t size() const;
	sf::Vector2u getWindowSize() const;

	/// Acquire new camea
	cam& acquire();

	/// Release entire camera
	/**
	 *	@pre: camera does exist
	 *	@post: camera doesn't exist
	 */
	void release(cam const& cam);

	/// Clear all cameras
	void clear();

	/// Update camera's origin
	void update(cam& camera, sf::Time const& elapsed,
		std::vector<sf::Vector2f> const& positions);

	void leave(cam& cam, Entity id);

	/*
	/// Split object to seperate camera
	**
	 *	@pre: camera has more then one object
	 *
	cam& split(cam& cam, Entity id);

	/// Join object to given camera
	**
	 *	@pre: cam exists, object is inside cam
	 *
	void join(cam& cam, Entity id);
	*/

	/// query camera by object
	/**
	 *	@throws std::out_of_range in object not found
	 */
	cam const& query(Entity id) const;

	/// query camera by object
	/**
	 *	@throws std::out_of_range in object not found
	 */
	cam& query(Entity id);
	
	/// query whether object is focused by any camera
	/// @param id Entity id
	/// @return true if a camera exists
	bool has(Entity id) const;

	typename container::iterator begin();
	typename container::iterator end();
	typename container::const_iterator begin() const;
	typename container::const_iterator end() const;
};

}  // ::utils

// include implementation details
#include <utils/camera.inl>
