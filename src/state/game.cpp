#include <Thor/Math.hpp>

#include <core/algorithm.hpp>
#include <core/teleport.hpp>
#include <game/generator.hpp>
#include <game/player.hpp>
#include <state/game.hpp>
#include <state/pause.hpp>
#include <state/tool/testmode.hpp>

namespace state {

void searchPosition(std::vector<sf::Vector2u> const & tiles, sf::Vector2f& pos, core::Dungeon const & dungeon, unsigned int max_step) {
	do {
		pos = sf::Vector2f{utils::randomAt(tiles)};
	} while (!core::getFreePosition([&](sf::Vector2f const & p) {
		if (dungeon.has(sf::Vector2u{p})) {
			auto const & cell = dungeon.getCell(sf::Vector2u{p});
			return cell.terrain == core::Terrain::Floor
				&& cell.entities.empty();
		}
		return false;
	}, pos, max_step));
}

// --------------------------------------------------------------------

GameState::GameState(App& app)
	: State{app}
	, utils::EventListener<rpg::ActionEvent, rpg::ExpEvent /* ALPHA ONLY */>{}
	, child{nullptr}
	, freezed{false}
	, difficulty{}
	, fps{}
	, time_monitor{} {
	auto& context = app.getContext();
	ASSERT(context.game != nullptr);
	auto& game = *context.game;
	
	auto const & hud_font = game.engine.mod.get<sf::Font>(context.globals.widget.font);
	auto const & combat_font = game.engine.mod.get<sf::Font>(context.globals.combat.font);
	auto const & notify_font = game.engine.mod.get<sf::Font>(context.globals.notification.font);
	auto hud_size = context.globals.widget.char_size;
	auto combat_size = context.globals.combat.char_size;
	auto notify_size = context.globals.notification.char_size;
	auto hud_pad = context.globals.hud_padding;
	auto hud_margin = context.globals.hud_margin;
	auto const & hud_status_box = game.engine.mod.get<sf::Texture>("ui/statusbar_box");
	auto const & hud_status_fill = game.engine.mod.get<sf::Texture>("ui/statusbar_fill");
	auto const & hud_focus_box = game.engine.mod.get<sf::Texture>("ui/focusbar_box");
	auto const & hud_focus_fill = game.engine.mod.get<sf::Texture>("ui/focusbar_fill");
	auto const & hud_deco_corner = game.engine.mod.get<sf::Texture>("ui/deco_corner");
	auto const & hud_deco_border = game.engine.mod.get<sf::Texture>("ui/deco_border");
	game.engine.mod.query<sf::Texture>("ui/deco_border").setRepeated(true);
	
	// setup fps sprite
	game.engine.ui.hud.setup(combat_font, combat_size, hud_deco_corner, hud_deco_border);
	difficulty.setFont(hud_font);
	difficulty.setCharacterSize(hud_size);
	difficulty.setFillColor(sf::Color::White);
	difficulty.setOutlineColor(sf::Color::White);
	difficulty.setString(context.locale("lobby.difficulty") + ": "
		+ context.locale("difficulty." + to_string(context.settings.difficulty)));
	{
		// set origin to topright
		auto rect = difficulty.getLocalBounds();
		difficulty.setOrigin({rect.left + rect.width, 0});
	}
	fps.setFont(hud_font);
	fps.setCharacterSize(hud_size);
	fps.setFillColor(sf::Color::White);
	fps.setOutlineColor(sf::Color::White);
	
	// -------------------------------------------- TEST IMPLEMENTATION
	
	// prepare player resources
	game.lobby.players.resize(game.lobby.num_players);
	std::size_t bot_level{0u};
	for (auto& player: game.lobby.players) {
		if (!game::preparePlayer(player.tpl, game.engine.mod, player.filename)) {
			throw std::runtime_error("Cannot prepare player save " + player.filename);
		}
		bot_level += player.tpl.level;
	}
	bot_level /= game.lobby.players.size();
	context.log.debug << "[State/Game] Average bot level is " << bot_level << "\n";
	
	auto num_bots = 3u + game.lobby.players.size() / 2;
	auto min_num_bots = static_cast<std::uint32_t>(std::ceil(num_bots * 0.5f));
	auto max_num_bots = num_bots * 2u;
	context.log.debug << "[State/Game] Average bot group size is "
		<< num_bots << " [" << min_num_bots << "; " << max_num_bots << "]\n";

	// determine difficulty factor
	auto diffic = context.globals.difficulty[context.settings.difficulty];
	diffic += 0.2f * (game.lobby.players.size() - 1u);
	context.log.debug << "[State/Game] Difficulty is '" << to_string(context.settings.difficulty) << "' (" << (int)(diffic * 100) << "%)\n";

	auto encounters = game.engine.mod.getAll<game::EncounterTemplate>();
	std::vector<std::string> scripts, tilesets;
	game.engine.mod.getAllFiles<game::AiScript>(scripts);
	game.engine.mod.getAllFiles<rpg::TilesetTemplate>(tilesets);
	auto const& downstairs = game.engine.mod.get<rpg::EntityTemplate>("downstairs");
	auto const& upstairs = game.engine.mod.get<rpg::EntityTemplate>("upstairs");
	
	auto const & regenerate = game.engine.mod.get<rpg::EffectTemplate>("regenerate");
	
	// enable blood
	game.engine.factory.blood_texture = &game.engine.mod.get<sf::Texture>("blood");
	
	// setup powerup gem
	game.engine.factory.gem_tpl = &game.engine.mod.get<rpg::EntityTemplate>("gem");
	
	// racod's boss template
	auto const & boss = game.engine.mod.get<game::BotTemplate>("racod");
	
	// create dungeons
	auto const & settings = context.globals.dungeon_gen;
	
	auto all_ambiences = game.engine.mod.getAllAmbiences();
	
	game::BuildSettings build_settings;
	build_settings.cell_size = settings.cell_size;
	build_settings.path_width = 3u;
	
	std::vector<utils::SceneID> scenes;
	game::BuildInformation::Floors const * player_start{nullptr};
	for (auto i = 0u; i < game.lobby.num_dungeons; ++i) {
		auto const & tileset = game.engine.mod.get<rpg::TilesetTemplate>(utils::randomAt(tilesets));
		
		auto scene = game.engine.factory.createDungeon(tileset, game.lobby.dungeon_size, build_settings);
		auto& dungeon = game.engine.dungeon[scene];
		auto& builder = game.engine.generator[scene].builder;
		context.log.debug << "[State/Game] " << builder.rooms.size() << " rooms created\n";
		
		rpg::SpawnMetaData spawn;
		spawn.scene = scene;
		// random-distribute ambiences
		auto dist = [&](game::BuildInformation::Floors const & v) {
			for (auto const & pos: v) {
				// generator setting!!
				if (thor::random(0.f, 1.f) < settings.ambience_density) {
					// spawn random on tile
					spawn.pos = sf::Vector2f{pos};
					spawn.pos.x += thor::random(0.33f, 0.67f);
					spawn.pos.y += thor::random(0.33f, 0.67f);
					auto const & tex = *all_ambiences[thor::random(0u, all_ambiences.size()-1u)];
					game.engine.factory.createAmbience(tex, spawn);
				}
			}
		};
		for (auto const & room: builder.info.rooms) {
			dist(room);
		}
		for (auto const & corridor: builder.info.corridors) {
			dist(corridor);
		}
		
		if (player_start == nullptr) {
			// add player(s)
			player_start = &utils::randomAt(builder.info.rooms);
			std::size_t i{0u};
			for (auto& player: game.lobby.players) {
				auto color = context.globals.player_colors[i];
				if (game.lobby.num_players == 1u) {
					color = sf::Color::Transparent;
				}
				searchPosition(*player_start, spawn.pos, dungeon);
				// spawn centered on tile
				spawn.pos.x += 0.5f;
				spawn.pos.y += 0.5f;
				player.id = game.engine.factory.createPlayer(player.tpl, player.keys, spawn, color);
				
				// add regeneration effect (ALPHA ONLY!)
				rpg::EffectEvent ev;
				ev.actor = player.id;
				ev.effect = &regenerate;
				ev.type = rpg::EffectEvent::Add;
				game.engine.avatar.effect.receive(ev);
				
				game.saver.add(player.id, player.tpl, player.filename);
				
				player.player_id = game.engine.session.player.query(player.id).player_id;
				auto& hud_data = game.engine.session.hud.query(player.id);
				hud_data.hud->setup(hud_font, hud_size, notify_font, notify_size,
					hud_pad, hud_margin, hud_status_box, hud_status_fill,
					hud_focus_box, hud_focus_fill);
				hud_data.hud->setName(player.tpl.display_name);
				
				++i;
			}
		}
		
		// add bots
		auto const & encounter = *utils::randomAt(encounters);
		spawn.direction = {0, -1};
		auto lvl = bot_level + scene - 1;
		auto diff = diffic;
		if (lvl == 0u) {
			diff *= 0.5f;
		} else if (lvl < 10) {
			diff *= (0.5f + (0.05f * lvl));
			--lvl;
		} else {
			lvl -= lvl / 10;
		}
		std::size_t n{0u};
		for (auto const & room: builder.info.rooms) {
			if (&room == player_start) {
				// no bots in initial room
				continue;
			}
			// create new script instance
			auto& script = game.engine.mod.createScript(utils::randomAt(scripts));
			
			// test whether neither last dungeon nor last room
			if (i < game.lobby.num_dungeons - 1u || n < builder.info.rooms.size() - 1u) {
				searchPosition(room, spawn.pos, dungeon);
				for (auto i = 0; i < thor::random(min_num_bots, max_num_bots); ++i) {
					// spawn near position
					core::getFreePosition([&](sf::Vector2f const & p) {
						if (dungeon.has(sf::Vector2u{p})) {
							auto const & cell = dungeon.getCell(sf::Vector2u{p});
							return cell.terrain == core::Terrain::Floor
								&& cell.entities.empty();
						}
						return false;
					}, spawn.pos, 100u);
					auto const & bot = encounter.pick(thor::random(0.f, 1.f));
					game.engine.factory.createBot(bot, spawn, lvl, /*script,*/ true, diff);
				}
			} else {
				// place racod in last dungeon's last room
				searchPosition(room, spawn.pos, dungeon);
				game.engine.factory.createBot(boss, spawn, lvl + lvl / 10, /*script,*/ true, 3.f * diff * game.lobby.players.size());
			}
			++n;
		}
		
		scenes.push_back(scene);
	}
	
	// create stairs
	for (auto i = 0; i < game.lobby.num_dungeons - 1u; ++i) {
		rpg::SpawnMetaData spawn1, spawn2;
		
		spawn1.scene = scenes[i];
		spawn1.direction = {0, 1};
		spawn2.scene = scenes[i+1];
		spawn2.direction = {0, -1};
		
		// pick dungeon data
		auto const & src_dungeon = game.engine.dungeon[spawn1.scene];
		auto const & dst_dungeon = game.engine.dungeon[spawn2.scene];
		auto const & src_builder = game.engine.generator[spawn1.scene].builder;
		auto const & dst_builder = game.engine.generator[spawn2.scene].builder;
		
		// pick random room within dungeon
		auto const & src_room = utils::randomAt(src_builder.info.rooms);
		auto const & dst_room = utils::randomAt(dst_builder.info.rooms);
		
		// pick random position
		searchPosition(src_room, spawn1.pos, src_dungeon);
		searchPosition(dst_room, spawn2.pos, dst_dungeon);
		spawn1.pos += utils::HalfTilePos;
		spawn2.pos += utils::HalfTilePos;
		
		// place stairs and teleport triggers
		game.engine.factory.createObject(downstairs, spawn1);
		game.engine.factory.createObject(upstairs, spawn2);
		game.engine.factory.addTeleport(spawn1.scene, spawn1.pos, spawn2.scene, spawn2.pos);
		game.engine.factory.addTeleport(spawn2.scene, spawn2.pos, spawn1.scene, spawn1.pos);
	}
	
	// register game state as listener for pause events
	game.engine.behavior.bind<rpg::ActionEvent>(*this);
	
	// register game state as listener for levelup events ALPHA ONLY!
	game.engine.avatar.bind<rpg::ExpEvent>(*this);
	
	game.applySettings();
}

void GameState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	auto& game = *getContext().game;
	
