#pragma once
#include <vector>
#include <memory>
#include <SFML/Audio/SoundBuffer.hpp>

#include <utils/animation_utils.hpp>
#include <utils/arcshape.hpp>
#include <utils/camera.hpp>
#include <utils/component_system.hpp>
#include <utils/enum_map.hpp>
#include <utils/keybinding.hpp>
#include <utils/layered_sprite.hpp>
#include <utils/lighting_system.hpp>
#include <utils/math2d.hpp>
#include <utils/spatial_scene.hpp>

#include <core/common.hpp>

namespace core {

// ---------------------------------------------------------------------------
// object components

struct CollisionData : ComponentData {
	bool is_projectile;
	utils::Collider shape;			// collision shape
	std::vector<ObjectID> ignore;	// ignored during collision
	
	mutable bool has_changed; // dirty flag for shape sprite

	CollisionData();
};

struct FocusData : ComponentData {
	std::string display_name;
	float sight;	// range of sight
	float fov;		// angle of fov
	bool is_active;

	mutable bool has_changed;  // dirty flag

	FocusData();
};

struct MovementData : ComponentData {
	mutable sf::Vector2f pos; // mutable to be reset by collision system
	sf::Vector2f last_pos, move, look;
	utils::SceneID scene;
	float max_speed;
	int num_speed_boni;		// negative for mali

	mutable bool has_changed;  // dirty flag

	MovementData();
};

struct AnimationData : ComponentData {
	using LegAnimation = utils::ActionFrames;
	using TorsoAnimation = utils::EnumMap<AnimationAction, utils::ActionFrames>;

	// animation templates with non-owning pointer inside
	struct {
		utils::EnumMap<SpriteLegLayer, LegAnimation const *> legs;
		utils::EnumMap<SpriteTorsoLayer, TorsoAnimation const *> torso;
	} tpl;

	utils::IntervalState brightness, alpha, min_saturation, max_saturation,
		light_intensity, light_radius;
	utils::ActionState legs, torso;  // current action states
	bool flying;
	AnimationAction current;  // current animation action

	mutable bool has_changed;  // dirty flag

	AnimationData();
};

struct RenderData : ComponentData {
	std::unique_ptr<sf::Sprite> highlight;
	utils::LayeredSprite<SpriteLegLayer> legs;
	utils::LayeredSprite<SpriteTorsoLayer> torso;
	std::unique_ptr<utils::Light> light;
	ObjectLayer layer;
	sf::Transform matrix; // sprite transformation
	float default_rotation; // referring sprite's direction
	std::vector<utils::Edge> edges;
	sf::Color blood_color;
	utils::ArcShape fov;
	std::unique_ptr<sf::Shape> shape;

	RenderData();
};

struct SoundData : ComponentData {
	utils::EnumMap<SoundAction, std::vector<sf::SoundBuffer const *>> sfx;

	SoundData();
};

// ---------------------------------------------------------------------------
// component managers

// general managers
template <typename T>
using ComponentManager = utils::ComponentSystem<ObjectID, T>;
using IdManager = utils::IdManager<ObjectID>;

// related to physics
using CollisionManager = ComponentManager<CollisionData>;
using FocusManager = ComponentManager<FocusData>;
using MovementManager = ComponentManager<MovementData>;

// related to graphics
using AnimationManager = ComponentManager<AnimationData>;
using RenderManager = ComponentManager<RenderData>;

// related to audio
using SoundManager = ComponentManager<SoundData>;

// ---------------------------------------------------------------------------
// other systems
using CameraData = utils::CameraData<ObjectID>;
using CameraSystem = utils::CameraSystem<ObjectID>;

}  // ::core
