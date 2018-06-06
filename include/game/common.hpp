#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <utils/spatial_scene.hpp>
#include <core/common.hpp>

namespace game {

/// fading delay (ms) before object release
unsigned int const FADE_DELAY = 1000;

// used for pathfinding
ENUM(PathPhase, Broad, (Broad)(Narrow))

// used by path tracer
ENUM(TraceState, Idle, (Idle)(Trigger)(Wait)(Trace))

using Path = std::vector<sf::Vector2u>;

}  // ::game

ENUM_STREAM(game::PathPhase)
ENUM_STREAM(game::TraceState)

SET_ENUM_LIMITS(game::PathPhase::Broad, game::PathPhase::Narrow)
SET_ENUM_LIMITS(game::TraceState::Idle, game::TraceState::Trace)