	sf::Clock clock;
	target.draw(game.engine.ui, states);
	auto t = clock.restart().asMilliseconds();
	time_monitor["draw"] += t;
	
	if (child != nullptr) {
		target.draw(*child, states);
	}
	
	target.setView(default_view);
	target.draw(difficulty, states);
	target.draw(fps, states);
}

void GameState::onPause(core::ObjectID player) {
	if (player == 0u) {
		// pause game (assigned to first player)
		player = getContext().game->engine.session.player.begin()->id;
	}
	auto& app = getApplication();
	auto launch = std::make_unique<PauseState>(app, player);
	app.push(launch);
}

void GameState::onSetFreeze(bool flag) {
	freezed = flag;
	if (!freezed) {
		activate();
	}
}

void GameState::handle(sf::Event const& event) {
	auto& context = getContext();
	auto& game = *context.game;
	
	if (child != nullptr && child->handle(event)) {
		return;
	}
	
	if (freezed) {
		return;
	}
	
	game.engine.behavior.handle(event);
	game.engine.ui.handle(event);
	
	switch (event.type) {
		case sf::Event::Closed:
			onPause();
			break;
			
		case sf::Event::Resized:
			State::onResize({event.size.width, event.size.height});
			// resize default view
			default_view = getApplication().getWindow().getView();
			// align difficulty and fps sprite
			difficulty.setPosition(event.size.width - 10u, 10u);
			fps.setPosition(event.size.width - 10u, event.size.height - 10u);
			break;
			
		case sf::Event::KeyPressed:
			if (event.key.code == sf::Keyboard::F12 && event.key.control) {
				if (child == nullptr) {
					child = std::make_unique<tool::TestMode>(*this);
					context.log.debug << "Testmode enabled\n";
				} else {
					child = nullptr;
					context.log.debug << "Testmode disabled\n";
				}
			}
			/*
			else if (event.key.code == sf::Keyboard::P && event.key.control) {
				sf::Clock clock;
				//auto fname = engine::get_preference_dir() + "screenshot.png";
				auto fname = "/tmp/screenshot.png";
				auto screenshot = getApplication().getWindow().capture();
				screenshot.saveToFile(fname);
				context.log.debug << "Screenshot within " << clock.restart() << "\n";
			}
			*/
			
		default:
			break;
	}
}

