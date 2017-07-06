#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>
#include <ui/notificationtext.hpp>

namespace state {

class ProfileCreatorState
	: public State {
  private:
	LobbyContext::Player& player;
	ui::Menu menu;
	sf::Text title_label, filename_label, charname_label;
	ui::NotificationNode warning_label;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onCreateClick();
	void onBackClick();
	
	void setWarning(std::string const & msg);
	
  public:
	ProfileCreatorState(App& app, LobbyContext::Player& player);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
