#include <core/focus.hpp>
#include <game/hud.hpp>

namespace game {

namespace hud_impl {

Context::Context(core::LogContext& log, core::CameraSystem const & camera_system,
	core::MovementManager const & movement_manager, core::FocusManager const & focus_manager,
	core::DungeonSystem& dungeon_system, rpg::StatsManager const & stats_manager,
	rpg::PlayerManager const & player_manager, HudManager& hud_manager,
	Localization& locale)
	: combat_font{nullptr}
	, combat_size{}
	, corner{}
	, border{}
	, log{log}
	, cameras{camera_system}
	, movements{movement_manager}
	, focus{focus_manager}
	, dungeons{dungeon_system}
	, stats{stats_manager}
	, players{player_manager}
	, huds{hud_manager}
	, locale{locale}
	, floating_texts{} {
}

// --------------------------------------------------------------------

void updateStatBars(HudData& hud, rpg::StatsData const & stats) {
	ASSERT(hud.hud != nullptr);
	hud.hud->setLife(stats.stats[rpg::Stat::Life], stats.properties[rpg::Property::MaxLife]);
	hud.hud->setMana(stats.stats[rpg::Stat::Mana], stats.properties[rpg::Property::MaxMana]);
	hud.hud->setStamina(stats.stats[rpg::Stat::Stamina], stats.properties[rpg::Property::MaxStamina]);
}

void updateExpBar(HudData& hud, rpg::PlayerData const & player) {
	ASSERT(hud.hud != nullptr);
	hud.hud->setExp(player.exp, player.base_exp, player.next_exp);
}

void addCombatLabel(Context& context, core::ObjectID id, unsigned int value, sf::Color const & color) {
	auto const & move = context.movements.query(id);
	ASSERT(move.scene > 0u);
	auto const & dungeon = context.dungeons[move.scene];
	auto pos = dungeon.toScreen(move.pos);
	
	auto factor = thor::random(0.8f, 1.2f);
	
	ASSERT(context.combat_font != nullptr);
	auto i = context.floating_texts.find(move.scene);
	if (i != context.floating_texts.end()) {
		context.floating_texts[move.scene] = ui::FloatingTexts{};
	}
	
	context.floating_texts[move.scene].add(*context.combat_font, std::to_string(value),
		static_cast<unsigned int>(context.combat_size * factor), pos, color,
		sf::seconds(2.f * factor));
}

// --------------------------------------------------------------------

void onTeleport(Context& context, core::TeleportEvent const & event) {
	auto& hud_data = context.huds.query(event.actor);
	
	if (event.src_scene < event.dst_scene) {
		// notify about going downstairs (because scene ID grew)
		hud_data.hud->notify(context.locale("hud.downstairs") + " "
			+ std::to_string(event.dst_scene), sf::Color::White);
		
	} else if (event.src_scene > event.dst_scene) {
		// notify about going upstairs (because scene ID fell)
		hud_data.hud->notify(context.locale("hud.upstairs") + " "
			+ std::to_string(event.dst_scene), sf::Color::White);
		
	} else {
		// notify about teleport
		hud_data.hud->notify(context.locale("hud.teleport"), sf::Color::White);
	}
}

void onStats(Context& context, rpg::StatsEvent const & event) {
	bool towards_player = context.huds.has(event.actor);
	bool from_player = event.actor > 0u && context.huds.has(event.actor);
	
	if (towards_player) {
		// update stat bars
		updateStatBars(context.huds.query(event.actor), context.stats.query(event.actor));
		
		auto life = event.delta[rpg::Stat::Life];
		auto mana = event.delta[rpg::Stat::Mana];
		
		if (life < 0) {
			// show damage towards player
			addCombatLabel(context, event.actor, std::abs(life), sf::Color::Red);
		} else if (life > 0) {
			// show healing towards player
			addCombatLabel(context, event.actor, life, sf::Color::Green);
		}
		
		if (mana > 0) {
			// show player's mana regeneration
			addCombatLabel(context, event.actor, mana, sf::Color::Blue);
		}
		
	} else if (from_player) {
		auto life = event.delta[rpg::Stat::Life];
		
		if (life < 0) {
			// show damage by player
			addCombatLabel(context, event.causer, std::abs(life), sf::Color::White);
		}
	
	}
}

void onDeath(Context& context, rpg::DeathEvent const & event) {
	if (context.huds.has(event.actor)) {
		auto& hud_data = context.huds.query(event.actor);
		
		// notify about respawn
		hud_data.hud->notify(context.locale("hud.death"), sf::Color::Red);
	}
}

void onSpawn(Context& context, rpg::SpawnEvent const & event) {
	auto& hud_data = context.huds.query(event.actor);
	
	if (event.respawn) {
		// notify about respawn
		hud_data.hud->notify(context.locale("hud.respawn"), sf::Color::Green);
	}
}

void onExp(Context& context, rpg::ExpEvent const & event) {
	if (context.huds.has(event.actor)) {
		// update exp bar
		auto& hud_data = context.huds.query(event.actor);
		updateExpBar(hud_data, context.players.query(event.actor));
		
		if (event.levelup) {
			auto const & stats_data = context.stats.query(event.actor);
			
			// update stat bars
			updateStatBars(hud_data, stats_data);
			
			// notify about levelup
			hud_data.hud->notify(context.locale("hud.levelup") + " "
				+ std::to_string(stats_data.level), sf::Color::Yellow);
		}
	}
}

void onFeedback(Context& context, rpg::FeedbackEvent const & event) {
}

void onPowerup(Context& context, PowerupEvent const & event) {
}

void onUpdate(Context const & context, HudData& hud, sf::Time const & elapsed) {
	ASSERT(hud.hud != nullptr);
	
	auto const & stats = context.stats.query(hud.id);
	auto const & move  = context.movements.query(hud.id);
	ASSERT(move.scene > 0u);
	auto const & dungeon = context.dungeons[move.scene];
	
	// query focus to update hud
	auto focus = core::focus_impl::getFocus(hud.id, dungeon, context.focus, context.movements);
	std::uint32_t life{0u}, max_life{0u}, level{0u};
	std::string name{};
	auto color = sf::Color::White;
	if (focus > 0u && context.focus.has(focus) && stats.stats[rpg::Stat::Life] > 0) {
		// set focus (only if actor is alive)
		name = context.focus.query(focus).display_name;
		if (context.stats.has(focus)) {
			if (!context.players.has(focus)) {
				color = sf::Color::Red;
			}
			auto const & tmp = context.stats.query(focus);
			life = tmp.stats[rpg::Stat::Life];
			max_life = tmp.properties[rpg::Property::MaxLife];
			level = tmp.level;
		}
	}
	// update focus hud (and disable it if necessary)
	hud.hud->setFocus(name, color, life, max_life, level);
	
	// update entire hud
	hud.hud->update(elapsed);
}

// ---------------------------------------------------------------------------

void drawDecoration(Context const & context, sf::RenderTarget& target, sf::RenderStates states) {
	auto hud_size = target.getView().getSize();
	auto border_size = context.border.getTexture()->getSize();
	auto tmp = states;
	auto& matrix = tmp.transform;
	
	// enlarge texture rect (texture repeat will do the rest)
	auto width = static_cast<int>(std::max(hud_size.x, hud_size.y));
	auto height = static_cast<int>(border_size.y);
	context.border.setTextureRect({0, 0, width, height});
	
	// draw borders
	target.draw(context.border, tmp);
	matrix.translate(hud_size.x, 0.f);
	matrix.rotate(90.f);
	target.draw(context.border, tmp);
	matrix.translate(hud_size.y, 0.f);
	matrix.rotate(90.f);
	target.draw(context.border, tmp);
	matrix.translate(hud_size.x, 0.f);
	matrix.rotate(90.f);
	target.draw(context.border, tmp);
	
	// draw corners
	matrix = states.transform;
	target.draw(context.corner, tmp);
	matrix.translate(hud_size.x, 0.f);
	matrix.rotate(90.f);
	target.draw(context.corner, tmp);
	matrix.translate(hud_size.y, 0.f);
	matrix.rotate(90.f);
	target.draw(context.corner, tmp);
	matrix.translate(hud_size.x, 0.f);
	matrix.rotate(90.f);
	target.draw(context.corner, tmp);
}

}  // ::hud_impl

// ---------------------------------------------------------------------------

HudSystem::HudSystem(core::LogContext& log, std::size_t max_objects, core::CameraSystem const & camera_system,
	core::MovementManager const & movement_manager, core::FocusManager const & focus_manager,
	core::DungeonSystem& dungeon_system, rpg::StatsManager const & stats_manager,
	rpg::PlayerManager const & player_manager, Localization& locale)
	: utils::EventListener<core::TeleportEvent,
		rpg::StatsEvent, rpg::DeathEvent, rpg::SpawnEvent, rpg::ExpEvent,
		rpg::FeedbackEvent, PowerupEvent>{}
	, HudManager{max_objects}
	, sf::Drawable{}
	, context{log, camera_system, movement_manager, focus_manager,
		dungeon_system, stats_manager, player_manager, *this, locale} {
}

void HudSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	ASSERT(context.corner.getTexture() != nullptr);
	ASSERT(context.border.getTexture() != nullptr);
	
