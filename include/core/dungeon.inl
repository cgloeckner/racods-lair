#include <utils/algorithm.hpp>

namespace core {

template <typename... Args>
utils::SceneID DungeonSystem::create(Args&&... args) {
	utils::SceneID id = scenes.size() + 1u;
	auto uptr = std::make_unique<Dungeon>(id, std::forward<Args>(args)...);
	scenes.push_back(std::move(uptr));
	return id;
}

}  // ::core
