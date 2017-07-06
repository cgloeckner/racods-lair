#include <utils/filesystem.hpp>

#include <ui/button.hpp>
#include <ui/checkbox.hpp>
#include <ui/select.hpp>

#include <state/lobby.hpp>
#include <state/profile_creator.hpp>
#include <state/savegame_menu.hpp>
#include <state/game_launcher.hpp>

namespace state {

static std::size_t const NUM_PLAYERS = 0u;
static std::size_t const NUM_DUNGEONS = 1u;
static std::size_t const DIFFICULTY = 2u;

static std::size_t const FIRST_PLAYER = 4u;

// --------------------------------------------------------------------

LobbyState::LobbyState(App& app)
	: State{app}
	, START{FIRST_PLAYER + getContext().globals.max_num_players}
	, BACK{START+1u}
	, menu{}
	, available_profiles{}
	, title_label{}
	, num_players_label{}
	, num_dungeons_label{}
	, difficulty_label{}
	, player_labels{}
	, device_labels{}
	, warning_label{}
	, lobby{getContext().globals.max_num_players} {
	auto& context = getContext();
	lobby.dungeon_size = context.globals.dungeon_size;
	
	// setup widgets
	setupTitle(title_label, "lobby.title", context);
	setupLabel(num_players_label, "lobby.num_players", context);
	setupLabel(num_dungeons_label, "lobby.num_dungeons", context);
	setupLabel(difficulty_label, "lobby.difficulty", context);
	setupWarning(warning_label.caption, context);
	auto& num_players_select = menu.acquire<ui::Select>(NUM_PLAYERS);
	setupSelect(num_players_select, context);
	for (auto i = 0; i < context.globals.max_num_players; ++i) {
		num_players_select.push_back(std::to_string(i+1));
	}
	num_players_select.setIndex(0u);
	auto& num_dungeons_select = menu.acquire<ui::Select>(NUM_DUNGEONS);
	setupSelect(num_dungeons_select, context);
	ASSERT(context.globals.min_num_dungeons <= context.globals.max_num_dungeons);
	num_dungeons_select.reserve(context.globals.max_num_dungeons - context.globals.min_num_dungeons + 1);
	for (auto i = context.globals.min_num_dungeons; i <= context.globals.max_num_dungeons; ++i) {
		num_dungeons_select.push_back(std::to_string(i));
	}
	ASSERT(num_dungeons_select.size() >= 1u);
	num_dungeons_select.setIndex(0u);
	auto& difficulty_select = menu.acquire<ui::Select>(DIFFICULTY);
	setupSelect(difficulty_select, context);
	for (auto value: utils::EnumRange<Difficulty>{}) {
		difficulty_select.push_back(context.locale("difficulty." + to_string(value)));
	}
	difficulty_select.setIndex(static_cast<std::size_t>(context.settings.difficulty));
	
	// create profile select widgets
	for (auto i = 0u; i < context.globals.max_num_players; ++i) {
		// setup profile label
		player_labels.emplace_back();
		auto& label = player_labels.back();
		setupLabel(label, "lobby.player", context, " " + std::to_string(i + 1));
		// setup widget
		auto& select = menu.acquire<ui::Select>(FIRST_PLAYER + i);
		setupSelect(select, context);
		select.activate = [&, i]() { onEditProfile(i); };
		select.change = [&, i]() { onUpdateSelection(i); };
		// setup device label
		device_labels.emplace_back();
		auto& label2 = device_labels.back();
		setupLabel(label2, "lobby.unassigned", context);
	}
	
	auto& start_btn = menu.acquire<ui::Button>(START);
	auto& back_btn = menu.acquire<ui::Button>(BACK);
	setupButton(start_btn, "lobby.start", context);
	setupButton(back_btn, "general.back", context);
	difficulty_select.change = [&]() { onSetDifficulty(); };
	num_players_select.change = [&]() { onSetNumPlayers(); };
	start_btn.activate = [&]() { onStartClick(); };
	back_btn.activate = [&]() { onBackClick(); };
	
	menu.setFocus(num_players_select);
	onSetNumPlayers();
}

void LobbyState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
	target.draw(num_players_label, states);
	target.draw(num_dungeons_label, states);
	target.draw(difficulty_label, states);
	target.draw(warning_label.caption, states);
	