	// draw each camera's hud
	for (auto const & cam_ptr : context.cameras) {
		// draw each object's playerhud
		target.setView(cam_ptr->hud);
		// draw camera decoration
		hud_impl::drawDecoration(context, target, states);
		// draw all player huds
		utils::SceneID scene{0u};
		for (auto id : cam_ptr->objects) {
			if (!has(id)) {
				continue;
			}
			if (scene == 0u) {
				// determine scene
				scene = context.movements.query(id).scene;
			}
			auto& data = query(id);
			ASSERT(data.hud != nullptr);
			target.draw(*data.hud, states);
		}
		// draw floating texts for this dungeon
		auto i = context.floating_texts.find(scene);
		if (i != context.floating_texts.end()) {
			target.setView(cam_ptr->scene);
			target.draw(i->second, states);
		}
	}
}

void HudSystem::setup(sf::Font const & combat_font, unsigned int combat_size,
	sf::Texture const & corner_tex, sf::Texture const & border_tex) {
	context.combat_font = &combat_font;
	context.combat_size = combat_size;
	context.corner.setTexture(corner_tex);
	context.border.setTexture(border_tex);
}

void HudSystem::handle(core::TeleportEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	
	onTeleport(context, event);
}

void HudSystem::handle(rpg::StatsEvent const & event) {
	onStats(context, event);
}

void HudSystem::handle(rpg::DeathEvent const & event) {
	if (!has(event.actor) && (event.causer > 0u && !has(event.causer))) {
		return;
	}
	
	onDeath(context, event);
}

void HudSystem::handle(rpg::SpawnEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	
	onSpawn(context, event);
}

void HudSystem::handle(rpg::ExpEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	
	onExp(context, event);
}

void HudSystem::handle(rpg::FeedbackEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	
	onFeedback(context, event);
}

void HudSystem::handle(PowerupEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	
	onPowerup(context, event);
}

void HudSystem::update(sf::Time const & elapsed) {
	dispatch<core::TeleportEvent>(*this);
	dispatch<rpg::StatsEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::ExpEvent>(*this);
	dispatch<rpg::FeedbackEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<PowerupEvent>(*this);
	
	for (auto& pair: context.floating_texts) {
		pair.second.update(elapsed);
	}
	
	for (auto& data: *this) {
		onUpdate(context, data, elapsed);
	}
}

}  // ::rage
