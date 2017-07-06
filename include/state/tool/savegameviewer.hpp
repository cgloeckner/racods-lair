#pragma once
#include <memory>

#include <game/resources.hpp>
#include <state/common.hpp>

namespace tool {

struct SavegameState {
	game::Mod& mod;
	std::string const filename;
	game::PlayerTemplate player;
	
	SavegameState(game::Mod& mod, std::string const & filename);
	
	void update();
};

// --------------------------------------------------------------------

class SavegameViewerState
	: public state::State {
  private:
	core::LogContext log;
	game::ResourceCache cache;
	std::string modpath;
	int index;
	std::vector<std::string> profiles;
	
	std::unique_ptr<game::Mod> mod;
	std::unique_ptr<SavegameState> current;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onModSpecify();
	void onProfileSelect();
	void onBackClick();

  public:
	SavegameViewerState(state::App& app);
	
	void handle(sf::Event const & event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::tool
