#include <ui/button.hpp>

#include <rpg/combat.hpp>
#include <state/character_viewer.hpp>

namespace state {

namespace char_impl {

StatsScreen::StatsScreen(Context& context, core::ObjectID actor)
	: context{context}
	, name{}
	, level{}
	, exp{}
	, next_exp{}
	, life{}
	, mana{}
	, stamina{}
	, attributes{}
	, damage{}
	, defense{} {
	ASSERT(actor > 0u);
	ASSERT(context.game->engine.session.stats.has(actor));
	ASSERT(context.game->engine.session.player.has(actor));
	ASSERT(context.game->engine.session.focus.has(actor));
	auto const & stat_data = context.game->engine.session.stats.query(actor);
	auto const & item_data = context.game->engine.session.item.query(actor);
	auto const & player_data = context.game->engine.session.player.query(actor);
	auto display_name = context.game->engine.session.focus.query(actor).display_name;
	auto pairSet = [&](LabelPair& pair, std::string const & lhs, std::string const & rhs) {
		setupLabel(pair.first, "", context, lhs);
		setupLabel(pair.second, "", context, rhs);
	};
	
	// update stat labels
	pairSet(name, context.locale("general.charname"), display_name);
	auto statSet = [&](LabelPair& pair, std::string const & key, std::uint64_t min, std::uint64_t max) {
		pairSet(pair, context.locale(key), std::to_string(min) + " / " + std::to_string(max));
	};
	pairSet(exp, context.locale("general.experience"), std::to_string(player_data.exp));
	pairSet(next_exp, context.locale("general.next_exp"), std::to_string(player_data.next_exp));
	pairSet(level, context.locale("general.level"), std::to_string(stat_data.level));
	statSet(life, "stat.Life", stat_data.stats[rpg::Stat::Life], stat_data.properties[rpg::Property::MaxLife]);
	statSet(mana, "stat.Mana", stat_data.stats[rpg::Stat::Mana], stat_data.properties[rpg::Property::MaxMana]);
	statSet(stamina, "stat.Stamina", stat_data.stats[rpg::Stat::Stamina], stat_data.properties[rpg::Property::MaxStamina]);
	
	// update attribute labels
	for (auto& pair: attributes) {
		auto lhs = context.locale("attribute." + rpg::to_string(pair.first));
		auto rhs = std::to_string(stat_data.attributes[pair.first]);
		pairSet(pair.second, lhs, rhs);
	}
	
	// update damage labels
	auto dmg_val = rpg::combat_impl::getWeaponDamage(stat_data, item_data.equipment[rpg::EquipmentSlot::Weapon], nullptr);
	for (auto& pair: damage) {
		auto lhs = context.locale("damage_type." + rpg::to_string(pair.first));
		auto rhs = std::to_string(dmg_val[pair.first]);
		pairSet(pair.second, lhs, rhs);
	}
	
	// update defense labels
	auto def_val = rpg::combat_impl::getDefense(stat_data);
	for (auto& pair: defense) {
		auto lhs = context.locale("defense_type." + rpg::to_string(pair.first));
		auto rhs = std::to_string(def_val[pair.first]);
		pairSet(pair.second, lhs, rhs);
	}
}

void StatsScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	auto drawPair = [&](LabelPair const & pair) {
		target.draw(pair.first, states);
		target.draw(pair.second, states);
	};
	drawPair(name);
	drawPair(level);
	drawPair(exp);
	drawPair(next_exp);
	drawPair(life);
	drawPair(mana);
	//drawPair(stamina);
	for (auto& pair: damage) {
		drawPair(pair.second);
	}
	for (auto& pair: defense) {
		drawPair(pair.second);
	}
}

void StatsScreen::onResize(sf::Vector2u screen_size) {
	auto pad = context.globals.vertical_padding;
	auto hpad = context.globals.horizontal_padding;
	
	auto start_y = 100.f + 2.f * pad;
	auto center_x = screen_size.x / 2.f;
	
	ui::setPosition(name.first, {center_x - 1.f * hpad, start_y});
	ui::setPosition(name.second, {center_x - 0.5f * hpad, start_y});
	ui::setPosition(level.first, {center_x - 1.f * hpad, start_y + pad});
	ui::setPosition(level.second, {center_x - 0.5f * hpad, start_y + pad});
	ui::setPosition(exp.first, {center_x - 1.f * hpad, start_y + 2.f * pad});
	ui::setPosition(exp.second, {center_x - 0.5f * hpad, start_y + 2.f * pad});
	ui::setPosition(next_exp.first, {center_x - 1.f * hpad, start_y + 3.f * pad});
	ui::setPosition(next_exp.second, {center_x - 0.5f * hpad, start_y + 3.f * pad});
	
	ui::setPosition(life.first, {center_x + 0.5f * hpad, start_y});
	ui::setPosition(life.second, {center_x + 1.f * hpad, start_y});
	ui::setPosition(mana.first, {center_x + 0.5f * hpad, start_y + pad});
	ui::setPosition(mana.second, {center_x + 1.f * hpad, start_y + pad});
	ui::setPosition(stamina.first, {center_x + 0.5f * hpad, start_y + 2.f * pad});
	ui::setPosition(stamina.second, {center_x + 1.f * hpad, start_y + 2.f * pad});
	
	float offset{5.f};
	std::size_t i{0u};
	for (auto& pair: defense) {
		if (i % 2u == 0u) {
			ui::setPosition(pair.second.first, {center_x - 1.f * hpad, start_y + offset * pad});
			ui::setPosition(pair.second.second, {center_x - 0.5f * hpad, start_y + offset * pad});
		} else {
			ui::setPosition(pair.second.first, {center_x + 0.5f * hpad, start_y + offset * pad});
			ui::setPosition(pair.second.second, {center_x + 1.f * hpad, start_y + offset * pad});
			offset += 1.f;
		}
		++i;
	}
	offset += 1.f;
	
	i = 0u;
	for (auto& pair: damage) {
		if (i % 2u == 0u) {
			ui::setPosition(pair.second.first, {center_x - 1.f * hpad, start_y + offset * pad});
			ui::setPosition(pair.second.second, {center_x - 0.5f * hpad, start_y + offset * pad});
		} else {
			ui::setPosition(pair.second.first, {center_x + 0.5f * hpad, start_y + offset * pad});
			ui::setPosition(pair.second.second, {center_x + 1.f * hpad, start_y + offset * pad});
			offset += 1.f;
		}
		++i;
	}
}

} // ::char_impl