	auto num_players = menu.query<ui::Select>(NUM_PLAYERS).getIndex() + 1u;
	for (auto i = 0u; i < num_players; ++i) {
		target.draw(player_labels[i], states);
		target.draw(device_labels[i], states);
	}
}

void LobbyState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	auto pad = context.globals.vertical_padding;
	auto hpad = context.globals.horizontal_padding;
	
	auto& start_btn = menu.query<ui::Button>(START);
	auto& back_btn = menu.query<ui::Button>(BACK);
	auto& num_players_select = menu.query<ui::Select>(NUM_PLAYERS);
	auto& num_dungeons_select = menu.query<ui::Select>(NUM_DUNGEONS);
	auto& difficulty_select = menu.query<ui::Select>(DIFFICULTY);
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	ui::setPosition(num_players_label, {screen_size.x / 2.f - hpad, 100.f + 2.f * pad});
	num_players_select.setPosition({screen_size.x / 2.f, 100.f + 2.f * pad});
	ui::setPosition(num_dungeons_label, {screen_size.x / 2.f - hpad, 100.f + 3.f * pad});
	num_dungeons_select.setPosition({screen_size.x / 2.f, 100.f + 3.f * pad});
	ui::setPosition(difficulty_label, {screen_size.x / 2.f - hpad, 100.f + 4.f * pad});
	difficulty_select.setPosition({screen_size.x / 2.f, 100.f + 4.f * pad});
	
	float dy = 100.f + 5.f * pad;
	for (auto i = 0u; i < context.globals.max_num_players; ++i) {
		player_labels[i].setPosition({screen_size.x / 2.f - hpad, dy});
		auto& select = menu.query<ui::Select>(FIRST_PLAYER + i);
		select.setPosition({screen_size.x / 2.f, dy});
		device_labels[i].setPosition({screen_size.x / 2.f + hpad, dy});
		dy += pad;
	}
	warning_label.caption.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - 2.f * pad});
	start_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - pad});
	back_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
}

void LobbyState::onJoystickDisconnected(std::size_t id) {
	auto& context = getContext();
	
	for (auto i = 0; i < context.globals.max_num_players; ++i) {
		auto& player = lobby.players[i];
		if (player.use_gamepad && player.gamepad_id == id) {
			player.use_gamepad = false;
			onUpdateSelection(i);
		}
	}
}

void LobbyState::onSetDifficulty() {
	auto& context = getContext();
	context.settings.difficulty = static_cast<Difficulty>(menu.query<ui::Select>(DIFFICULTY).getIndex());
	
	auto path = engine::get_preference_dir();
	if (!context.settings.saveToFile(path + context.settings.getFilename())) {
		context.log.error << "[State/Lobby] " << "Cannot save settings\n";
		context.log.error.flush();
	}
}

void LobbyState::onSetNumPlayers() {
	auto& context = getContext();
	lobby.num_players = menu.query<ui::Select>(NUM_PLAYERS).getIndex() + 1u;
	
	for (auto i = 0u; i < context.globals.max_num_players; ++i) {
		auto& select = menu.query<ui::Select>(FIRST_PLAYER + i);
		select.setVisible(i < lobby.num_players);
		
		if (lobby.num_players > 1u) {
			player_labels[i].setColor(context.globals.player_colors[i]);
		} else {
			player_labels[i].setColor(sf::Color::White);
		}
	}
}