void GameState::handle(rpg::ActionEvent const& event) {
	if (event.action == rpg::PlayerAction::Pause) {
		onPause(event.actor);
	}
}

void GameState::handle(rpg::ExpEvent const& event) { /* ALPHA ONLY! */
	if (event.levelup == 0u) {
		return;
	}
	
	auto& context = getContext();
	auto& game = *context.game;
	
	// automatically use attribute and perk points
	auto weapon_ptr = game.engine.session.item.query(event.actor).equipment[rpg::EquipmentSlot::Weapon];
	auto const & perks = game.engine.session.perk.query(event.actor).perks;
	
	rpg::TrainingEvent ev;
	ev.actor = event.actor;
	
	for (auto i = 0u; i < event.levelup; ++i) {
		ev.type = rpg::TrainingEvent::Attrib;
		if (perks.empty()) {
			ASSERT(weapon_ptr != nullptr);
			if (weapon_ptr->melee) {
				// assume warrior
				ev.attrib = rpg::Attribute::Strength;
				game.engine.avatar.player.receive(ev);
				game.engine.avatar.player.receive(ev);
				game.engine.avatar.player.receive(ev);
				ev.attrib = rpg::Attribute::Dexterity;
				game.engine.avatar.player.receive(ev);
				game.engine.avatar.player.receive(ev);
			} else {
				// assume archer
				ev.attrib = rpg::Attribute::Dexterity;
				game.engine.avatar.player.receive(ev);
				game.engine.avatar.player.receive(ev);
				game.engine.avatar.player.receive(ev);
			}
		} else {
			// assume wizard
			ev.attrib = rpg::Attribute::Strength;
			game.engine.avatar.player.receive(ev);
			game.engine.avatar.player.receive(ev);
			ev.attrib = rpg::Attribute::Wisdom;
			game.engine.avatar.player.receive(ev);
			game.engine.avatar.player.receive(ev);
			game.engine.avatar.player.receive(ev);
			ev.type = rpg::TrainingEvent::Perk;
			ev.perk = perks.front().perk;
			game.engine.avatar.player.receive(ev);
		}
	}
}

