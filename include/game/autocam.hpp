#pragma once
#include <core/entity.hpp>
#include <core/dungeon.hpp>

namespace game {

namespace autocam_impl {

extern unsigned int const UPDATE_COOLDOWN; // in ms

struct Meta {
	core::ObjectID id;
	sf::Vector2f pos;
	core::CameraData* cam;
};

using Cluster = std::vector<Meta const *>;

struct Context {
	core::LogContext& log;
	core::MovementManager const & movement;
	core::DungeonSystem const & dungeon;
	core::CameraSystem& camera;
	
	float distance; // distance other players
	std::vector<std::vector<Meta>> scenes;
	std::vector<Cluster> clusters;
	bool changed;
	
	Context(core::LogContext& log, core::MovementManager const & movement,
		core::DungeonSystem const & dungeon, core::CameraSystem& camera);
};

bool onTeleport(Context& context, core::ObjectID actor);

void exploreScenes(Context& context);
void createClusters(Context& context, std::vector<Meta> const & data);
void applyCluster(Context& context, Cluster const & cluster);

void refreshCameras(Context& context);

void onUpdate(Context& context);

} // ::autocam_impl

// --------------------------------------------------------------------

class AutoCamSystem {
  private:
	autocam_impl::Context context;
	sf::Time cooldown;
	
  public:
	AutoCamSystem(core::LogContext& log, core::MovementManager const & movement,
		core::DungeonSystem const & dungeon, core::CameraSystem& camera);
	
	void setDistance(float distance);
	
	bool onTeleport(core::ObjectID actor);
	bool update(sf::Time const & elapsed);
};

} // ::engine
