#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

#include <core/focus.hpp>

namespace core {

float const MAX_SIGHT = 25.f;

FocusSystem::FocusSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon,
	MovementManager const& movement_manager)
	// Event API
	: utils::EventListener<InputEvent, MoveEvent>{}
	, utils::EventSender<FocusEvent>{}  // Component API
	, FocusManager{max_objects}
	, context{log, *this, *this, dungeon, movement_manager} {}

void FocusSystem::handle(InputEvent const& event) {
	if (!has(event.actor)) {
		// object has no focus component
		// note: object might have been deleted
		return;
	}
	auto& data = query(event.actor);

	if (event.look.x != 0 || event.look.y != 0) {
		focus_impl::onLook(context, data, event);
	}
}

void FocusSystem::handle(MoveEvent const& event) {
	if (!has(event.actor)) {
		// object has no focus component
		// note: object might have been deleted
		return;
	}
	auto& data = query(event.actor);

	// note: on TileLeft and TileReached (e.g. after collision!)
	focus_impl::onMove(context, data, event);
}

void FocusSystem::update(sf::Time const& elapsed) {
	dispatch<MoveEvent>(*this);
	dispatch<InputEvent>(*this);

	propagate<FocusEvent>();
}

// ---------------------------------------------------------------------------

namespace focus_impl {

Context::Context(LogContext& log, FocusSender& focus_sender,
	FocusManager& focus_manager, DungeonSystem& dungeon_system,
	MovementManager const& movement_manager)
	: log{log}
	, focus_sender{focus_sender}
	, focus_manager{focus_manager}
	, dungeon_system{dungeon_system}
	, movement_manager{movement_manager} {}

// ---------------------------------------------------------------------------

ObjectID traverseCells(Dungeon const& dungeon, sf::Vector2u pos,
	sf::Vector2i const& dir, float depth,
	std::function<bool(ObjectID, int)> handle) {
	int step = 0;
	ObjectID found = 0u;
	do {
		try {
			auto const& cell = dungeon.getCell(pos);
			if (cell.terrain != core::Terrain::Floor) {
				// cannot look through non-floor terrain
				return found;
			}
			// search focusable object
			for (auto other : cell.entities) {
				if (handle(other, step)) {
					// found suitable object
					found = other;
					break;
				}
			}
		} catch (std::out_of_range const& e) {
			// out of grid
			break;
		}
		if (found > 0u) {
			// suitable object was already found!
			break;
		}
		// go to next tile in looking direction
		pos = sf::Vector2u{sf::Vector2i{pos} + dir};
		++step;
	} while (step <= depth);

	return found;
}

void setFocus(
	Context const& context, FocusData& observer, FocusData* observed) {
	// unfocus previously focused object
	if (observer.focus > 0u) {
		auto previous = observer.focus;
		if (context.focus_manager.has(observer.focus)) {
			auto& tmp = context.focus_manager.query(observer.focus);
			observer.focus = 0u;
			bool found = utils::pop(tmp.observers, observer.id);
			ASSERT(found);
		}
		// propagate, that observer lost its previous focus
		FocusEvent event;
		event.type = FocusEvent::Lost;
		event.observer = observer.id;
		event.observed = previous;
		context.focus_sender.send(event);
	}

	// focus next object
	if (observed != nullptr) {
		observer.focus = observed->id;
		observed->observers.push_back(observer.id);
		// propagate, that observer gained new focus
		FocusEvent event;
		event.type = FocusEvent::Gained;
		event.observer = observer.id;
		event.observed = observed->id;
		context.focus_sender.send(event);
	}
}

void onLook(Context const& context, FocusData& data, InputEvent const& event) {
	if (data.sight == 0.f || !data.is_active) {
		// object cannot focus
		return;
	}

	ASSERT(context.movement_manager.has(data.id));
	auto const& move_data = context.movement_manager.query(data.id);
	if (move_data.scene == 0u) {
		// object vanished, yet
		return;
	}
	auto const& dungeon = context.dungeon_system[move_data.scene];

	// search next focusable
	auto focus = traverseCells(dungeon, sf::Vector2u{move_data.target},
		event.look, data.sight, [&](ObjectID other, int) {
			if (other == data.id) {
				// shouldn't focus yourself
				return false;
			}
			if (!context.focus_manager.has(other)) {
				// object has cannot focus
				return false;
			}
			auto const& tmp = context.focus_manager.query(other);
			if (tmp.display_name.empty() || !tmp.is_active) {
				// object cannot be focused
				return false;
			}
			return true;
		});

	if (focus != data.focus) {
		if (focus > 0u) {
			setFocus(context, data, &context.focus_manager.query(focus));
		} else {
			setFocus(context, data, nullptr);
		}
	}

	// update actor's looking direction
	data.look = event.look;
	data.has_changed = true;
}

void onMove(Context& context, FocusData& data, MoveEvent const& event) {
	ASSERT(context.movement_manager.has(data.id));
	auto const& move_data = context.movement_manager.query(data.id);
	if (move_data.scene == 0u) {
		// object has vanished yet
		return;
	}
	auto const& dungeon = context.dungeon_system[move_data.scene];

	// renew focus
	core::InputEvent input;
	input.actor = data.id;
	input.move = {0, 0};  // not evaluated in this context
	input.look = data.look;
	onLook(context, data, input);
	// note: object update and event propagation are done by onLook

	// determine new observers
	auto old_observers = data.observers;
	std::vector<ObjectID> new_observers;
	sf::Vector2i dir;
	for (dir.y = -1; dir.y <= 1; ++dir.y) {
		for (dir.x = -1; dir.x <= 1; ++dir.x) {
			if (dir.x == 0 && dir.y == 0) {
				continue;
			}

			// track nearest observer (and maybe the actor's new focus)
			traverseCells(dungeon, move_data.target, dir, MAX_SIGHT,
				[&](ObjectID other, int distance) {
					if (other == data.id || !context.focus_manager.has(other)) {
						// shouldn't focus yourself - or an object that cannot
						// be focused
						return false;
					}
					auto& tmp = context.focus_manager.query(other);
					if (tmp.sight == 0.f || !tmp.is_active) {
						// object cannot focus
						return false;
					}
					if (tmp.look == -dir && distance <= tmp.sight) {
						// object is looking towards the actor
						new_observers.push_back(other);
					} // else: object is not looking towards actor
					// but blocks sight!
					return true;
				});
		}
	}

	// renew observers
	for (auto other : old_observers) {
		if (!utils::contains(new_observers, other)) {
			if (!context.focus_manager.has(other)) {
				// object was released
				continue;
			}
			auto& observer = context.focus_manager.query(other);
			// renew previous observer's focus
			core::InputEvent event;
			event.actor = other;
			event.move = {0, 0};  // not evaluated in this context
			event.look = observer.look;
			onLook(context, observer, event);
			// note: object update and event propagation are done by onLook
		}
	}
	for (auto other : new_observers) {
		if (!utils::contains(old_observers, other)) {
			auto& observer = context.focus_manager.query(other);
			setFocus(context, observer, &data);
		}
	}
}

}  // ::focus_impl

}  // ::core