void LobbyState::onUpdateSelection(std::size_t i) {
	auto& context = getContext();
	
	ASSERT(i < context.globals.max_num_players);
	auto& player = lobby.players[i];
	
	// load profile
	auto& select = menu.query<ui::Select>(FIRST_PLAYER + i);
	auto index = select.getIndex();
	if (index > 0u) {
		ASSERT(index <= available_profiles.size());
		player.filename = available_profiles[index-1];
		auto sav_path = player.getSavegameName();
		auto key_path = player.getKeybindingName();
		if (!player.tpl.loadFromFile(sav_path)) {
			context.log.error << "[Game/Lobby] Cannot load profile '"
				<< player.filename << "': corrupted savegame\n";
			quit();
			return;
		}
		if (!player.keys.loadFromFile(key_path)) {
			context.log.error << "[Game/Lobby] Cannot load profile '"
				<< player.filename << "': corrupted keybinding\n";
			quit();
			return;
		}
	} else {
		player = LobbyContext::Player{};
	}
	
	// determine whether gamepad device suits keysbinding more
	bool is_gamepad{false};
	for (auto action: utils::EnumRange<rpg::PlayerAction>{}) {
		if (player.keys.map.get(action).type != utils::InputAction::Key) {
			// at least one gamepad action was found!
			is_gamepad = true;
			if (!player.use_gamepad) {
				bool pad_found{false};
				for (auto i = 0; i < sf::Joystick::Count; ++i) {
					if (sf::Joystick::isConnected(i)) {
						player.gamepad_id = i;
						pad_found = true;
						break;
					}
				}
				if (!pad_found) {
					is_gamepad = false;
				}
			}
			break;
		}
	}
	player.use_gamepad = is_gamepad;
	
	// create any apply device caption
	std::string caption;
	if (player.use_gamepad) {
		caption = context.locale("general.gamepad") + " #"
			+ std::to_string(player.gamepad_id);
	} else {
		caption = context.locale("general.keyboard");
	}
	ASSERT(i < device_labels.size());
	auto& label = device_labels[i];
	label.setString(caption);
	ui::centerify(label);
}

void LobbyState::onEditProfile(std::size_t n) {
	ASSERT(n < lobby.players.size());
	auto& player = lobby.players[n];
	auto& app = getApplication();
	
	if (player.filename.empty()) {
		// enter profile creator
		auto launch = std::make_unique<ProfileCreatorState>(app, player);
		app.push(launch);
	} else {
		// enter savegame menu
		auto launch = std::make_unique<SavegameMenuState>(app, player);
		app.push(launch);
	}
}

void LobbyState::onStartClick() {
	auto& context = getContext();
	
	lobby.num_players = menu.query<ui::Select>(NUM_PLAYERS).getIndex() + 1u;
	lobby.num_dungeons = menu.query<ui::Select>(NUM_DUNGEONS).getIndex() + context.globals.min_num_dungeons;
	
	std::size_t lhs, rhs;
	utils::InputAction a;
	unsigned int id;
	
	// avoid inconsistent profiles (mixture of keyboard and gamepad input)
	if (lobby.hasInconsistentProfile(lhs)) {
		auto name = lobby.players[lhs].tpl.display_name;
		context.log.debug << "[State/Lobby] " << "Inconsistent profile '" << name << "'\n";
		setWarning(context.locale("lobby.inconsistent_profile")
			+ " '" + name + "'");
		return;
	}
	
	// avoid unset profiles
	if (lobby.hasUnsetProfile(lhs)) {
		auto name = lobby.players[lhs].tpl.display_name;
		context.log.debug << "[State/Lobby] " << "Unset profile #" << lhs << "\n";
		setWarning(context.locale("lobby.unset_profile")
			+ " " + std::to_string(lhs) + "");
		return;
	}
	
	// avoid multiple use of profiles
	if (lobby.hasDoubleUsedProfile(lhs)) {
		auto name = lobby.players[lhs].tpl.display_name;
		context.log.debug << "[State/Lobby] " << "Doubleuse of profile '" << name << "'\n";
		setWarning(context.locale("lobby.multi_profile")
			+ " '" + name + "'");
		return;
	}
	
	// avoid ambiguous input (like one key for multiple actions)
	if (lobby.hasAmbiguousInput(a, lhs)) {
		auto name = lobby.players[lhs].tpl.display_name;
		auto dump = a.toString();
		context.log.debug << "[State/Lobby] " << "Ambiguous input '" << dump
			<< "' by '" << name << "'\n";
		setWarning(context.locale("lobby.ambiguous_input")
			+ " '" + dump + "' ('" + name + "')");
		return;
	}
	
	// avoid shared input (like one key used by multiple profiles)
	if (lobby.hasSharedInput(a, lhs, rhs)) {
		auto name1 = lobby.players[lhs].tpl.display_name;
		auto name2 = lobby.players[rhs].tpl.display_name;
		auto dump = a.toString();
		context.log.debug << "[State/Lobby] " << "Shared input '" << dump
			<< "' between '" << name1 << "' and '" << name2 << "'\n";
		setWarning(context.locale("lobby.ambiguous_input")
			+ " '" + dump + "' ('" + name1 + "', '" + name2 + "')");
		return;
	}
	
	// avoid shared gamepads
	if (lobby.hasSharedGamepad(id)) {
		context.log.debug << "[State/Lobby] " << "Shared gamepad #" << id << "\n";
		setWarning(context.locale("lobby.shared_gamepad") + " #"
			+ std::to_string(id));
		return;
	}
	
	auto& app = getApplication();
	auto launch = std::make_unique<GameLauncherState>(app, lobby);
	app.push(launch);
}

