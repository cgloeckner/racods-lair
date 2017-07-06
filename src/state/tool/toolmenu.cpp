#include <ui/imgui.hpp>
#include <state/tool/toolmenu.hpp>
#include <state/tool/modverify.hpp>
#include <state/tool/spritepacker.hpp>
#include <state/tool/spriteviewer.hpp>
#include <state/tool/roomeditor.hpp>
#include <state/tool/savegameviewer.hpp>

namespace tool {

ToolMenuState::ToolMenuState(state::App& app)
	: state::State{app} {
}

void ToolMenuState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	ImGui::Render();
}

void ToolMenuState::onVerifyClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<ModVerifyState>(app);
	app.push(launch);
}

void ToolMenuState::onPackerClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<SpritePackerState>(app);
	app.push(launch);
}

void ToolMenuState::onViewerClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<SpriteViewerState>(app);
	app.push(launch);
}

void ToolMenuState::onRoomClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<RoomEditorState>(app);
	app.push(launch);
}

void ToolMenuState::onSavegameClick() {
	auto& app = getApplication();
	auto launch = std::make_unique<SavegameViewerState>(app);
	app.push(launch);
}

void ToolMenuState::onQuitClick() {
	quit();
}

void ToolMenuState::handle(sf::Event const & event) {
	ImGui::SFML::ProcessEvent(event);
	
	switch (event.type) {
		case sf::Event::Closed:
			onQuitClick();
			break;
			
		default:
			break;
	}
}

void ToolMenuState::update(sf::Time const & elapsed) {
	ImGui::SFML::Update();
	
	auto size = getApplication().getWindow().getSize();
	ImGui::SetNextWindowSize(ImVec2(size * 3u / 4u));
	ImGui::SetNextWindowPosCenter();
	
	ImGui::Begin("Development tools");
	
		if (ImGui::Button("Mod verification")) {
			onVerifyClick();
		}
		if (ImGui::Button("Sprite packer")) {
			onPackerClick();
		}
		if (ImGui::Button("Sprite viewer")) {
			onViewerClick();
		}
		if (ImGui::Button("Room editor")) {
			onRoomClick();
		}
		if (ImGui::Button("Savegame viewer")) {
			onSavegameClick();
		}
		if (ImGui::Button("Quit")) {
			onQuitClick();
		}
		
	ImGui::End();
}

} // ::tool
