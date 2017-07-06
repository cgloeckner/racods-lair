#include <game/entity.hpp>

namespace game {

HudData::HudData() : core::ComponentData{}, hud{nullptr} {}

ScriptData::ScriptData()
	: core::ComponentData{}, is_active{true}, api{nullptr}, script{nullptr} {}

}  // ::rage

// ---------------------------------------------------------------------------
// Template instatiations

namespace utils {

template class ComponentSystem<core::ObjectID, game::HudData>;
template class ComponentSystem<core::ObjectID, game::ScriptData>;

}  // ::utils
