#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

class SettingsState
	: public State {
  private:
	ui::Menu menu;
	sf::Sound sound; // preview
	sf::Text title_label, resolution_label, lighting_label, sound_label, music_label;
	Settings latest;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onSettingsChanged();
	void onResize(sf::Vector2u screen_size) override;
	void onSetResolution();
	void onSetFullscreen();
	void onSetAutoCam();
	void onSetAutoSave();
	void onSetLighting();
	void onSetSound();
	void onSetMusic();
	void onApplyClick();
	void onDiscardClick();
	
  public:
	SettingsState(App& app);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
