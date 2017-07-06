#include <SFML/System.hpp>

#include <utils/filesystem.hpp>
#include <engine/savemanager.hpp>

namespace engine {

SaveManager::SaveManager(core::LogContext& log, rpg::Session const & session, std::mutex&  mutex)
	: log{log}
	, session{session}
	, mutex{mutex}
	, active{false}
	, elapsed{0u}
	, saver{}
	, players{} {
}

SaveManager::~SaveManager() {
	if (isRunning()) {
		stop();
	}
}

void SaveManager::add(core::ObjectID id, game::PlayerTemplate data, std::string const & filename) {
	players.emplace_back();
	auto& node = players.back();
	node.id = id;
	node.data = data;
	node.filename = filename;
}

void SaveManager::save() {
	sf::Clock clock;
	{
		// fetch all players' data
		std::lock_guard<std::mutex> lock{mutex};
		for (auto& node: players) {
			auto const & item = session.item.query(node.id);
			auto const & perk = session.perk.query(node.id);
			auto const & stats = session.stats.query(node.id);
			auto const & qslot = session.quickslot.query(node.id);
			auto const & player = session.player.query(node.id);
			
			node.data.fetch(item, perk, stats, qslot, player);
		}
		
		log.debug << "[Engine/SaveManager] Fetched data of "
			<< players.size() << " players\n";
	}
	
	// save all players' data
	for (auto const & node: players) {
		auto filename = get_preference_dir() + "saves/" + node.filename + ".sav";
		utils::rename_file(filename, filename + ".tmp");
		node.data.saveToFile(filename);
		utils::remove_file(filename + ".tmp");
	}
}

void SaveManager::start() {
	log.debug << "[Engine/SaveManager] started\n";
	saver = std::thread{[&]() {
		auto interval = sf::milliseconds(1000);
		active = true;
		while (active) {
			sf::Clock wait;
			while (wait.getElapsedTime() < interval) {
				sf::sleep(sf::milliseconds(20));
				if (!active) {
					return;
				}
			}
			sf::Clock clock;
			save();
			elapsed = clock.restart().asMilliseconds();
		}
	}};
}

bool SaveManager::isRunning() const {
	return active;
}

void SaveManager::stop() {
	active = false;
	try {
		saver.join();
	} catch (std::system_error const & err) {
	}
	log.debug << "[Engine/SaveManager] stopped\n";
}

sf::Time SaveManager::getElapsedTime() const {
	return sf::milliseconds(elapsed.exchange(0u));
}

} // ::engine
