#include <utils/assert.hpp>

#include <core/dungeon.hpp>

namespace core {

BaseCell::BaseCell()
	: terrain{Terrain::Void}
	, tile{}
	, trigger{nullptr}
	, ambiences{} {
}

// ---------------------------------------------------------------------------

DungeonSystem::DungeonSystem() : scenes{} {}

Dungeon& DungeonSystem::operator[](utils::SceneID scene_id) {
	ASSERT(scene_id > 0u);
	ASSERT(scene_id <= scenes.size());
	auto& ptr = scenes[scene_id - 1u];
	ASSERT(ptr != nullptr);
	return *ptr;
}

Dungeon const& DungeonSystem::operator[](utils::SceneID scene_id) const {
	ASSERT(scene_id > 0u);
	ASSERT(scene_id <= scenes.size());
	auto const & ptr = scenes[scene_id - 1u];
	ASSERT(ptr != nullptr);
	return *ptr;
}

DungeonSystem::container::const_iterator DungeonSystem::begin() const {
	return scenes.begin();
}

DungeonSystem::container::const_iterator DungeonSystem::end() const {
	return scenes.end();
}

std::size_t DungeonSystem::size() const {
	return scenes.size();
}

void DungeonSystem::clear() {
	scenes.clear();
}

}  // ::core
