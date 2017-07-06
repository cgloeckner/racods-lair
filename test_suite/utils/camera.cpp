#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>
#include <utils/camera.hpp>

using ObjectID = std::uint8_t;
using CameraData = utils::CameraData<ObjectID>;
using CameraSystem = utils::CameraSystem<ObjectID>;

BOOST_AUTO_TEST_SUITE(camera_test)

BOOST_AUTO_TEST_CASE(camera_acquire_adds_camera) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	cam1.objects.push_back(7u);
	BOOST_CHECK_NO_THROW(sys.query(7u));
}

BOOST_AUTO_TEST_CASE(camera_systemctor_with_non_positive_zoom_fails) {
	CameraSystem({320, 240}, 0.5f);
	BOOST_CHECK_ASSERT(CameraSystem({320, 240}, 0.f));
	BOOST_CHECK_ASSERT(CameraSystem({320, 240}, -2.f));
}

BOOST_AUTO_TEST_CASE(camera_datactor_with_non_positive_zoom_fails) {
	CameraData(0.5f);
	BOOST_CHECK_ASSERT(CameraData(0.f));
	BOOST_CHECK_ASSERT(CameraData(-2.f));
}

BOOST_AUTO_TEST_CASE(camera_pixeldoubling_implies_zooming_in_by_factor_two) {
	CameraSystem single_sys{{320, 240}};
	auto& single_cam = single_sys.acquire();
	auto single_size = single_cam.scene.getSize();
	auto single_center = single_cam.scene.getCenter();

	CameraSystem double_sys{{320, 240}, 0.5f};
	auto& double_cam = double_sys.acquire();
	auto double_size = double_cam.scene.getSize();
	auto double_center = double_cam.scene.getCenter();

	single_size /= 2.f;  // because each dimension was halfed
	BOOST_CHECK_VECTOR_CLOSE(double_size, single_size, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(double_center, single_center, 0.0001f);
}

BOOST_AUTO_TEST_CASE(camera_pixelhalfing_implies_zooming_out_by_factor_two) {
	CameraSystem single_sys{{320, 240}};
	auto& single_cam = single_sys.acquire();
	auto single_size = single_cam.scene.getSize();
	auto single_center = single_cam.scene.getCenter();

	CameraSystem half_sys{{320, 240}, 2.f};
	auto& half_cam = half_sys.acquire();
	auto half_size = half_cam.scene.getSize();
	auto half_center = half_cam.scene.getCenter();

	single_size *= 2.f;  // because each dimension was doubled
	BOOST_CHECK_VECTOR_CLOSE(half_size, single_size, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(half_center, single_center, 0.0001f);
}

BOOST_AUTO_TEST_CASE(camera_acquire_adds_multiple_cameras) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	auto& cam2 = sys.acquire();
	cam1.objects.push_back(7u);
	cam2.objects.push_back(13u);
	BOOST_CHECK_NO_THROW(sys.query(7u));
	BOOST_CHECK_NO_THROW(sys.query(13u));
}

BOOST_AUTO_TEST_CASE(camera_acquire_adds_shared_camera) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	cam1.objects.push_back(7u);
	cam1.objects.push_back(13u);
	BOOST_CHECK_NO_THROW(sys.query(7u));
	BOOST_CHECK_NO_THROW(sys.query(13u));
}

BOOST_AUTO_TEST_CASE(camera_release_removes_single_camera) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	cam1.objects.push_back(7u);
	BOOST_REQUIRE_NO_THROW(sys.query(7u));

	sys.release(cam1);
	BOOST_CHECK_THROW(sys.query(7u), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(camera_release_removes_one_of_multiple_cameras) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	auto& cam2 = sys.acquire();
	cam1.objects.push_back(7u);
	cam2.objects.push_back(13u);
	BOOST_REQUIRE_NO_THROW(sys.query(7u));

	sys.release(cam1);
	BOOST_CHECK_THROW(sys.query(7u), std::out_of_range);
	BOOST_CHECK_NO_THROW(sys.query(13u));
}

