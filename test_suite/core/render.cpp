#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <core/render.hpp>

struct RenderFixture {
	sf::Texture dummy_texture, dummy_texture2;
	core::IdManager id_manager;
	std::vector<core::ObjectID> ids;

	core::LogContext log;
	core::RenderManager render_manager;
	core::AnimationManager animation_manager;
	core::MovementManager movement_manager;
	core::FocusManager focus_manager;
	core::DungeonSystem dungeon_system;
	core::CameraSystem camera_system;
	utils::LightingSystem lighting_system;
	core::render_impl::Context context;

	struct {
		utils::ActionFrames legs;
		utils::EnumMap<core::AnimationAction, utils::ActionFrames> torso;
	} demo_template;

	RenderFixture(sf::Vector2u const& map_size = {12u, 12u})
		: dummy_texture{}
		, dummy_texture2{}
		, id_manager{}
		, log{}
		, render_manager{}
		, animation_manager{}
		, movement_manager{}
		, focus_manager{}
		, dungeon_system{}
		, camera_system{{320, 180}}
		, lighting_system{{320, 180}, dummy_texture}
		, context{log, render_manager, animation_manager, movement_manager,
			focus_manager, dungeon_system, camera_system, lighting_system} {
		// add a scenes
		auto scene = dungeon_system.create(
			dummy_texture, map_size, sf::Vector2f{64.f, 64.f});
		assert(scene == 1u);
		auto& dungeon = dungeon_system[1u];
		for (auto y = 0u; y < map_size.y; ++y) {
			for (auto x = 0u; x < map_size.x; ++x) {
				auto& cell = dungeon.getCell({x, y});
				cell.terrain = core::Terrain::Floor;
				cell.tile.refresh(sf::Vector2u(x, y), {32u, 32u}, {},
					{64u, 64u}, utils::ShadeTopLeft, true);
			}
		}
		// create demo animation template
		demo_template.legs.frames.reserve(4u);
		demo_template.legs.append(
			{0, 0, 10, 5}, {1.f, 0.2f}, sf::milliseconds(15));
		demo_template.legs.append(
			{10, 0, 10, 5}, {1.f, 0.2f}, sf::milliseconds(17));
		demo_template.legs.append(
			{20, 0, 10, 5}, {1.f, 0.2f}, sf::milliseconds(23));
		demo_template.legs.append(
			{30, 0, 10, 5}, {1.f, 0.2f}, sf::milliseconds(12));
		demo_template.legs.refresh();
		for (auto& pair : demo_template.torso) {
			pair.second.frames.reserve(4u);
			pair.second.append(
				{0, 5, 10, 5}, {1.2f, 0.5f}, sf::milliseconds(15));
			pair.second.append(
				{10, 5, 10, 5}, {1.2f, 0.5f}, sf::milliseconds(17));
			pair.second.append(
				{20, 5, 10, 5}, {1.2f, 0.5f}, sf::milliseconds(23));
			pair.second.append(
				{30, 5, 10, 5}, {1.2f, 0.5f}, sf::milliseconds(12));
			pair.second.refresh();
		}
	}

