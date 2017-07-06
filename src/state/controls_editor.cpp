#include <string>

#include <ui/input.hpp>
#include <ui/select.hpp>
#include <ui/button.hpp>

#include <state/controls_editor.hpp>

namespace state {

static std::size_t const DEVICE = 0u;
static std::size_t const FIRST_ACTION = 1u;

static std::size_t const RESET		= FIRST_ACTION + utils::getEnumCount<rpg::PlayerAction>();
static std::size_t const SAVE		= RESET + 1u;
static std::size_t const DISCARD	= SAVE + 1u;
static std::size_t const BACK		= DISCARD + 1u;

// --------------------------------------------------------------------

ControlsEditorState::ControlsEditorState(App& app, LobbyContext::Player& player)
	: State{app}
	, player{player}
	, keys{}
	, menu{}
	, title_label{}
	, warning_label{}
	, bind_labels{}
	, wait_for{nullptr} {
	auto& context = app.getContext();
	
	setupTitle(title_label, "controls.title", context, " (" + player.tpl.display_name + ")");
	setupWarning(warning_label.caption, context);
	
	auto& device_select = menu.acquire<ui::Select>(DEVICE);
	setupSelect(device_select, context);
	device_select.push_back(context.locale("general.keyboard"));
	device_select.setIndex(0u);
	device_select.change = [&]() { onSelectDevice(); };
	
	// register known gamepads
	for (auto i = 0u; i < sf::Joystick::Count; ++i) {
		if (sf::Joystick::isConnected(i)) {
			onJoystickConnected(i);
		}
	}
	// re-select device
	if (player.use_gamepad) {
		for (auto i = 1u; i < device_select.size(); ++i) {
			auto buf = device_select[i];
			auto n = buf.rfind("#");
			ASSERT(n != std::string::npos);
			if (player.gamepad_id == std::stoul(buf.substr(n+1))) {
				device_select.setIndex(i);
				break;
			}
		}
	} else {
		device_select.setIndex(0u);
	}
	
	// load player profile
	auto path = player.getKeybindingName();
	if (!player.keys.loadFromFile(path)) {
		context.log.error << "[Game/Profile] Cannot load profile '"
			<< player.filename << "'\n";
		onBackClick();
		return;
	}
	// prepare binding buttons
	keys = player.keys;
	for (auto& pair: bind_labels) {
		auto& bind_btn = menu.acquire<ui::Button>(FIRST_ACTION + static_cast<std::size_t>(pair.first));
		setupButton(bind_btn, "", context, keys.map.get(pair.first).toString());
		bind_btn.activate = [&, pair]() { onWaitBinding(pair.first); };
		setupLabel(pair.second, "controls." + rpg::to_string(pair.first), context);
	}
	
	auto& reset_btn = menu.acquire<ui::Button>(RESET);
	auto& save_btn = menu.acquire<ui::Button>(SAVE);
	auto& discard_btn = menu.acquire<ui::Button>(DISCARD);
	auto& back_btn = menu.acquire<ui::Button>(BACK);
	setupButton(reset_btn, "controls.reset", context);
	setupButton(save_btn, "general.apply", context);
	setupButton(discard_btn, "general.discard", context);
	setupButton(back_btn, "general.back", context);
	reset_btn.activate = [&]() { onResetClick(); };
	save_btn.activate = [&]() { onSaveClick(); };
	discard_btn.activate = [&]() { onBackClick(); };
	back_btn.activate = [&]() { onBackClick(); };
	
	refreshButtons();
	menu.setFocus(device_select);
}

void ControlsEditorState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
	for (auto const & label: bind_labels) {
		target.draw(label.second, states);
	}
	target.draw(warning_label.caption, states);
}

void ControlsEditorState::onJoystickConnected(unsigned int id) {
	auto& context = getContext();
	
	auto ident = sf::Joystick::getIdentification(id);
	context.log.debug << "[State/ControlsEditor] " << "Joystick #" << id
		<< " connected: " << ident.vendorId << ", " << ident.productId
		<< ", " << ident.name.toAnsiString() << "\n";
	
	// register gamepad
	auto& select = menu.query<ui::Select>(DEVICE);
	select.push_back(context.locale("general.gamepad") + " #" + std::to_string(id));
	select.setIndex(select.getIndex());
}

