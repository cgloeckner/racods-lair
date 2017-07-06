#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <engine/engine.hpp>

namespace engine {

class SaveManager {
  private:
	core::LogContext& log;
	rpg::Session const & session;
	std::mutex& mutex;
	std::atomic<bool> active;
	mutable std::atomic<unsigned int> elapsed;
	std::thread saver;
	
	struct Node {
		core::ObjectID id;
		game::PlayerTemplate data;
		std::string filename;
	};
	
	std::vector<Node> players;
	
  public:
	/// @param engine Const reference to session
	/// @param mutex Protects session
	SaveManager(core::LogContext& log, rpg::Session const & session, std::mutex& mutex);
	~SaveManager();
	
	void add(core::ObjectID id, game::PlayerTemplate data, std::string const & filename);
	void save();
	
	void start();
	bool isRunning() const;
	void stop();
	
	sf::Time getElapsedTime() const;
};

} // ::engine
