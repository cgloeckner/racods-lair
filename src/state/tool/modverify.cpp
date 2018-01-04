#include <sstream>

#include <utils/filesystem.hpp>
#include <game/mod.hpp>
#include <ui/imgui.hpp>
#include <state/tool/modverify.hpp>

namespace tool {

ModVerifyState::ModVerifyState(state::App& app)
	: state::State{app}
	, modlog{}
	, modname{"./data"}
	, result{} {
}

void ModVerifyState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	ImGui::Render();
}

void ModVerifyState::onVerifyClick() {
	modlog.clear();
	if (!utils::file_exists(modname)) {
		modlog.appendf("Not found\n");
		result = "Mod " + modname + " was not found :(";
		return;
	}
	
	core::LogContext unused;
	utils::Logger log;
	std::stringstream output;
	log.add(output);
	
	game::ResourceCache cache;
	game::Mod mod{unused, cache, modname};
	bool success{true};
	try {
		mod.preload();
		success = mod.verify(log);
	} catch (std::exception const & err) {
		modlog.appendf("%s", err.what());
		success = false;
	}
	
	if (success) {
		result = "Mod " + modname + " was successfully checked :)";
	} else {
		result = "Mod " + modname + " is incomplete :S";
	}
	
	modlog.appendf("%s", output.str().c_str());
}

void ModVerifyState::onBackClick() {
	quit();
}

void ModVerifyState::handle(sf::Event const & event) {
	ImGui::SFML::ProcessEvent(event);
	
	switch (event.type) {
		case sf::Event::Closed:
			onBackClick();
			break;
			
		default:
			break;
	}
}

void ModVerifyState::update(sf::Time const & elapsed) {
	ImGui::SFML::Update();
	
	auto size = getApplication().getWindow().getSize();
	ImGui::SetNextWindowSize(ImVec2(size * 3u / 4u));
	ImGui::SetNextWindowPosCenter();
	
	ImGui::Begin("Mod Verification");
	
		ui::InputText("Mod path", modname);
		if (ImGui::Button("Verify")) {
			onVerifyClick();
		}
		if (ImGui::Button("Back")) {
			onBackClick();
		}
		
		if (!modlog.empty()) {
			ImGui::Text("Result: %s", result.c_str());
				ImGui::BeginChild("Log", ImVec2(0, 0), true);
				ImGui::TextUnformatted(modlog.begin());
			ImGui::EndChild();
		}
		
	ImGui::End();
}

} // ::tool
