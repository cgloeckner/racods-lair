#include <core/common.hpp>

namespace core {

float constexpr MAX_COLLISION_RADIUS = 1.f;
float constexpr MAX_FRAMETIME_MS     = 10.f;

// -----------------------------------------------------------

float constexpr MAX_DISTANCE = 2 * MAX_COLLISION_RADIUS;  // because two entities are considered
float constexpr MAX_TIME     = MAX_FRAMETIME_MS / 1000.f; // in seconds

/// Maximum entity speed (tiles per second)
/// The max collision radius determines the number of tiles that are checked within
/// a single update. The max frametime determines the elapsed time since the last update
/// Basically v_max ~= 2 * r_max / t_max
/// MAX_DISTANCE / MAX_TIME returns how many tiles (mostly < 1.f) can be crossed during a frame
float constexpr MAX_SPEED = MAX_DISTANCE / MAX_TIME;

static_assert(MAX_SPEED > 0.f, "Either MAX_COLLISION_RADIUS was set too low or MAX_FRAMETIME_MS was set too high at core/common.hpp");

}  // ::core
