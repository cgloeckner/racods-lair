#pragma once
#include <memory>
#include <mutex>

#include <utils/animation_utils.hpp>
#include <utils/states.hpp>
#include <core/common.hpp>
#include <game/resources.hpp>
#include <game/player.hpp>
#include <engine/engine.hpp>
#include <engine/savemanager.hpp>
#include <ui/button.hpp>
#include <ui/input.hpp>
#include <ui/checkbox.hpp>
#include <ui/select.hpp>
#include <state/resources.hpp>

namespace state {

extern unsigned int const MIN_SCREEN_WIDTH;
extern unsigned int const MIN_SCREEN_HEIGHT;

struct Context;
struct GameContext;
struct LobbyContext;
class State;

using App = utils::Application<Context>;

void apply(core::LogContext& log, sf::Window& window, state::Settings const & settings, unsigned int framelimit);

// --------------------------------------------------------------------

class State
	: public utils::State<Context> {
  protected:
	virtual void onResize(sf::Vector2u screen_size);
	
  public:
	State(App& app);
	
	void activate() override;
};

// --------------------------------------------------------------------

class SubState
	: public sf::Drawable {
  public:
	virtual bool handle(sf::Event const & event) = 0;
	virtual void update(sf::Time const & elapsed) = 0;
};

// --------------------------------------------------------------------

struct Context {
	App& app;
	sf::Sound sfx;
	sf::Music theme;
	core::LogContext log;
	game::ResourceCache cache;
	game::Mod mod;
	game::Localization locale;
	state::GlobalSettings globals;
	state::Settings settings;
	
	std::unique_ptr<GameContext> game;
	
	Context(App& app);
	
	void update(sf::Time const & elapsed);
	void onResize(sf::Vector2u screen_size);
	void drawBackground(sf::RenderTarget& target, sf::RenderStates states) const;
	
	// menu stuff
	sf::Sprite background;
};

// --------------------------------------------------------------------

struct LobbyContext {
	struct Player {
		std::string filename;
		rpg::Keybinding keys;
		game::PlayerTemplate tpl;
		core::ObjectID id;
		rpg::PlayerID player_id;
		bool use_gamepad;
		unsigned int gamepad_id;
		
		std::string getSavegameName() const;
		std::string getKeybindingName() const;
		
		Player();
	};
	
	unsigned int num_players, num_dungeons;
	sf::Vector2u dungeon_size;
	std::vector<Player> players;
	
	LobbyContext(std::size_t max_num_players);
	
	bool hasUnsetProfile(std::size_t& profile) const;
	bool hasInconsistentProfile(std::size_t& profile) const;
	bool hasDoubleUsedProfile(std::size_t& profile) const;
	bool hasAmbiguousInput(utils::InputAction& input, std::size_t& profile) const;
	bool hasSharedInput(utils::InputAction& input, std::size_t& lhs, std::size_t& rhs) const;
	bool hasSharedGamepad(unsigned int& pad_id) const;
};

// --------------------------------------------------------------------

struct GameContext {
	App& app;
	Context& parent;
	LobbyContext lobby;
	
	engine::Engine engine;
	std::mutex mutex;
	engine::SaveManager saver;
	
	/// Lobby is copied here!!
	GameContext(App& app, LobbyContext lobby);
	
	void update(sf::Time const & elapsed);
	void applySettings();
};

// --------------------------------------------------------------------

void setupTitle(sf::Text& label, std::string const & key, Context& context, std::string const & caption_ext="");
void setupLabel(sf::Text& label, std::string const & key, Context& context, std::string const & caption_ext="");
void setupWarning(sf::Text& label, Context& context);
void setupButton(ui::Button& button, std::string const & key, Context& context, std::string const & caption_ext="");
void setupSelect(ui::Select& select, Context& context);
void setupInput(ui::Input& input, std::string const & key, Context& context);
void setupCheckbox(ui::Checkbox& checkbox, std::string const & key, Context& context);

} // ::state