void LobbyState::onBackClick() {
	quit();
}

void LobbyState::setWarning(std::string const & msg) {
	warning_label.caption.setString(msg);
	ui::centerify(warning_label.caption);
	warning_label.max_age = sf::seconds(5.f);
	warning_label.decay = 1.f / warning_label.max_age.asMilliseconds();
	warning_label.alpha = 1.f;
}

void LobbyState::handle(sf::Event const& event) {
	menu.handle(event);
	
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		case sf::Event::JoystickDisconnected:
			onJoystickDisconnected(event.joystickConnect.joystickId);
			menu.refreshMenuControls();
			break;
			
		case sf::Event::Closed:
			onBackClick();
			break;
			
		case sf::Event::JoystickConnected:
			menu.refreshMenuControls();
			break;
			
		default:
			break;
	}
}

void LobbyState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	warning_label.update(elapsed);
	
	menu.update(elapsed);
}

void LobbyState::activate() {
	State::activate();
	auto& context = getContext();
	
	// continue theme if paused
	// note: this happens after the game is quit
	if (context.theme.getStatus() != sf::SoundSource::Playing) {
		context.theme.play();
	}
	
	auto old = available_profiles;
	
	// query available profiles
	available_profiles.clear();
	auto path = engine::get_preference_dir() + "saves/";
	utils::for_each_file(path, ".xml", [&](std::string const & p, std::string const & fname) {
		available_profiles.push_back(fname);
	});
	
	// create profile select widgets
	for (auto i = 0u; i < context.globals.max_num_players; ++i) {
		// setup widget
		auto& select = menu.query<ui::Select>(FIRST_PLAYER + i);
		auto prev = select.getIndex();
		select.clear();
		select.push_back(context.locale("lobby.new_profile"));
		for (auto const & fname: available_profiles) {
			LobbyContext::Player tmp;
			tmp.filename = fname;
			auto p = tmp.getSavegameName();
			if (!tmp.tpl.loadFromFile(p)) {
				context.log.warning << "[State/Lobby] Profile '"
					<< fname << "' is broken. Skipped\n";
				continue;
			}
			select.push_back(tmp.tpl.display_name);
		}
		std::size_t index{0u};
		if (prev > 0) {
			ASSERT(prev <= old.size());
			// search profile to select it again
			auto pname = old[prev - 1];
			for (auto i = 0u; i < available_profiles.size(); ++i) {
				if (available_profiles[i] == pname) {
					index = i + 1;
					break;
				}
			}
		}
		select.setIndex(index);
		
		onUpdateSelection(i);
	}
}

} // ::state