	void reset() {
		auto& dungeon = dungeon_system[1u];
		// clear dungeon
		for (auto y = 0u; y < 12u; ++y) {
			for (auto x = 0u; x < 12u; ++x) {
				dungeon.getCell({x, y}).entities.clear();
			}
		}
		// remove components
		for (auto id : ids) {
			if (movement_manager.has(id)) {
				movement_manager.release(id);
			}
			if (focus_manager.has(id)) {
				focus_manager.release(id);
			}
			if (animation_manager.has(id)) {
				animation_manager.release(id);
			}
			render_manager.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		movement_manager.cleanup();
		focus_manager.cleanup();
		animation_manager.cleanup();
		render_manager.cleanup();
		camera_system.clear();
	}

	core::ObjectID add_object(
		sf::Vector2u const& pos, sf::Vector2i const& look, float sight=0.f) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		render_manager.acquire(id);
		auto& move_data = movement_manager.acquire(id);
		move_data.pos = sf::Vector2f{pos};
		move_data.scene = 1u;
		move_data.look = look;
		if (sight > 0.f) {
			auto& focus_data = focus_manager.acquire(id);
			focus_data.sight = sight;
			focus_data.fov = 120.f;
		}
		auto& ani_data = animation_manager.acquire(id);
		for (auto& pair : ani_data.tpl.legs) {
			pair.second = &demo_template.legs;
		}
		for (auto& pair : ani_data.tpl.torso) {
			pair.second = &demo_template.torso;
		}
		auto& dungeon = dungeon_system[1u];
		dungeon.getCell(pos).entities.push_back(id);
		return id;
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(render_test)

BOOST_AUTO_TEST_CASE(culling_buffer_draws_quads_per_tile) {
	core::render_impl::CullingBuffer buffer;
	for (auto const& pair : buffer.terrain) {
		BOOST_REQUIRE(pair.second.getPrimitiveType() == sf::Triangles);
	}
}

BOOST_AUTO_TEST_CASE(culling_buffer_draws_lines_for_gridborders) {
	core::render_impl::CullingBuffer buffer;
	BOOST_CHECK(buffer.grid.getPrimitiveType() == sf::Lines);
}

BOOST_AUTO_TEST_CASE(looking_south_causes_zero_degree_rotation) {
	BOOST_CHECK_CLOSE(0.f, core::render_impl::getRotation({0, 1}), 0.0001f);
}

BOOST_AUTO_TEST_CASE(looking_southwest_causes_45_degree_rotation) {
	BOOST_CHECK_CLOSE(45.f, core::render_impl::getRotation({-1, 1}), 0.0001f);
}

BOOST_AUTO_TEST_CASE(looking_north_causes_180_degree_rotation) {
	BOOST_CHECK_CLOSE(180.f, core::render_impl::getRotation({0, -1}), 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(applying_animation_will_alter_leg_layers_synchronously) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	auto& actor_ani = fix.animation_manager.query(id);
	++actor_ani.legs.index;
	// apply current animation
	core::render_impl::applyAnimation(actor_ani, actor_render);
	// assert sprite properties
	auto const& leg_base_rect =
		actor_render.legs[core::SpriteLegLayer::Base].getTextureRect();
	auto const& leg_armor_rect =
		actor_render.legs[core::SpriteLegLayer::Armor].getTextureRect();
	auto const& leg_base_origin =
		actor_render.legs[core::SpriteLegLayer::Base].getOrigin();
	auto const& leg_armor_origin =
		actor_render.legs[core::SpriteLegLayer::Armor].getOrigin();
	BOOST_CHECK_RECT_EQUAL(leg_base_rect, sf::IntRect(10, 0, 10, 5));
	BOOST_CHECK_RECT_EQUAL(leg_armor_rect, sf::IntRect(10, 0, 10, 5));
	BOOST_CHECK_VECTOR_CLOSE(leg_base_origin, sf::Vector2f(1.f, 0.2f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		leg_armor_origin, sf::Vector2f(1.f, 0.2f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(applying_animation_will_alter_torso_layers_synchronously) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	auto& actor_ani = fix.animation_manager.query(id);
	++actor_ani.torso.index;
	// apply current animation
	core::render_impl::applyAnimation(actor_ani, actor_render);
	// assert sprite properties
	auto const& torso_base_rect =
		actor_render.torso[core::SpriteTorsoLayer::Base].getTextureRect();
	auto const& torso_armor_rect =
		actor_render.torso[core::SpriteTorsoLayer::Armor].getTextureRect();
	auto const& weapon_rect =
		actor_render.torso[core::SpriteTorsoLayer::Weapon].getTextureRect();
	auto const& torso_base_origin =
		actor_render.torso[core::SpriteTorsoLayer::Base].getOrigin();
	auto const& torso_armor_origin =
		actor_render.torso[core::SpriteTorsoLayer::Armor].getOrigin();
	auto const& weapon_origin =
		actor_render.torso[core::SpriteTorsoLayer::Weapon].getOrigin();
	// note: in general, the values aren't that synchronous, but belong to the
	// same frame index
	BOOST_CHECK_RECT_EQUAL(torso_base_rect, sf::IntRect(10, 5, 10, 5));
	BOOST_CHECK_RECT_EQUAL(torso_armor_rect, sf::IntRect(10, 5, 10, 5));
	BOOST_CHECK_RECT_EQUAL(weapon_rect, sf::IntRect(10, 5, 10, 5));
	BOOST_CHECK_VECTOR_CLOSE(
		torso_base_origin, sf::Vector2f(1.2f, 0.5f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(
		torso_armor_origin, sf::Vector2f(1.2f, 0.5f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(weapon_origin, sf::Vector2f(1.2f, 0.5f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(applying_animation_skipps_unused_layers) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	auto& actor_ani = fix.animation_manager.query(id);
	++actor_ani.legs.index;
	actor_ani.tpl.legs[core::SpriteLegLayer::Armor] = nullptr;
	actor_ani.tpl.torso[core::SpriteTorsoLayer::Armor] = nullptr;
	// apply current animation
	core::render_impl::applyAnimation(actor_ani, actor_render);
	// assert sprite properties
	auto const& leg_base_rect =
		actor_render.legs[core::SpriteLegLayer::Base].getTextureRect();
	auto const& leg_armor_rect =
		actor_render.legs[core::SpriteLegLayer::Armor].getTextureRect();
	auto const& leg_base_origin =
		actor_render.legs[core::SpriteLegLayer::Base].getOrigin();
	auto const& leg_armor_origin =
		actor_render.legs[core::SpriteLegLayer::Armor].getOrigin();
	BOOST_CHECK_RECT_EQUAL(leg_base_rect, sf::IntRect(10, 0, 10, 5));
	BOOST_CHECK_RECT_EQUAL(leg_armor_rect, sf::IntRect(0, 0, 0, 0));
	BOOST_CHECK_VECTOR_CLOSE(leg_base_origin, sf::Vector2f(1.f, 0.2f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(leg_armor_origin, sf::Vector2f(0.f, 0.f), 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(update_object_fails_if_no_scene_assigned) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	auto& move_render = fix.movement_manager.query(id);
	move_render.scene = 0u;
	// expect assertion
	BOOST_CHECK_ASSERT(
		core::render_impl::updateObject(fix.context, actor_render));
}

BOOST_AUTO_TEST_CASE(update_object_fails_if_no_movement_component_exists) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	// release movement component
	fix.movement_manager.release(id);
	fix.movement_manager.cleanup();
	// expect assertion
	BOOST_CHECK_ASSERT(
		core::render_impl::updateObject(fix.context, actor_render));
}

BOOST_AUTO_TEST_CASE(object_with_only_move_and_render_can_be_updated) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	// setup minimal object
	auto id = fix.id_manager.acquire();
	fix.ids.push_back(id);
	auto& render_data = fix.render_manager.acquire(id);
	auto& move_data = fix.movement_manager.acquire(id);
	move_data.pos = {2.f, 5.f};
	move_data.scene = 1u;
	move_data.has_changed = true;
	auto& dungeon = fix.dungeon_system[1u];
	dungeon.getCell({2u, 5u}).entities.push_back(id);
	// update
	BOOST_CHECK_NO_ASSERT(
		core::render_impl::updateObject(fix.context, render_data));
}

BOOST_AUTO_TEST_CASE(
	update_object_without_dirtyflags_doesnt_change_any_matrix) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, {-1, -1});
	auto& actor_render = fix.render_manager.query(id);
	auto& actor_move = fix.movement_manager.query(id);
	actor_move.has_changed = false;
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert expected matrix
	BOOST_CHECK_4x4_MATRIX_CLOSE(
		actor_render.legs_matrix, sf::Transform::Identity, 0.0001f);
	BOOST_CHECK_4x4_MATRIX_CLOSE(
		actor_render.torso_matrix, sf::Transform::Identity, 0.0001f);
}

BOOST_AUTO_TEST_CASE(move_dirtyflag_will_change_legs_matrices) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	auto& actor_move = fix.movement_manager.query(id);
	actor_move.has_changed = true;
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert expected matrix
	auto& dungeon = fix.dungeon_system[1];
	auto expected = sf::Transform::Identity;
	expected.translate(dungeon.toScreen(actor_move.pos));
	expected.rotate(core::render_impl::getRotation({0, 1}));
	BOOST_CHECK_4x4_MATRIX_CLOSE(actor_render.legs_matrix, expected, 0.0001f);
}

BOOST_AUTO_TEST_CASE(move_dirtyflag_will_change_highlight_pos) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	actor_render.highlight = std::make_unique<sf::Sprite>();
	auto& actor_move = fix.movement_manager.query(id);
	actor_move.has_changed = true;
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// expect different position
	BOOST_CHECK(actor_render.highlight->getPosition() != sf::Vector2f{});
}

BOOST_AUTO_TEST_CASE(move_dirtyflag_will_does_not_change_fov_direction) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, {0, 1}, 1.f);
	auto& actor_render = fix.render_manager.query(id);
	auto& actor_move = fix.movement_manager.query(id);
	actor_move.has_changed = true;
	actor_move.look = {0, -1};
	// assert different value (because it's not updated yet)
	BOOST_REQUIRE_VECTOR_CLOSE(actor_render.fov.getDirection(), sf::Vector2f(0.f, 1.f), 0.0001f);
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert same direction
	// note: drawing the fov uses the sprite's transformation matrix
	// (including the proper rotation)
	BOOST_REQUIRE_VECTOR_CLOSE(actor_render.fov.getDirection(), sf::Vector2f(0.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(focus_dirtyflag_will_change_fov_settings) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, {0, 1}, 1.f);
	auto& actor_render = fix.render_manager.query(id);
	auto& actor_focus = fix.focus_manager.query(id);
	actor_focus.has_changed = true;
	actor_focus.sight = 7.5f;
	actor_focus.fov = 90.f;
	actor_focus.is_active = true;
	// assert different values (because it's not updated yet)
	BOOST_REQUIRE_CLOSE(actor_render.fov.getRadius(), 0.f, 0.001f);
	BOOST_REQUIRE_CLOSE(actor_render.fov.getAngle(), 360.f, 0.001f);
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert expected settings
	auto tile_size = fix.dungeon_system[1].getTileSize().x;
	BOOST_CHECK_CLOSE(actor_render.fov.getRadius(), actor_focus.sight * tile_size, 0.001f);
	BOOST_CHECK_CLOSE(actor_render.fov.getAngle(), actor_focus.fov, 0.001f);
}


BOOST_AUTO_TEST_CASE(focus_dirtyflag_will_set_radius_to_zero_if_inactive) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, {0, 1}, 1.f);
	auto& actor_render = fix.render_manager.query(id);
	actor_render.fov.setRadius(10.f);
	actor_render.fov.setOrigin({10.f, 10.f});
	auto& actor_focus = fix.focus_manager.query(id);
	actor_focus.has_changed = true;
	actor_focus.sight = 7.5f;
	actor_focus.fov = 90.f;
	actor_focus.is_active = false;
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert expected settings
	BOOST_CHECK_CLOSE(actor_render.fov.getRadius(), 0.f, 0.001f);
	BOOST_CHECK_VECTOR_CLOSE(actor_render.fov.getOrigin(), sf::Vector2f(0.f, 0.f), 0.001f);
}

// -----------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(
	animation_dirtyflag_will_cause_rect_and_origin_to_be_changed) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	auto& actor_ani = fix.animation_manager.query(id);
	actor_ani.torso.index = 2u;
	actor_ani.has_changed = true;
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert rect and origin
	auto const& rect =
		actor_render.torso[core::SpriteTorsoLayer::Base].getTextureRect();
	auto const& origin =
		actor_render.torso[core::SpriteTorsoLayer::Base].getOrigin();
	BOOST_CHECK_RECT_EQUAL(rect, sf::IntRect(20, 5, 10, 5));
	BOOST_CHECK_VECTOR_CLOSE(origin, sf::Vector2f(1.2f, 0.5f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(animation_without_dirtyflag_will_keep_rect_and_origin) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	auto& sprite = actor_render.torso[core::SpriteTorsoLayer::Base];
	sprite.setTextureRect({3, 5, 10, 5});
	sprite.setOrigin({0.2f, 1.9f});
	auto& actor_ani = fix.animation_manager.query(id);
	actor_ani.torso.index = 2u;
	actor_ani.has_changed = false;
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert rect and origin
	auto const& rect =
		actor_render.torso[core::SpriteTorsoLayer::Base].getTextureRect();
	auto const& origin =
		actor_render.torso[core::SpriteTorsoLayer::Base].getOrigin();
	BOOST_CHECK_RECT_EQUAL(rect, sf::IntRect(3, 5, 10, 5));
	BOOST_CHECK_VECTOR_CLOSE(origin, sf::Vector2f(0.2f, 1.9f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	animation_without_dirtyflag_doesnt_change_light_intensity) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	actor_render.light = std::make_unique<utils::Light>();
	actor_render.light->intensity = 15u;
	auto& sprite = actor_render.torso[core::SpriteTorsoLayer::Base];
	sprite.setTextureRect({3, 5, 10, 5});
	sprite.setOrigin({0.2f, 1.9f});
	auto& actor_ani = fix.animation_manager.query(id);
	actor_ani.light_intensity = 24.f;
	actor_ani.has_changed = false;
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert rect and origin
	BOOST_CHECK_EQUAL(actor_render.light->intensity, 15u);
}

BOOST_AUTO_TEST_CASE(animation_with_dirtyflag_changes_light_intensity) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({}, {0, 1});
	auto& actor_render = fix.render_manager.query(id);
	actor_render.light = std::make_unique<utils::Light>();
	actor_render.light->intensity = 15u;
	auto& sprite = actor_render.torso[core::SpriteTorsoLayer::Base];
	sprite.setTextureRect({3, 5, 10, 5});
	sprite.setOrigin({0.2f, 1.9f});
	auto& actor_ani = fix.animation_manager.query(id);
	actor_ani.light_intensity = 0.3f;
	actor_ani.has_changed = true;
	// update
	core::render_impl::updateObject(fix.context, actor_render);
	// assert rect and origin
	BOOST_CHECK_EQUAL(actor_render.light->intensity, 76u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(updateCameras_guarantees_right_number_of_culling_buffers) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	for (auto i = 0u; i < 3u; ++i) {
		fix.camera_system.acquire().objects.push_back(
			fix.add_object({}, {0, 1}));
	}
	// update cameras
	core::render_impl::updateCameras(fix.context, sf::milliseconds(50));
	// assert three culling buffers
	BOOST_CHECK_EQUAL(3u, fix.context.buffers.size());
}

BOOST_AUTO_TEST_CASE(updateCameras_fails_if_once_cam_has_no_objects) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto& cam1 = fix.camera_system.acquire();
	fix.camera_system.acquire();
	auto& cam3 = fix.camera_system.acquire();
	cam1.objects.push_back(fix.add_object({}, {0, 1}));
	cam3.objects.push_back(fix.add_object({}, {0, 1}));
	// expect failure
	BOOST_CHECK_ASSERT(
		core::render_impl::updateCameras(fix.context, sf::milliseconds(50)));
}

BOOST_AUTO_TEST_CASE(updateCameras_fails_if_once_cams_object_has_no_scene) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto& cam1 = fix.camera_system.acquire();
	auto& cam2 = fix.camera_system.acquire();
	auto& cam3 = fix.camera_system.acquire();
	cam1.objects.push_back(fix.add_object({}, {0, 1}));
	cam2.objects.push_back(fix.add_object({}, {0, 1}));
	cam3.objects.push_back(fix.add_object({}, {0, 1}));
	auto& move_data = fix.movement_manager.query(cam3.objects.front());
	move_data.scene = 0u;
	// expect failure
	BOOST_CHECK_ASSERT(
		core::render_impl::updateCameras(fix.context, sf::milliseconds(50)));
}

BOOST_AUTO_TEST_CASE(
	updateCameras_doesnt_fails_if_all_cams_have_valid_objects) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto& cam1 = fix.camera_system.acquire();
	auto& cam2 = fix.camera_system.acquire();
	auto& cam3 = fix.camera_system.acquire();
	cam1.objects.push_back(fix.add_object({}, {0, 1}));
	cam2.objects.push_back(fix.add_object({}, {0, 1}));
	cam3.objects.push_back(fix.add_object({}, {0, 1}));
	// expect normal execution
	BOOST_CHECK_NO_ASSERT(
		core::render_impl::updateCameras(fix.context, sf::milliseconds(50)));
}

BOOST_AUTO_TEST_CASE(updated_cam_pos_affected_by_their_objects) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto& cam1 = fix.camera_system.acquire();
	auto& cam2 = fix.camera_system.acquire();
	cam1.objects.push_back(fix.add_object({2u, 5u}, {}));
	cam1.objects.push_back(fix.add_object({1u, 3u}, {}));
	cam2.objects.push_back(fix.add_object({7u, 2u}, {}));
	// update cameras
	core::render_impl::updateCameras(fix.context, sf::milliseconds(50));
	// assert expected camera positions (in screen scale!)
	auto& dungeon = fix.dungeon_system[1];
	auto& pos1 = cam1.scene.getCenter();
	auto& pos2 = cam2.scene.getCenter();
	BOOST_CHECK_VECTOR_CLOSE(pos1, dungeon.toScreen({1.5f, 4.f}), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(pos2, dungeon.toScreen({7.f, 2.f}), 0.0001f);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(culling_makes_buffer_contain_all_visible_objects) {
	RenderFixture fix{{60u, 20u}};
	// prepare scene
	auto a = fix.add_object({15u, 12u}, {0, 1});
	fix.add_object({50u, 19u}, {0, 1});
	auto c = fix.add_object({12u, 13u}, {0, 1});
	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(1);
	auto cam = fix.camera_system.acquire();
	cam.objects.push_back(a);
	cam.scene.setCenter(dungeon.toScreen({15.f, 12.f}));
	// cull scene
	core::render_impl::cullScene(
		fix.context, fix.context.buffers[0], cam, dungeon);
	// expect objects a and c to be culled
	auto const& objects =
		fix.context.buffers[0].objects[core::ObjectLayer::Bottom];
	BOOST_REQUIRE_EQUAL(2u, objects.size());
	BOOST_CHECK_EQUAL(objects[0]->id, a);
	BOOST_CHECK_EQUAL(objects[1]->id, c);
}

BOOST_AUTO_TEST_CASE(culling_makes_buffer_contain_all_visible_ambiences) {
	RenderFixture fix{{60u, 20u}};
	// prepare scene
	auto a = fix.add_object({15u, 12u}, {0, 1});
	fix.add_object({50u, 19u}, {0, 1});
	auto& dungeon = fix.dungeon_system[1];
	auto& cell1 = dungeon.getCell({50u, 19u});
	cell1.ambiences.emplace_back();
	auto& cell2 = dungeon.getCell({12u, 13u});
	cell2.ambiences.emplace_back();
	auto& cell3 = dungeon.getCell({15u, 13u});
	cell3.ambiences.emplace_back();
	// prepare camera
	fix.context.buffers.resize(1);
	auto cam = fix.camera_system.acquire();
	cam.objects.push_back(a);
	cam.scene.setCenter(dungeon.toScreen({15.f, 12.f}));
	// cull scene
	core::render_impl::cullScene(fix.context, fix.context.buffers[0], cam, dungeon);
	// expect sprite from cell2 and cell3 to be culled
	auto const& ambiences = fix.context.buffers[0].ambiences;
	BOOST_REQUIRE_EQUAL(ambiences.size(), 2u);
	BOOST_CHECK(!utils::contains(ambiences, &cell1.ambiences.front()));
	BOOST_CHECK(utils::contains(ambiences, &cell2.ambiences.front()));
	BOOST_CHECK(utils::contains(ambiences, &cell3.ambiences.front()));
}

BOOST_AUTO_TEST_CASE(culling_makes_buffer_contain_all_visible_highlighting_sprites) {
	RenderFixture fix{{60u, 20u}};
	// prepare scene
	auto& a = fix.render_manager.query(fix.add_object({15u, 12u}, {0, 1}));
	auto& b = fix.render_manager.query(fix.add_object({50u, 19u}, {0, 1}));
	auto& c = fix.render_manager.query(fix.add_object({12u, 13u}, {0, 1}));
	a.highlight = std::make_unique<sf::Sprite>();
	b.highlight = std::make_unique<sf::Sprite>();
	c.highlight = std::make_unique<sf::Sprite>();
	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(1);
	auto cam = fix.camera_system.acquire();
	cam.objects.push_back(a.id);
	cam.scene.setCenter(dungeon.toScreen({15.f, 12.f}));
	// cull scene
	core::render_impl::cullScene(fix.context, fix.context.buffers[0], cam, dungeon);
	// expect objects a and c to be culled
	auto const& objects = fix.context.buffers[0].highlights;
	BOOST_REQUIRE_EQUAL(2u, objects.size());
	BOOST_CHECK_EQUAL(objects[0], a.highlight.get());
	BOOST_CHECK_EQUAL(objects[1], c.highlight.get());
}

BOOST_AUTO_TEST_CASE(culling_sorts_objects_by_its_layer) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	// prepare scene
	auto a = fix.add_object({5u, 7u}, {0, 1});
	auto b = fix.add_object({7u, 7u}, {0, 1});
	auto c = fix.add_object({8u, 8u}, {0, 1});
	fix.render_manager.query(a).layer = core::ObjectLayer::Middle;
	fix.render_manager.query(b).layer = core::ObjectLayer::Top;
	fix.render_manager.query(c).layer = core::ObjectLayer::Bottom;
	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(1);
	auto cam = fix.camera_system.acquire();
	cam.objects.push_back(a);
	// cull scene
	core::render_impl::cullScene(
		fix.context, fix.context.buffers[0], cam, dungeon);
	// expect objects sorted
	auto const& buffer = fix.context.buffers[0];
	BOOST_REQUIRE_EQUAL(buffer.objects[core::ObjectLayer::Bottom].size(), 1u);
	BOOST_REQUIRE_EQUAL(buffer.objects[core::ObjectLayer::Middle].size(), 1u);
	BOOST_REQUIRE_EQUAL(buffer.objects[core::ObjectLayer::Top].size(), 1u);
	BOOST_CHECK_EQUAL(buffer.objects[core::ObjectLayer::Bottom][0]->id, c);
	BOOST_CHECK_EQUAL(buffer.objects[core::ObjectLayer::Middle][0]->id, a);
	BOOST_CHECK_EQUAL(buffer.objects[core::ObjectLayer::Top][0]->id, b);
}

BOOST_AUTO_TEST_CASE(culling_always_clears_old_buffer_state) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	// prepare scene
	auto a = fix.add_object({5u, 2u}, {0, 1});
	auto b = fix.add_object({3u, 9u}, {0, 1});
	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(2);
	auto cam1 = fix.camera_system.acquire();
	auto cam2 = fix.camera_system.acquire();
	cam1.objects.push_back(a);
	cam1.scene.setCenter(dungeon.toScreen({5.f, 2.f}));
	cam2.objects.push_back(b);
	cam2.scene.setCenter(dungeon.toScreen({3.f, 9.f}));
	// cull scene
	core::render_impl::cullScene(
		fix.context, fix.context.buffers[0], cam1, dungeon);
	core::render_impl::cullScene(
		fix.context, fix.context.buffers[1], cam2, dungeon);
	// expect buffers to contain disjoint objects
	auto const& obj1 =
		fix.context.buffers[0].objects[core::ObjectLayer::Bottom];
	BOOST_REQUIRE_EQUAL(1u, obj1.size());
	BOOST_CHECK_EQUAL(obj1[0]->id, a);
	auto const& obj2 =
		fix.context.buffers[1].objects[core::ObjectLayer::Bottom];
	BOOST_REQUIRE_EQUAL(1u, obj2.size());
	BOOST_CHECK_EQUAL(obj2[0]->id, b);
}

BOOST_AUTO_TEST_CASE(culling_never_contains_grid_borders_if_debug_disabled) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(1);
	auto cam = fix.camera_system.acquire();
	cam.objects.push_back(fix.add_object({}, {0, 1}));
	// cull scene
	core::render_impl::cullScene(
		fix.context, fix.context.buffers[0], cam, dungeon);
	// assert empty grid borders
	BOOST_CHECK_EQUAL(0u, fix.context.buffers[0].grid.getVertexCount());
}

BOOST_AUTO_TEST_CASE(culling_always_contains_grid_borders_if_debug_enabled) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	fix.context.grid_color = sf::Color::Red;
	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(1);
	auto cam = fix.camera_system.acquire();
	cam.objects.push_back(fix.add_object({}, {0, 1}));
	// cull scene
	core::render_impl::cullScene(
		fix.context, fix.context.buffers[0], cam, dungeon);
	// assert empty grid borders
	BOOST_CHECK_NE(0u, fix.context.buffers[0].grid.getVertexCount());
}

BOOST_AUTO_TEST_CASE(
	culling_always_contains_edges_and_lights_borders_if_lighting_details_greater_zero) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	fix.lighting_system.setLevelOfDetails(1u);
	// prepare scene
	auto id = fix.add_object({}, {0, 1});
	auto& render_data = fix.render_manager.query(id);
	render_data.light = std::make_unique<utils::Light>();
	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(1);
	auto cam = fix.camera_system.acquire();
	cam.objects.push_back(id);
	// cull scene
	core::render_impl::cullScene(
		fix.context, fix.context.buffers[0], cam, dungeon);
	// assert edges and lights
	BOOST_CHECK(!fix.context.buffers[0].edges.empty());
	BOOST_CHECK(!fix.context.buffers[0].lights.empty());
}

