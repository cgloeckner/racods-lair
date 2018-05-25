#pragma once
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <utils/animation_utils.hpp>
#include <utils/event_system.hpp>
#include <utils/spatial_scene.hpp>

#include <core/common.hpp>

namespace core {

// ---------------------------------------------------------------------------
// event data

struct InputEvent {
	ObjectID actor;
	sf::Vector2f move, look;
	
	InputEvent();
};

struct MoveEvent {
	ObjectID actor;
	enum { Start, Stop } type;
};

struct CollisionEvent {
	ObjectID actor, collider;
	bool interrupt;
	
	CollisionEvent();
};

struct AnimationEvent {
	using LegAnimation = utils::ActionFrames;
	using TorsoAnimation = utils::EnumMap<AnimationAction, utils::ActionFrames>;

	ObjectID actor;
	enum {
		Action,
		Brightness,
		Alpha,
		LightIntensity,
		LightRadius,
		MinSaturation,
		MaxSaturation,
		Legs,
		Torso
	} type;

	AnimationAction action;			// ::Action
	bool move, force;				// ::Move
	utils::IntervalState interval;  // ::Brightness, ::Alpha, ::MinSaturation,
									// ::MaxSaturation, ::LightIntensity, ::LightRadius
	LegAnimation const* legs;		// ::Legs
	SpriteLegLayer leg_layer;		// ::Legs
	TorsoAnimation const* torso;	// ::Torso
	SpriteTorsoLayer torso_layer;   // ::Torso

	AnimationEvent();
};

struct SpriteEvent {
	ObjectID actor;
	enum { Legs, Torso } type;
	SpriteLegLayer leg_layer;
	SpriteTorsoLayer torso_layer;
	sf::Texture const* texture;
	
	SpriteEvent();
};

struct SoundEvent {
	sf::SoundBuffer const * buffer;
	float pitch, relative_volume;

	SoundEvent();
};

struct MusicEvent {
	std::string filename;
	
	MusicEvent();
};

struct TeleportEvent {
	ObjectID actor;
	utils::SceneID src_scene, dst_scene;
	sf::Vector2u src_pos, dst_pos;
};

// ---------------------------------------------------------------------------
// event sender and listener

using InputSender = utils::SingleEventSender<InputEvent>;
using MoveSender = utils::SingleEventSender<MoveEvent>;
using CollisionSender = utils::SingleEventSender<CollisionEvent>;
using AnimationSender = utils::SingleEventSender<AnimationEvent>;
using SpriteSender = utils::SingleEventSender<SpriteEvent>;
using SoundSender = utils::SingleEventSender<SoundEvent>;
using MusicSender = utils::SingleEventSender<MusicEvent>;
using TeleportSender = utils::SingleEventSender<TeleportEvent>;

using InputListener = utils::SingleEventListener<InputEvent>;
using MoveListener = utils::SingleEventListener<MoveEvent>;
using CollisionListener = utils::SingleEventListener<CollisionEvent>;
using AnimationListener = utils::SingleEventListener<AnimationEvent>;
using SpriteListener = utils::SingleEventListener<SpriteEvent>;
using SoundListener = utils::SingleEventListener<SoundEvent>;
using MusicListener = utils::SingleEventListener<MusicEvent>;
using TeleportListener = utils::SingleEventListener<TeleportEvent>;

}  // ::core