void ControlsEditorState::onJoystickDisconnected(unsigned int id) {
	auto& context = getContext();
	
	context.log.debug << "[State/ControlsEditor] " << "Joystick #" << id << " disconnected\n";
	
	auto& select = menu.query<ui::Select>(DEVICE);
	if (select.getIndex() != 0u) {
		context.log.debug << "[State/ControlsEditor] " << "Profile editor quit\n";
		onBackClick();
		
	} else {
		select.clear();
		select.push_back(context.locale("general.keyboard"));
		for (auto i = 0u; i < sf::Joystick::Count; ++i) {
			if (sf::Joystick::isConnected(i)) {
				select.push_back(context.locale("general.gamepad") + " #" + std::to_string(id));
			}
		}
		select.setIndex(0u);
	}
}

void ControlsEditorState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	auto pad = context.globals.vertical_padding;
	auto hpad = context.globals.horizontal_padding;
	
	auto& device_select = menu.query<ui::Select>(DEVICE);
	auto& reset_btn = menu.query<ui::Button>(RESET);
	auto& save_btn = menu.query<ui::Button>(SAVE);
	auto& discard_btn = menu.query<ui::Button>(DISCARD);
	auto& back_btn = menu.query<ui::Button>(BACK);
	
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	device_select.setPosition({screen_size.x / 2.f, 100.f + 2.f * pad});
	
	// distribute elements in alternating order
	sf::Vector2f pos{screen_size.x / 2.f, 100.f + 3.f * pad};
	auto n = utils::getEnumCount<rpg::PlayerAction>() / 2;
	std::size_t i{0u};
	bool left{true};
	for (auto& pair: bind_labels) {
		auto& bind_btn = menu.query<ui::Button>(FIRST_ACTION + static_cast<std::size_t>(pair.first));
		auto& label = pair.second;
		if (left) {
			ui::setPosition(label, {pos.x - 1.5f * hpad, pos.y});
			bind_btn.setPosition({pos.x - 0.5f * hpad, pos.y});
		} else {
			ui::setPosition(label, {pos.x + 0.5f * hpad, pos.y});
			bind_btn.setPosition({pos.x + 1.5f * hpad, pos.y});
		}
		pos.y += pad;
		++i;
		if (i > n) {
			left = false;
			i = 0;
			pos.y = 100.f + 3.f * pad;
		}
	}
	
	warning_label.caption.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - 3.f * pad});
	reset_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - 2.f * pad});
	save_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - pad});
	discard_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
	back_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
}

void ControlsEditorState::onSelectDevice() {
	auto& select = menu.query<ui::Select>(DEVICE);
	auto index = select.getIndex();
	player.use_gamepad = (index > 0u);
	if (player.use_gamepad) {
		auto buf = select[index];
		auto n = buf.rfind("#");
		ASSERT(n != std::string::npos);
		player.gamepad_id = std::stoul(buf.substr(n+1));
	}
	
	refreshButtons();
}

void ControlsEditorState::onWaitBinding(rpg::PlayerAction action) {
	auto& context = getContext();
	
	auto& bind_btn = menu.query<ui::Button>(FIRST_ACTION + static_cast<std::size_t>(action));
	bind_btn.setString(context.locale("controls.wait"));
	wait_for = std::make_unique<rpg::PlayerAction>(action);
}

void ControlsEditorState::onUpdateBinding(utils::InputAction action) {
	auto& context = getContext();
	
	auto caption = action.toString();
	if (caption == "Unknown") {
		context.log.debug << "[State/ControlsEditor] " << "InputAction not supported\n";
		setWarning(context.locale("controls.unsupported"));
		return;
	}
	
	// filter action
	if (player.use_gamepad) {
		std::size_t id;
		switch (action.type) {
			case utils::InputAction::Key:
				context.log.debug << "[State/ControlsEditor] " << "Keyboard input ignored while waiting for gamepad\n";
				setWarning(context.locale("controls.expect_gamepad"));
				return;
				
			case utils::InputAction::Axis:
				id = action.axis.gamepad_id;
				break;
				
			case utils::InputAction::Button:
				id = action.button.gamepad_id;
				break;
		}
		if (player.gamepad_id != id) {
			context.log.debug << "[State/ControlsEditor] " << "Gamepad input by #" << id
				<< " ignored while listening to gamepad #"
				<< player.gamepad_id << "\n";
			setWarning(context.locale("controls.expect_another_gamepad"));
			return;
		}
	} else {
		if (action.type != utils::InputAction::Key) {
			context.log.debug << "[State/ControlsEditor] " << "Gamepad input ignored while waiting for keyboard\n";
			setWarning(context.locale("controls.expect_keyboard"));
			return;
		}
	}
	
	// update wait button state
	ASSERT(wait_for != nullptr);
	keys.map.set(*wait_for, action);
	auto id = FIRST_ACTION + static_cast<std::size_t>(*wait_for);
	menu.query<ui::Button>(id).setString(caption);
	context.log.debug << rpg::to_string(*wait_for) << " := " << caption << "\n";
	wait_for = nullptr;
	
	refreshButtons();
}

