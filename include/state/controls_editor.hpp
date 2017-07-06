#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>
#include <ui/notificationtext.hpp>

namespace state {

class ControlsEditorState
	: public State {
  private:
	LobbyContext::Player& player;
	rpg::Keybinding keys;
	
	ui::Menu menu;
	sf::Text title_label;
	ui::NotificationNode warning_label;
	utils::EnumMap<rpg::PlayerAction, sf::Text> bind_labels;
	std::unique_ptr<rpg::PlayerAction> wait_for;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onJoystickConnected(unsigned int id);
	void onJoystickDisconnected(unsigned int id);
	void onResize(sf::Vector2u screen_size) override;
	void onSelectDevice();
	void onWaitBinding(rpg::PlayerAction action);
	void onUpdateBinding(utils::InputAction action);
	void onResetClick();
	void onSaveClick();
	void onBackClick();
	
	void refreshButtons();
	
	void setWarning(std::string const & msg);
	
  public:
	ControlsEditorState(App& app, LobbyContext::Player& player);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
