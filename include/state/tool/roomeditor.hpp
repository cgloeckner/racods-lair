#pragma once
#include <memory>
#include <functional>

#include <engine/engine.hpp>

#include <ui/imgui.hpp>
#include <game/mod.hpp>
#include <state/common.hpp>

namespace tool {

using CellHandler = std::function<void(sf::Vector2u const &)>;

struct EngineState {
	state::App& app;
	game::ResourceCache cache;
	game::Mod mod;
	engine::Engine engine;
	
	game::RoomTemplate current_room;
	std::string current_name;
	bool changed;
	
	utils::SceneID scene;
	core::ObjectID viewer, mouse;
	sf::Texture empty_tex;
	rpg::SpriteTemplate sprite;
	
	EngineState(state::App& app, std::string const & mod_name);
	
	rpg::TilesetTemplate const & getTileset();
	void newRoom(std::string const & room_name);
	void loadRoom(std::string const & room_name);
	void saveRoom();
	
	void rebuild();
	void draw(sf::Vector2u const & pen, CellHandler handle);
	void setTerrain(sf::Vector2u const & pen, core::Terrain terrain);
	void setEntity(sf::Vector2u const & pen, std::string const & name,
		sf::Vector2i const & direction);
	
	void setLighting(bool lighting);
	void setShowGrid(bool show);
	void updateMouseLight();
	sf::Vector2f getWorldPos() const;
	void scroll(sf::Vector2i const & delta);
};

// --------------------------------------------------------------------

class RoomEditorState
	: public state::State {
  protected:
	std::unique_ptr<EngineState> engine;
	
	std::string next_popup, new_modpath, new_filename, load_modpath;
	int load_filename_index, entity_filename_index;
	std::vector<std::string> load_filename, entity_filename;
	int edit_mode;
	int edit_pen[2], entity_direction[2];
	sf::RectangleShape pen;
	boost::optional<sf::Vector2u> last_pos;
	bool lighting, show_grid;
	
	bool hasMap() const;
	
	void onLoadMod(std::string const & mod_name);
	void onPenResize();
	void onPenMove();
	void onModeSet();
	void onEntitySelect();
	
	void onNewClick();
	void onLoadClick();
	void onSaveClick();
	void onQuitClick();
	void onMouseClick(sf::Mouse::Button button);
	void onPlaceTileClick();
	void onRemoveTileClick();
	void onPlaceEntityClick();
	void onRemoveEntityClick();
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
  public:
	RoomEditorState(state::App& app);
	
	void handle(sf::Event const & event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::tool
