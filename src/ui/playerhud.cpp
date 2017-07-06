#include <iostream>
#include <utils/assert.hpp>
#include <ui/common.hpp>
#include <ui/playerhud.hpp>

namespace ui {

PlayerHud::PlayerHud()
	: offset{}
	, name{}
	, focus_name{}
	, life{}
	, mana{}
	, stamina{}
	, focus_life{true}
	, exp{}
	, notification{}
	, padding{5u}
	, margin{20u} {
	life.setFillColor(sf::Color::Red);
	mana.setFillColor(sf::Color::Blue);
	stamina.setFillColor(sf::Color::Yellow);
	focus_life.setFillColor(sf::Color::Red);
	exp.setOutlineColor({150, 150, 150});
	exp.setFillColor(sf::Color::White);
	exp.setOutlineThickness(padding);
}

void PlayerHud::setup(sf::Font const & hud_font, unsigned int hud_size,
	sf::Font const & notify_font, unsigned int nofity_size, float padding,
	float margin, sf::Texture const & statbox_tex, sf::Texture const & statfill_tex,
	sf::Texture const & focusbox_tex, sf::Texture const & focusfill_tex) {
	this->padding = padding;
	this->margin = margin;
	name.setFont(hud_font);
	name.setCharacterSize(hud_size);
	
	focus_name.setFont(hud_font);
	focus_name.setCharacterSize(hud_size);
	focus_level.setFont(hud_font);
	focus_level.setCharacterSize(hud_size);
	
	life.setBoxTexture(statbox_tex);
	life.setFillTexture(statfill_tex);
	mana.setBoxTexture(statbox_tex);
	mana.setFillTexture(statfill_tex);
	stamina.setBoxTexture(statbox_tex);
	stamina.setFillTexture(statfill_tex);
	
	focus_life.setBoxTexture(focusbox_tex);
	focus_life.setFillTexture(focusfill_tex);
	
	sf::Vector2f exp_size;
	exp_size.x = focusbox_tex.getSize().x + margin / 2.f;
	exp_size.y = padding;
	exp.setSize(sf::Vector2u{exp_size});
	exp.setOutlineThickness(3.f);
	
	notification.setup(notify_font, nofity_size);
}

void PlayerHud::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	target.draw(name, states);
	target.draw(life, states);
	target.draw(mana, states);
	//target.draw(stamina, states);
	target.draw(exp, states);
	
	if (!focus_name.getString().isEmpty()) {
		target.draw(focus_name, states);
		if (focus_life.isValid()) {
			target.draw(focus_level, states);
			target.draw(focus_life, states);
		}
	}
	
	target.draw(notification, states);
}

void PlayerHud::resize(sf::Vector2u const& screen_size, std::size_t column) {
	auto stat_size = life.getSize();
	auto focus_size = focus_life.getSize();
	auto exp_size = exp.getSize();
	auto rect = name.getLocalBounds();
	sf::Vector2f name_size{static_cast<float>(exp_size.x), rect.top + rect.height};
	sf::Vector2f focus_offset{exp_size.x / 2.f, focus_size.y / 2.f};
	
	offset.x = padding + column * (exp_size.x + padding);
	offset.y = screen_size.y - stat_size.y - exp_size.y - 3.f * padding - name_size.y;
	
	sf::Vector2f pos{offset};
	
	// place focus hud
	pos.y = padding;
	ui::setPosition(focus_name, pos + focus_offset);
	pos.y += name_size.y + padding;
	ui::setPosition(focus_level, pos + focus_offset);
	pos.y += name_size.y + padding;
	ui::setPosition(focus_life, pos + focus_offset);
	
	// place notifications
	pos = offset;
	pos.y -= 2 * padding;
	notification.setPosition(pos); // note: nodes are aligned internally
	
	// place own hud
	pos = offset;
	ui::setPosition(name, pos + name_size / 2.f);
	pos.x += padding;
	pos.y += name_size.y + padding;
	ui::setPosition(life, pos + sf::Vector2f{stat_size} / 2.f);
	pos.x += stat_size.x + padding;
	ui::setPosition(mana, pos + sf::Vector2f{stat_size} / 2.f);
	pos.x += stat_size.x + padding;
	ui::setPosition(stamina, pos + sf::Vector2f{stat_size} / 2.f);
	pos.x = offset.x;
	pos.y += stat_size.y + padding;
	ui::setPosition(exp, pos + sf::Vector2f{exp_size} / 2.f);
}

void PlayerHud::setColor(sf::Color const & color) {
	name.setColor(color);
}

void PlayerHud::setName(std::string const & display_name) {
	name.setString(display_name);
	ui::centerify(name);
}

void PlayerHud::setExp(std::uint64_t value, std::uint64_t base, std::uint64_t next) {
	if (value < base) {
		base = value;
	}
	if (next < value) {
		next = value;
	}
	exp.update(value-base, next-base);
}

void PlayerHud::setLife(std::uint32_t value, std::uint32_t max) {
	life.update(value, max);
}

void PlayerHud::setMana(std::uint32_t value, std::uint32_t max) {
	mana.update(value, max);
}

void PlayerHud::setStamina(std::uint32_t value, std::uint32_t max) {
	stamina.update(value, max);
}

void PlayerHud::setFocus(std::string const & name, sf::Color const & color, std::uint32_t life, std::uint32_t max_life, std::uint32_t level) {
	focus_name.setString(name);
	ui::centerify(focus_name);
	focus_level.setString("Lvl. " + std::to_string(level));
	ui::centerify(focus_level);
	focus_name.setColor(color);
	if (life == 0u) {
		// note: causes bar to disappear
		max_life = 0u;
	}
	focus_life.update(life, max_life);
}

void PlayerHud::notify(std::string const & name, sf::Color const & color) {
	notification.add(name, color, sf::seconds(4.f));
}

void PlayerHud::update(sf::Time const & elapsed) {
	notification.update(elapsed);
}

}  // ::ui
