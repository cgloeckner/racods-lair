#pragma once
#include <rpg/event.hpp>

namespace game {

struct PathFailedEvent {
	core::ObjectID actor;
	sf::Vector2u pos;

	PathFailedEvent();
};

struct PowerupEvent {
	core::ObjectID actor;
	utils::EnumMap<rpg::Stat, int> delta;
};

struct ReleaseEvent {
	core::ObjectID actor;
};

// ---------------------------------------------------------------------------
// event sender and listener

using PathFailedSender = utils::SingleEventSender<PathFailedEvent>;
using PowerupSender = utils::SingleEventSender<PowerupEvent>;
using ReleaseSender = utils::SingleEventSender<ReleaseEvent>;

using PathFailedListener = utils::SingleEventListener<PathFailedEvent>;
using PowerupListener = utils::SingleEventListener<PowerupEvent>;
using ReleaseListener = utils::SingleEventListener<ReleaseEvent>;

}  // ::engine
