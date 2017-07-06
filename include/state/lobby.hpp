#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>
#include <ui/notificationtext.hpp>

namespace state {

class LobbyState
	: public State {
  private:
	std::size_t const START, BACK;
	
	ui::Menu menu;
	std::vector<std::string> available_profiles;
	sf::Text title_label, num_players_label, num_dungeons_label, difficulty_label;
	std::vector<sf::Text> player_labels, device_labels;
	ui::NotificationNode warning_label;
	
	LobbyContext lobby;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onJoystickDisconnected(std::size_t id);
	void onSetDifficulty();
	void onSetNumPlayers();
	void onUpdateSelection(std::size_t i);
	void onEditProfile(std::size_t n);
	void onStartClick();
	void onBackClick();
	
	void setWarning(std::string const & msg);
	
  public:
	LobbyState(App& app);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
	void activate() override;
};

} // ::state
