#pragma once
#include <cstdint>
#include <vector>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <utils/logger.hpp>
#include <utils/enum_utils.hpp>
#include <utils/enum_map.hpp>

namespace core {

using ObjectID = std::uint16_t;

// ---------------------------------------------------------------------------

extern float const MAX_COLLISION_RADIUS; // in tiles
extern float const MAX_FRAMETIME_MS;     // maxused per physics update
extern float const MAX_SPEED;

struct ComponentData {
	ObjectID id;

	ComponentData();
};

// ---------------------------------------------------------------------------
// enumerations

// action that can be performed by each animated gameobject
// those values are created and propagated by the avatar system
ENUM(AnimationAction, Idle, (Idle)(Melee)(Range)(Magic)(Use)(Die))

// sprite layers (in rendering order)
// LegsBase and LegsArmor are always supposted to be synchronous
// TorsoBase, TorsoArmor and Weapon are always supposted to be synchronous
ENUM(SpriteLegLayer, Base, (Base)(Armor))

ENUM(SpriteTorsoLayer, Weapon, (Weapon)(Shield)(Base)(Helmet)(Armor))

// tile terrains
ENUM(Terrain, Void, (Void)(Wall)(Floor))

// object layer (for sprites)
ENUM(ObjectLayer, Bottom, (Bottom)(Middle)(Top))

// used by objects and items
ENUM(SoundAction, Spawn, (Spawn)(Move)(Item)(Perk)(Attack)(Hit)(Death))

// ---------------------------------------------------------------------------

struct LogContext {
	utils::Logger warning, error, debug;
};

}  // ::core

ENUM_STREAM(core::AnimationAction);
ENUM_STREAM(core::SpriteLegLayer);
ENUM_STREAM(core::SpriteTorsoLayer);
ENUM_STREAM(core::Terrain);
ENUM_STREAM(core::ObjectLayer);
ENUM_STREAM(core::SoundAction);

SET_ENUM_LIMITS(core::AnimationAction::Idle, core::AnimationAction::Die)
SET_ENUM_LIMITS(core::SpriteLegLayer::Base, core::SpriteLegLayer::Armor)
SET_ENUM_LIMITS(core::SpriteTorsoLayer::Weapon, core::SpriteTorsoLayer::Armor)
SET_ENUM_LIMITS(core::Terrain::Void, core::Terrain::Floor)
SET_ENUM_LIMITS(core::ObjectLayer::Bottom, core::ObjectLayer::Top)
SET_ENUM_LIMITS(core::SoundAction::Spawn, core::SoundAction::Death)
