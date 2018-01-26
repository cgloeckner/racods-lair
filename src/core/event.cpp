#include <core/event.hpp>

namespace core {

InputEvent::InputEvent()
	: actor{0u}
	, move{}
	, look{} {
}

CollisionEvent::CollisionEvent()
	: actor{0u}
	, collider{0u}
	, pos{}
	, reset_to{}
	, interrupt{false} {
}

AnimationEvent::AnimationEvent()
	: actor{0u}
	, action{default_value<AnimationAction>()}
	, move{false}
	, force{false}
	, interval{0.f}
	, legs{nullptr}
	, leg_layer{default_value<SpriteLegLayer>()}
	, torso{nullptr}
	, torso_layer{default_value<SpriteTorsoLayer>()} {}

SpriteEvent::SpriteEvent()
	: actor{0u}
	, type{SpriteEvent::Legs}
	, leg_layer{}
	, torso_layer{}
	, texture{nullptr} {
}

SoundEvent::SoundEvent()
	: buffer{nullptr}
	, pitch{1.f}
	, relative_volume{1.f} {
}

MusicEvent::MusicEvent()
	: filename{} {
}

}  // ::core

namespace utils {

// ---------------------------------------------------------------------------
// Template instatiations

template class SingleEventSender<core::InputEvent>;
template class SingleEventSender<core::MoveEvent>;
template class SingleEventSender<core::FocusEvent>;
template class SingleEventSender<core::CollisionEvent>;
template class SingleEventSender<core::AnimationEvent>;
template class SingleEventSender<core::SpriteEvent>;
template class SingleEventSender<core::SoundEvent>;
template class SingleEventSender<core::MusicEvent>;

template class SingleEventListener<core::InputEvent>;
template class SingleEventListener<core::MoveEvent>;
template class SingleEventListener<core::FocusEvent>;
template class SingleEventListener<core::CollisionEvent>;
template class SingleEventListener<core::AnimationEvent>;
template class SingleEventListener<core::SpriteEvent>;
template class SingleEventListener<core::SoundEvent>;
template class SingleEventListener<core::MusicEvent>;

}  // ::utils
