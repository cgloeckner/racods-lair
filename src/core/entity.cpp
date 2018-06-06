#include <utils/enum_utils.hpp>
#include <core/entity.hpp>

namespace core {

ComponentData::ComponentData() : id{0u} {}

// ---------------------------------------------------------------------------

CollisionData::CollisionData()
	: ComponentData{}
	, is_projectile{false}
	, shape{}
	, ignore{}
	, has_changed{false} {}

FocusData::FocusData()
	: ComponentData{}
	, display_name{}
	, sight{0.f}
	, fov{0.f}
	, is_active{true}
	, has_changed{true} {}

MovementData::MovementData()
	: ComponentData{}
	, pos{}
	, last_pos{}
	, move{}
	, scene{0u}
	, max_speed{0.f}
	, num_speed_boni{0}
	, has_changed{true} {}

AnimationData::AnimationData()
	: ComponentData{}
	, tpl{}
	, brightness{1.f}
	, alpha{1.f}
	, min_saturation{0.f}
	, max_saturation{1.f}
	, light_intensity{0.5f}
	, light_radius{1.f}
	, legs{}
	, torso{}
	, flying{false}
	, current{default_value<AnimationAction>()}
	, has_changed{true} {
	// reset templates
	for (auto& pair : tpl.legs) {
		pair.second = nullptr;
	}
	for (auto& pair : tpl.torso) {
		pair.second = nullptr;
	}
}

RenderData::RenderData()
	: ComponentData{}
	, highlight{nullptr}
	, legs{}
	, torso{}
	, light{nullptr}
	, layer{default_value<ObjectLayer>()}
	, matrix{sf::Transform::Identity}
	, default_rotation{0.f}
	, edges{}
	, blood_color{sf::Color::Transparent}
	, fov{} {}

SoundData::SoundData() : ComponentData{}, sfx{} {}

}  // ::core

// ---------------------------------------------------------------------------
// Template instatiations

namespace utils {

template class IdManager<core::ObjectID>;

template class ComponentSystem<core::ObjectID, core::CollisionData>;
template class ComponentSystem<core::ObjectID, core::FocusData>;
template class ComponentSystem<core::ObjectID, core::MovementData>;
template class ComponentSystem<core::ObjectID, core::AnimationData>;
template class ComponentSystem<core::ObjectID, core::RenderData>;
template class ComponentSystem<core::ObjectID, core::SoundData>;

template struct CameraData<core::ObjectID>;
template class CameraSystem<core::ObjectID>;

}  // ::utils
