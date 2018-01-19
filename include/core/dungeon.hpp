#pragma once
#include <memory>
#include <vector>

#include <utils/ortho_tile.hpp>
#include <utils/spatial_scene.hpp>

#include <core/common.hpp>

namespace core {

struct BaseTrigger {
	virtual ~BaseTrigger(){};

	virtual void execute(core::ObjectID actor) = 0;
	
	virtual bool isExpired() const = 0;
};

struct BaseCell {
	Terrain terrain;
	utils::OrthoTile tile;
	std::unique_ptr<BaseTrigger> trigger;
	std::vector<sf::Sprite> ambiences;

	BaseCell();
};

using DungeonCell = utils::SpatialCell<BaseCell, ObjectID>;
using Dungeon =
	utils::SpatialScene<BaseCell, ObjectID, utils::GridMode::Orthogonal>;

// ---------------------------------------------------------------------------

class DungeonSystem {
  private:
	using container = std::vector<std::unique_ptr<Dungeon>>;
	using const_iterator = container::const_iterator;
	
	container scenes;

  public:
	DungeonSystem();

	template <typename... Args>
	utils::SceneID create(Args&&... args);

	Dungeon& operator[](utils::SceneID scene_id);
	Dungeon const& operator[](utils::SceneID scene_id) const;
	
	const_iterator begin() const;
	const_iterator end() const;
	
	std::size_t size() const;
	
	void clear();
};

}  // ::core

// include implementation details
#include <core/dungeon.inl>
