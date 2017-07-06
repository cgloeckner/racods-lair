#include <state/app_launcher.hpp>
#include <state/title.hpp>

namespace state {

LoadThreadState::LoadThreadState(App& app)
	: State{app}
	, loader{}
	, loaded{false}
	, finished{false} {
	// note: cannot start thread here due to virtual function load()
}

LoadThreadState::~LoadThreadState() {
	try {
		loader.join();
	} catch (std::system_error const & err) {
	}
}

void LoadThreadState::start() {
	loader = std::thread{[&]() { load(); loaded = true; }};
}

void LoadThreadState::update(sf::Time const& elapsed) {
	if (finished) {
		quit();
		return;
	}
	if (loaded) {
		finished = true;
		postload();
		return;
	}
}

} // ::state
