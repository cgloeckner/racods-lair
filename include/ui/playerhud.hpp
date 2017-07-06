#pragma once
#include <ui/statusbar.hpp>
#include <ui/notificationtext.hpp>

namespace ui {

class PlayerHud : public sf::Drawable, public sf::Transformable {

  private:
	sf::Vector2f offset;
	sf::Text name, focus_name, focus_level;
	StatsBar life, mana, stamina, focus_life;
	ExperienceBar exp;
	NotificationTexts notification;
	
	float padding, margin;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
	PlayerHud();
	
	void setup(sf::Font const & hud_font, unsigned int hud_size,
		sf::Font const & notify_font, unsigned int nofity_size, float padding,
		float margin, sf::Texture const & statbox_tex, sf::Texture const & statfill_tex,
		sf::Texture const & focusbox_tex, sf::Texture const & focusfill_tex);

	void resize(sf::Vector2u const& screen_size, std::size_t column);

	void setColor(sf::Color const & color);
	void setName(std::string const & display_name);
	void setExp(std::uint64_t value, std::uint64_t base, std::uint64_t next);
	void setLife(std::uint32_t value, std::uint32_t max);
	void setMana(std::uint32_t value, std::uint32_t max);
	void setStamina(std::uint32_t value, std::uint32_t max);
	void setFocus(std::string const & name, sf::Color const & color, std::uint32_t life=0u, std::uint32_t max_life=0u, std::uint32_t level=0u);
	
	void notify(std::string const & name, sf::Color const & color);
	
	void update(sf::Time const & elapsed);
};

}  // ::ui