void ControlsEditorState::onResetClick() {
	auto& context = getContext();
	
	auto& device_select = menu.query<ui::Select>(DEVICE);
	
	// reset keys
	if (player.use_gamepad) {
		keys = context.globals.default_gamepad;
	} else {
		keys = context.globals.default_keyboard;
	}
	
	// update bind buttons
	for (auto& pair: bind_labels) {
		auto& bind_btn = menu.query<ui::Button>(FIRST_ACTION + static_cast<std::size_t>(pair.first));
		setupButton(bind_btn, "", context, keys.map.get(pair.first).toString());
	}
	
	refreshButtons();
	menu.setFocus(device_select);
	
	context.log.debug << "[State/ControlsEditor] Reset keys to default "
		<< (player.use_gamepad ? "gamepad" : "keyboard") << " layout\n";
}

void ControlsEditorState::onSaveClick() {
	auto path = player.getKeybindingName();
	
	auto& device_select = menu.query<ui::Select>(DEVICE);
	
	// save profile settings
	player.keys = keys;
	ASSERT(player.keys.saveToFile(path));
	
	refreshButtons();
	menu.setFocus(device_select);
}

void ControlsEditorState::onBackClick() {
	quit();
}

void ControlsEditorState::setWarning(std::string const & msg) {
	warning_label.caption.setString(msg);
	ui::centerify(warning_label.caption);
	warning_label.max_age = sf::seconds(5.f);
	warning_label.decay = 1.f / warning_label.max_age.asMilliseconds();
	warning_label.alpha = 1.f;
}

void ControlsEditorState::refreshButtons() {
	auto& context = getContext();
	
	auto& reset_btn = menu.query<ui::Button>(RESET);
	auto& save_btn = menu.query<ui::Button>(SAVE);
	auto& discard_btn = menu.query<ui::Button>(DISCARD);
	auto& back_btn = menu.query<ui::Button>(BACK);
	
	bool has_changed = !keys.map.isSimilar(player.keys.map);
	
	// hide save / discard buttons if settings changed
	save_btn.setVisible(has_changed);
	discard_btn.setVisible(has_changed);
	// else hide back btn
	back_btn.setVisible(!has_changed);
	
	// hide reset button if key setup is already default
	bool is_custom{false};
	if (player.use_gamepad) {
		is_custom = !keys.map.isSimilar(context.globals.default_gamepad.map);
	} else {
		is_custom = !keys.map.isSimilar(context.globals.default_keyboard.map);
	}
	reset_btn.setVisible(is_custom);
}

void ControlsEditorState::handle(sf::Event const& event) {
	if (wait_for != nullptr) {
		// handle some events
		switch (event.type) {
			case sf::Event::JoystickConnected:
				onJoystickConnected(event.joystickConnect.joystickId);
				menu.refreshMenuControls();
				break;
			
			case sf::Event::JoystickDisconnected:
				onJoystickDisconnected(event.joystickConnect.joystickId);
				menu.refreshMenuControls();
				break;
				
			case sf::Event::JoystickButtonPressed:
				onUpdateBinding(utils::InputAction{
					event.joystickButton.joystickId,
					event.joystickButton.button});
				break;
				
			case sf::Event::JoystickMoved:
				// note: later only the hresehold's sign is used
				onUpdateBinding(utils::InputAction{
					event.joystickMove.joystickId,
					event.joystickMove.axis,
					event.joystickMove.position});
				break;
			
			case sf::Event::KeyPressed:
				onUpdateBinding(utils::InputAction{event.key.code});
				break;
				
			default:
				break;
		}
		
	} else {
		// update regular state 
		menu.handle(event);
		
		switch (event.type) {
			case sf::Event::JoystickConnected:
				onJoystickConnected(event.joystickConnect.joystickId);
				break;
			
			case sf::Event::JoystickDisconnected:
				onJoystickDisconnected(event.joystickConnect.joystickId);
				break;
			
			case sf::Event::Resized:
				onResize({event.size.width, event.size.height});
				break;
				
			case sf::Event::Closed:
				onBackClick();
				break;
				
			default:
				break;
		}
	}
}

void ControlsEditorState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	warning_label.update(elapsed);
	
	menu.update(elapsed);
}

} // ::state
