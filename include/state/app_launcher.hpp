#pragma once
#include <thread>
#include <atomic>
#include <Thor/Graphics.hpp>

#include <state/common.hpp>
#include <state/loadthread.hpp>

namespace state {

class AppLauncherState
	: public LoadThreadState {
  private:
	bool tools;
	sf::Text label;
	thor::ColorGradient gradient;
	float progress;
	
	void load() override;
	void postload() override;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	
  public:
	AppLauncherState(App& app, bool tools=false);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