/*
BOOST_AUTO_TEST_CASE(camera_join_disjoint_cameras_and_release_old) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	auto& cam2 = sys.acquire();
	cam1.objects.push_back(7u);
	cam2.objects.push_back(11u);

	BOOST_REQUIRE_NO_ASSERT(sys.join(cam1, 11u));
	BOOST_CHECK(utils::contains(cam1.objects, 7u));
	BOOST_CHECK(utils::contains(cam1.objects, 11u));
	BOOST_CHECK_ASSERT(sys.release(cam2));  // doesn't exist anymore
}

BOOST_AUTO_TEST_CASE(camera_join_disjoint_cameras_and_but_old_one_stays_alive) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	auto& cam2 = sys.acquire();
	cam1.objects.push_back(7u);
	cam2.objects.push_back(11u);
	cam2.objects.push_back(5u);  // will keep old camera alive

	BOOST_REQUIRE_NO_ASSERT(sys.join(cam1, 11u));
	BOOST_CHECK(utils::contains(cam1.objects, 7u));
	BOOST_CHECK(utils::contains(cam1.objects, 11u));
	BOOST_CHECK_EQUAL(&cam2, &sys.query(5u));
}

BOOST_AUTO_TEST_CASE(camera_join_intersecting_cameras) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	auto& cam2 = sys.acquire();
	cam1.objects.push_back(7u);
	cam2.objects.push_back(7u);
	cam2.objects.push_back(11u);

	BOOST_REQUIRE_NO_ASSERT(sys.join(cam1, 11u));
	BOOST_CHECK(utils::contains(cam1.objects, 7u));
	BOOST_CHECK(utils::contains(cam1.objects, 11u));
	BOOST_CHECK(utils::contains(cam2.objects, 7u));
}

BOOST_AUTO_TEST_CASE(camera_split_camera_into_two_cameras) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	cam1.objects.push_back(7u);
	cam1.objects.push_back(11u);

	auto& cam2 = sys.split(cam1, 7u);
	BOOST_CHECK(utils::contains(cam1.objects, 11u));
	BOOST_CHECK(utils::contains(cam2.objects, 7u));
	BOOST_CHECK_EQUAL(&cam1, &sys.query(11u));
	BOOST_CHECK_EQUAL(&cam2, &sys.query(7u));
}
*/

BOOST_AUTO_TEST_CASE(camera_leave_drops_camera_if_was_last_entity) {
	CameraSystem sys{{320, 240}};
	BOOST_REQUIRE_EQUAL(sys.size(), 0u);
	auto& cam = sys.acquire();
	cam.objects.push_back(1u);
	sys.leave(cam, 1u);
	BOOST_CHECK_EQUAL(sys.size(), 0u);
}

BOOST_AUTO_TEST_CASE(camera_leave_drops_camera_if_was_not_last_entity) {
	CameraSystem sys{{320, 240}};
	BOOST_REQUIRE_EQUAL(sys.size(), 0u);
	auto& cam = sys.acquire();
	cam.objects.push_back(1u);
	BOOST_REQUIRE_EQUAL(sys.size(), 1u);
	cam.objects.push_back(2u);
	sys.leave(cam, 1u);
	BOOST_REQUIRE_EQUAL(sys.size(), 1u);
	BOOST_REQUIRE_EQUAL(cam.objects.size(), 1u);
	BOOST_CHECK_EQUAL(cam.objects.front(), 2u);
}