BOOST_AUTO_TEST_CASE(
	culling_neither_contains_edges_nor_lights_borders_if_lighting_details_equal_zero) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	fix.lighting_system.setLevelOfDetails(0u);
	// prepare scene
	auto id = fix.add_object({}, {0, 1});
	auto& render_data = fix.render_manager.query(id);
	render_data.light = std::make_unique<utils::Light>();
	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(1);
	auto cam = fix.camera_system.acquire();
	cam.objects.push_back(id);
	// cull scene
	core::render_impl::cullScene(
		fix.context, fix.context.buffers[0], cam, dungeon);
	// assert neither edges nor lights
	BOOST_CHECK(fix.context.buffers[0].edges.empty());
	BOOST_CHECK(fix.context.buffers[0].lights.empty());
}

BOOST_AUTO_TEST_CASE(culling_can_handle_multiple_scenes_via_multiple_buffers) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	// prepare objects
	auto a = fix.add_object({5u, 2u}, {0, 1});
	auto b = fix.add_object({2u, 4u}, {0, 1});
	auto& dungeon = fix.dungeon_system[1];
	// prepare camera
	fix.context.buffers.resize(2);
	auto& cam1 = fix.camera_system.acquire();
	cam1.objects.push_back(a);
	cam1.scene.setCenter(dungeon.toScreen({5.f, 2.f}));
	auto& cam2 = fix.camera_system.acquire();
	cam2.objects.push_back(b);
	cam2.scene.setCenter(dungeon.toScreen({2.f, 4.f}));
	// cull all scenes at once
	core::render_impl::cullScenes(fix.context);
	// assert different buffers
	auto& obj1 = fix.context.buffers[0].objects[core::ObjectLayer::Bottom];
	auto& obj2 = fix.context.buffers[1].objects[core::ObjectLayer::Bottom];
	BOOST_CHECK(obj1 != obj2);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(leg_sprite_texture_can_be_changed_via_event) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 2u}, {0, 1});
	auto& data = fix.render_manager.query(id);
	core::render_impl::updateTexture(
		fix.context, data, core::SpriteLegLayer::Base, &fix.dummy_texture2);
	BOOST_CHECK(data.legs[core::SpriteLegLayer::Base].getTexture() ==
				&fix.dummy_texture2);
}

BOOST_AUTO_TEST_CASE(torso_sprite_texture_can_be_changed_via_event) {
	auto& fix = Singleton<RenderFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 2u}, {0, 1});
	auto& data = fix.render_manager.query(id);
	core::render_impl::updateTexture(
		fix.context, data, core::SpriteTorsoLayer::Weapon, &fix.dummy_texture2);
	BOOST_CHECK_EQUAL(data.torso[core::SpriteTorsoLayer::Weapon].getTexture(),
		&fix.dummy_texture2);
}

BOOST_AUTO_TEST_SUITE_END()
