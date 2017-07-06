#pragma once
#include <iostream>
#include <vector>

#include <game/resources.hpp>
#include <game/mod.hpp>

namespace game {

PlayerTemplate createPlayer(std::string const & charname, std::uint32_t level);

/// Load a player profile from a savegame file
/// The player profile is loaded into the given template using the
/// given mod context. It is specified by the provided filename.
/// Whether loading was successfull can be checked by observing the
/// returning boolean. Related items and perks are loaded as well if
/// possible. The profile itself is not loaded to the mod
/// @param profile Reference to the outgoing profile template
/// @param mod Reference to mod context used to load additional resources
/// @param fname Filename of the player savegame
/// @return true if player was successfully loaded
bool preparePlayer(PlayerTemplate& profile, Mod& mod, std::string const& fname);

}  // ::rage