void GameState::update(sf::Time const& elapsed) {
	auto& context = getContext();
	context.update(elapsed);
	auto& game = *context.game;
	
	std::lock_guard<std::mutex> lock{game.mutex};
	
	if (child != nullptr) {
		child->update(elapsed);
	}
	
	if (freezed) {
		return;
	}
	
	sf::Clock local, clock;
	for (auto ptr : game.engine.session.systems) {
		ptr->cleanup();
	}
	game.engine.session.id_manager.cleanup();
	time_monitor["cleanup"] += clock.restart().asMilliseconds();
	time_monitor.update(elapsed);

	core::updateChunked([&](sf::Time const& t) {
		time_monitor["behavior"] += game.engine.behavior.update(t).asMilliseconds();
		time_monitor["physics"] += game.engine.physics.update(t).asMilliseconds();
		time_monitor["avatar"] += game.engine.avatar.update(t).asMilliseconds();
		time_monitor["ui"] += game.engine.ui.update(t, context.settings.autocam).asMilliseconds();
		{
			sf::Clock clock;
			game.engine.combat.update(t);
			time_monitor["combat"] += clock.restart().asMilliseconds();
		}
		game.engine.factory.update(t);
	}, elapsed, sf::milliseconds(core::MAX_FRAMETIME_MS));

	time_monitor["ai"] += game.engine.ai.update(elapsed).asMilliseconds();

	auto remain = sf::milliseconds(1000 / 60) - local.getElapsedTime();
	if (remain <= sf::milliseconds(5u)) {
		remain = sf::milliseconds(5u);
	}
	clock.restart();
	game.engine.ai.path.calculate(remain);
	auto delta = clock.restart();
	time_monitor["ai"] += delta.asMilliseconds();
	
	time_monitor["save"] += game.saver.getElapsedTime().asMilliseconds();
	
	// dispatch pause events
	dispatch<rpg::ActionEvent>(*this);
	dispatch<rpg::ExpEvent>(*this); /* ALPHA ONLY */
}

void GameState::onFramerateUpdate(float framerate) {
	if (freezed) {
		fps.setString("Engine frozen");
	} else {
		// update sprite with framerate
		fps.setString(std::to_string(static_cast<int>(framerate)));
	}
	// set origin to bottomright
	auto rect = fps.getLocalBounds();
	fps.setOrigin({rect.left + rect.width, rect.top + rect.height});
}

void GameState::activate() {
	State::activate();
	
	auto& context = getContext();
	auto& game = *context.game;
	
	// reset state of input mapper
	game.engine.behavior.input.reset();
	
	// stop title theme
	context.theme.stop();
	
	// resume ambient music
	game.engine.ui.music.resume();
	
	// start saving thread
	if (context.settings.autosave && !game.saver.isRunning()) {
		game.saver.start();
	}
}

}  // ::state