// --------------------------------------------------------------------

static std::size_t const BACK = 0u;

// --------------------------------------------------------------------

CharacterViewerState::CharacterViewerState(App& app, core::ObjectID actor)
	: State{app}
	, menu{}
	, title_label{}
	, stats{app.getContext(), actor} {
	auto& context = app.getContext();
	
	// setup widgets
	ASSERT(context.game != nullptr);
	auto display_name = context.game->engine.physics.focus.query(actor).display_name;
	setupTitle(title_label, "character.title", context, " (" + display_name + ")");
	auto& back_btn = menu.acquire<ui::Button>(BACK);
	setupButton(back_btn, "general.back", context);
	back_btn.activate = [&]() { onBackClick(); };
	menu.setFocus(back_btn);
}

void CharacterViewerState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
	target.draw(stats, states);
}

void CharacterViewerState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& back_btn = menu.query<ui::Button>(BACK);
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	stats.onResize(screen_size);
	back_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
}

void CharacterViewerState::onBackClick() {
	quit();
}

void CharacterViewerState::handle(sf::Event const& event) {
	menu.handle(event);
	
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		case sf::Event::Closed:
			onBackClick();
			break;
			
		case sf::Event::JoystickConnected:
		case sf::Event::JoystickDisconnected:
			menu.refreshMenuControls();
			break;
			
		default:
			break;
	}
}

void CharacterViewerState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	menu.update(elapsed);
}

} // ::state
