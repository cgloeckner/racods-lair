#include <game/entity.hpp>

namespace game {

TracerData::TracerData()
	: core::ComponentData{}
	, request{}
	, path{}
	, is_enabled{true} {
}

HudData::HudData() : core::ComponentData{}, hud{nullptr} {}

ScriptData::ScriptData()
	: core::ComponentData{}, is_active{true}, /*api{nullptr},*/ script{nullptr} {}

} // ::game

// ---------------------------------------------------------------------------
// Template instatiations

namespace utils {

template class ComponentSystem<core::ObjectID, game::TracerData>;
template class ComponentSystem<core::ObjectID, game::HudData>;
template class ComponentSystem<core::ObjectID, game::ScriptData>;

}  // ::utils
