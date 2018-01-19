#include <utils/math2d.hpp>
#include <game/autocam.hpp>

namespace game {

namespace autocam_impl {

unsigned int const UPDATE_COOLDOWN = 250u;

Context::Context(core::LogContext& log, core::MovementManager const & movement,
	core::DungeonSystem const & dungeon, core::CameraSystem& camera)
	: log{log}
	, movement{movement}
	, dungeon{dungeon}
	, camera{camera} 
	, distance{7.f}
	, scenes{}
	, clusters{}
	, changed{false} {
	ASSERT(distance > 0.f);
}

bool onTeleport(Context& context, core::ObjectID actor) {
	bool changed{false};
	if (context.camera.has(actor)) {
		auto& cam = context.camera.query(actor);
		if (cam.objects.size() > 1u) {
			// split actor to extra cam
			auto& solo = context.camera.acquire();
			utils::pop(cam.objects, actor);
			solo.objects.push_back(actor);
			changed = true;
		}
	}
	
	if (changed) {
		refreshCameras(context);
	}
	
	return changed;
}

void exploreScenes(Context& context) {
	for (auto const & cam_ptr: context.camera) {
		for (auto id: cam_ptr->objects) {
			auto const & move = context.movement.query(id);
			ASSERT(move.scene > 0u);
			auto& objects = context.scenes[move.scene - 1u];
			objects.emplace_back();
			auto& data = objects.back();
			data.id = id;
			data.pos = move.pos;
			data.cam = cam_ptr.get();
		}
	}
}

void createClusters(Context& context, std::vector<Meta> const & data) {
	context.clusters.clear();
	auto const max_dist = context.distance * context.distance;
	
	Cluster remain, outside;
	remain.reserve(data.size());
	outside.reserve(data.size());
	for (auto const & node: data) {
		remain.push_back(&node);
	}
	
	while (!remain.empty()) {
		// create cluster and determine center
		context.clusters.emplace_back();
		auto& cluster = context.clusters.back();
		sf::Vector2f center = remain.front()->pos;
		
		// partition remaining objects to cluster or outside
		for (auto ptr: remain) {
			auto dist = utils::distance(center, ptr->pos);
			if (dist < max_dist) {
				cluster.push_back(ptr);
			} else {
				outside.push_back(ptr);
			}
		}
		remain = std::move(outside);
	}
}

void applyCluster(Context& context, Cluster const & cluster) {
	ASSERT(!cluster.empty());
	
	// determine cluster's camera
	auto cam = cluster.front()->cam;
	auto id = cluster.front()->id;
	
	// check whether any outsider is registered to this cam
	bool need_new{false};
	for (auto id: cam->objects) {
		if (utils::find_if(cluster, [&](Meta const * ptr) {
			return ptr->id == id;
		}) == cluster.end()) {
			need_new = true;
			break;
		}
	}
	
	if (need_new) {
		// create new camera
		cam = &context.camera.acquire();
	}
	
	// make sure everybody is registered to this camera
	for (auto ptr: cluster) {
		if (ptr->cam != cam) {
			// switch to cluster's camera
			context.camera.leave(*ptr->cam, ptr->id);
			cam->objects.push_back(ptr->id);
			context.changed = true;
			
			if (ptr->id == id) {
				context.log.debug << "[Game/AutoCam] Object " << ptr->id
					<< " got his own camera\n";
			} else {
				context.log.debug << "[Game/AutoCam] Object " << ptr->id
					<< " joined camera of " << id << "\n";
			}
		}
	}
}

void refreshCameras(Context& context) {
	// sort cameras to ensure objects being sorted by their IDs
	for (auto& uptr: context.camera) {
		std::sort(uptr->objects.begin(), uptr->objects.end());
	}
	
	// sort camera to make camera with first player be first camera
	std::sort(context.camera.begin(), context.camera.end(),
	[](std::unique_ptr<core::CameraData> const & lhs, std::unique_ptr<core::CameraData> const & rhs) {
		ASSERT(!lhs->objects.empty());
		ASSERT(!rhs->objects.empty());
		return lhs->objects.front() < rhs->objects.front();
	});
	
	// update camera sizes
	context.camera.resize(context.camera.getWindowSize());
}

void onUpdate(Context& context) {
	context.scenes.resize(context.dungeon.size());
	exploreScenes(context);
	for (auto const & scene: context.scenes) {
		createClusters(context, scene);
		for (auto& cluster: context.clusters) {
			applyCluster(context, cluster);
		}
	}
	context.scenes.clear();
	
	if (context.changed) {
		refreshCameras(context);
	}
}

} // ::autocam_impl

// --------------------------------------------------------------------

AutoCamSystem::AutoCamSystem(core::LogContext& log, core::MovementManager const & movement,
	core::DungeonSystem const & dungeon, core::CameraSystem& camera)
	: context{log, movement, dungeon, camera}
	, cooldown{sf::Time::Zero} {
}

void AutoCamSystem::setDistance(float distance) {
	ASSERT(distance > 0.f);
	context.distance = distance;
}

bool AutoCamSystem::onTeleport(core::ObjectID actor) {
	return autocam_impl::onTeleport(context, actor);
}

bool AutoCamSystem::update(sf::Time const & elapsed) {
	context.changed = false;
	
	cooldown -= elapsed;
	if (cooldown <= sf::Time::Zero) {
		cooldown = sf::milliseconds(autocam_impl::UPDATE_COOLDOWN);
		autocam_impl::onUpdate(context);
	}
	
	return context.changed;
}

} // ::engine

