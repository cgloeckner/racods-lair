#pragma once
#include <state/common.hpp>
#include <ui/common.hpp>

namespace state {

namespace char_impl {

struct StatsScreen
	: public sf::Drawable {
	
	using LabelPair = std::pair<sf::Text, sf::Text>;
	Context& context;
	
	LabelPair name, level, exp, next_exp, life, mana, stamina;
	utils::EnumMap<rpg::Attribute, LabelPair> attributes;
	utils::EnumMap<rpg::DamageType, LabelPair> damage, defense;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	StatsScreen(Context& context, core::ObjectID actor);
	
	void onResize(sf::Vector2u screen_size);
};

} // ::char_impl

// --------------------------------------------------------------------

class CharacterViewerState
	: public State {
  private:
	ui::Menu menu;
	sf::Text title_label;
	char_impl::StatsScreen stats;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	
	void onResize(sf::Vector2u screen_size) override;
	void onBackClick();
	
  public:
	CharacterViewerState(App& app, core::ObjectID actor);
	
	void handle(sf::Event const& event) override;
	void update(sf::Time const& elapsed) override;
};

} // ::state