BOOST_AUTO_TEST_CASE(camera_single_screen_resizes_correctly) {
	CameraSystem sys{{320, 240}};
	auto& cam = sys.acquire();
	// assert cam1 at center
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(320.f, 240.f), cam.screen.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 120.f), cam.screen.getCenter(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(320.f, 240.f), cam.hud.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 120.f), cam.hud.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	camera_twice_splitted_screen_resizes_first_cam_correctly_to_left_half) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	sys.acquire();

	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 240.f), cam1.screen.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(80.f, 120.f), cam1.screen.getCenter(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 240.f), cam1.hud.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(80.f, 120.f), cam1.hud.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	camera_twice_splitted_screen_resizes_second_cam_correctly_to_right_half) {
	CameraSystem sys{{320, 240}};
	sys.acquire();
	auto& cam2 = sys.acquire();

	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 240.f), cam2.screen.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(240.f, 120.f), cam2.screen.getCenter(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 240.f), cam2.hud.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(80.f, 120.f), cam2.hud.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	camera_twice_splitted_screen_resizes_first_cam_correctly_to_top_half_if_screen_very_high) {
	CameraSystem sys{{300, 800}};
	auto& cam1 = sys.acquire();
	sys.acquire();

	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(300.f, 400.f), cam1.screen.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(150.f, 200.f), cam1.screen.getCenter(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(300.f, 400.f), cam1.hud.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(150.f, 200.f), cam1.hud.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	camera_twice_splitted_screen_resizes_second_cam_correctly_to_bottom_half_if_screen_very_high) {
	CameraSystem sys{{300, 800}};
	sys.acquire();
	auto& cam2 = sys.acquire();

	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(300.f, 400.f), cam2.screen.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(150.f, 600.f), cam2.screen.getCenter(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(300.f, 400.f), cam2.hud.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(150.f, 200.f), cam2.hud.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	camera_three_times_splitted_screen_resizes_first_cam_correctly_to_topleft) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	sys.acquire();
	sys.acquire();

	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 120.f), cam1.screen.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(80.f, 60.f), cam1.screen.getCenter(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 120.f), cam1.hud.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(80.f, 60.f), cam1.hud.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	camera_three_times_splitted_screen_resizes_second_cam_correctly_to_topright) {
	CameraSystem sys{{320, 240}};
	sys.acquire();
	auto& cam2 = sys.acquire();
	sys.acquire();

	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 120.f), cam2.screen.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(240.f, 60.f), cam2.screen.getCenter(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 120.f), cam2.hud.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(80.f, 60.f), cam2.hud.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	camera_three_times_splitted_screen_resizes_third_cam_correctly_to_entire_bottom) {
	CameraSystem sys{{320, 240}};
	sys.acquire();
	sys.acquire();
	auto& cam3 = sys.acquire();
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(320.f, 120.f), cam3.screen.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 180.f), cam3.screen.getCenter(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(320.f, 120.f), cam3.hud.getSize(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(160.f, 60.f), cam3.hud.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(camera_update_camera_origin_by_single_position) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	std::vector<sf::Vector2f> positions;
	positions.emplace_back(143.f, 546.f);

	sys.update(cam1, sf::milliseconds(20), positions);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(143.f, 546.f), cam1.scene.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(camera_update_camera_origin_by_two_position) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	std::vector<sf::Vector2f> positions;
	positions.emplace_back(100.f, 25.f);
	positions.emplace_back(200.f, 75.f);

	sys.update(cam1, sf::milliseconds(20), positions);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(150.f, 50.f), cam1.scene.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(camera_update_camera_origin_by_three_position) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	std::vector<sf::Vector2f> positions;
	positions.emplace_back(70.f, 20.f);
	positions.emplace_back(50.f, 70.f);
	positions.emplace_back(60.f, 30.f);

	sys.update(cam1, sf::milliseconds(20), positions);
	BOOST_CHECK_VECTOR_CLOSE(
		sf::Vector2f(60.f, 40.f), cam1.scene.getCenter(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(camera_update_camera_changes_zoom) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	std::vector<sf::Vector2f> positions;
	positions.emplace_back(100.f, 25.f);
	positions.emplace_back(200.f, 75.f);
	float old_zoom = cam1.zoom;

	sys.update(cam1, sf::milliseconds(50), positions);
	BOOST_CHECK_LE(cam1.zoom, old_zoom);
}

BOOST_AUTO_TEST_CASE(camera_query_cam_by_object) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	auto& cam2 = sys.acquire();
	cam1.objects.push_back(7u);
	cam1.objects.push_back(5u);
	cam2.objects.push_back(11u);

	BOOST_CHECK_EQUAL(&cam1, &sys.query(7u));
	BOOST_CHECK_EQUAL(&cam1, &sys.query(5u));
	BOOST_CHECK_EQUAL(&cam2, &sys.query(11u));
}

BOOST_AUTO_TEST_CASE(camera_query_cam_by_object_that_has_multiple_cams) {
	CameraSystem sys{{320, 240}};
	auto& cam1 = sys.acquire();
	auto& cam2 = sys.acquire();
	cam1.objects.push_back(7u);
	cam1.objects.push_back(11u);
	cam2.objects.push_back(11u);

	BOOST_CHECK_EQUAL(&cam1, &sys.query(11u));
}

BOOST_AUTO_TEST_SUITE_END()
