#include <ui/button.hpp>
#include <ui/select.hpp>

#include <state/camera_editor.hpp>

namespace state {

static std::size_t const FIRST_PLAYER = 0u;

// --------------------------------------------------------------------

CameraEditorState::CameraEditorState(App& app)
	: State{app}
	, OK{FIRST_PLAYER + getContext().globals.max_num_players}
	, menu{}
	, title_label{} 
	, cam_labels{} {
	auto& context = app.getContext();
	ASSERT(context.game != nullptr);
	auto& game = *context.game;
	
	// setup widgets
	setupTitle(title_label, "camera.title", context);
	
	std::size_t i{0u};
	for (auto& profile: game.lobby.players) {
		ASSERT(profile.id > 0u);
		// setup label
		auto name = context.game->engine.physics.focus.query(profile.id).display_name;
		cam_labels.emplace_back();
		auto& label = cam_labels.back();
		setupLabel(label, "", context, name);
		// setup select widget
		auto& select = menu.acquire<ui::Select>(FIRST_PLAYER + i);
		setupSelect(select, context);
		for (auto j = 0u; j < game.lobby.num_players; ++j) {
			select.push_back(context.locale("camera.camera") + " " + std::to_string(j+1));
		}
		
		// select current camera
		auto const & cam = game.engine.session.camera.query(profile.id);
		auto owner = cam.objects.front();
		std::size_t j{0u};
		bool set{false};
		for (auto const & player: game.lobby.players) {
			if (owner == player.id) {
				set = true;
				select.setIndex(j);
				break;
			}
			++j;
		}
		ASSERT(set);
		++i;
	}
	
	auto& ok_btn = menu.acquire<ui::Button>(OK);
	setupButton(ok_btn, "general.ok", context);
	ok_btn.activate = [&]() { onOkClick(); };
	menu.setFocus(ok_btn);
}

void CameraEditorState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
	
	for (auto& label: cam_labels) {
		target.draw(label, states);
	}
}

void CameraEditorState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	ASSERT(context.game != nullptr);
	auto& game = *context.game;
	ASSERT(cam_labels.size() == game.lobby.num_players);
	
	auto pad = context.globals.vertical_padding;
	auto hpad = context.globals.horizontal_padding;
	
	auto& ok_btn = menu.query<ui::Button>(OK);
	
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	auto n = game.lobby.num_players;
	for (auto i = 0u; i < n; ++i) {
		auto& label = cam_labels[i];
		label.setPosition({screen_size.x / 2.f - hpad, screen_size.y - 100.f - (2.f + n - i) * pad});
		auto& select = menu.query<ui::Select>(FIRST_PLAYER + i);
		select.setPosition({screen_size.x / 2.f + hpad, screen_size.y - 100.f - (2.f + n - i) * pad});
	}
	ok_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
}

void CameraEditorState::onOkClick() {
	auto& context = getContext();
	ASSERT(context.game != nullptr);
	auto& game = *context.game;
	auto& cam_system = game.engine.session.camera;
	
	std::vector<utils::CameraData<core::ObjectID>*> cams;
	cams.resize(game.lobby.num_players, nullptr);
	
	// reinitialize camera system
	cam_system.clear();
	std::size_t i{0u};
	for (auto const & player: game.lobby.players) {;
		auto& select = menu.query<ui::Select>(FIRST_PLAYER + i);
		auto index = select.getIndex();
		
		// add player to desired camera
		if (cams[index] == nullptr) {
			cams[index] = &cam_system.acquire();
		}
		cams[index]->objects.push_back(player.id);
		
		context.log.debug << "[State/Camera] " << "Added '" << player.tpl.display_name
			<< "' to camera " << index << "\n";
		
		ASSERT(cam_system.has(player.id));
		++i;
	}
	
	quit();
}

void CameraEditorState::handle(sf::Event const& event) {
	menu.handle(event);
	
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		case sf::Event::Closed:
			onOkClick();
			break;
			
		case sf::Event::JoystickConnected:
		case sf::Event::JoystickDisconnected:
			menu.refreshMenuControls();
			break;
			
		default:
			break;
	}
}

void CameraEditorState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	menu.update(elapsed);
}

} // ::state
