#include <utils/enum_utils.hpp>
#include <core/entity.hpp>

namespace core {

ComponentData::ComponentData() : id{0u} {}

// ---------------------------------------------------------------------------

CollisionData::CollisionData()
	: ComponentData{}, is_projectile{false}, radius{0.f} {}

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
	, scene{0u}
	, max_speed{0.f}
	, target{}
	, move{}
	, look{0, 1}
	, next_move{}
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
	, is_moving{false}
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
	, legs_matrix{sf::Transform::Identity}
	, torso_matrix{sf::Transform::Identity}
	, edges{}
	, blood_color{sf::Color::Transparent} {}

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
