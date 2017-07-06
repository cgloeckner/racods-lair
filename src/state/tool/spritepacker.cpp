#include <sstream>

#include <utils/filesystem.hpp>
#include <rpg/spritepacker.hpp>
#include <ui/imgui.hpp>
#include <state/tool/spritepacker.hpp>

namespace tool {

SpritePackerState::SpritePackerState(state::App& app)
	: state::State{app}
	, packlog{}
	, source{"./_work"}
	, target{"./data"}
	, result{} {
}

void SpritePackerState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	ImGui::Render();
}

void SpritePackerState::onPackClick() {
	packlog.clear();
	if (!utils::file_exists(source)) {
		packlog.append("%s not found\n", source.c_str());
		result = "Source path '" + source + "' not found :S";
		return;
	}
	if (!utils::file_exists(target)) {
		packlog.append("%s not found\n", target.c_str());
		result = "Target path '" + target + "' not found :S";
		return;
	}
	
	core::LogContext log;
	std::stringstream output;
	log.debug.add(output);
	log.debug.add(std::cout);
	rpg::SpritePacker packer{source, target, log.debug};
	
	bool success{true};
	std::size_t n{0u};
	utils::for_each_file(source, ".xml", [&](std::string const & p, std::string const & fname) {
		if (!success) {
			// skip others
		}
		if (!p.empty()) {
			// skipping subdirectories
			return;
		}
		++n;
		try {
			log.debug << "Building " << fname << " from " << source
				<< "/" << fname << ".xml\n";
			if (!packer(fname)) {
				success = false;
				log.debug << "Failed\n";
			}
			log.debug << "Ok\n";
		} catch (std::exception const & err) {
			success = false;
			log.debug << "Abort: " << err.what() << "\n";
		}
	});
	
	if (success) {
		result = "Sprites from " + source + " were successfully packed to "
			+ target + " :)";
	} else {
		result = "Packing sprites from " + source + " to " + target
			+ " failed :S";
	}
	if (n == 0u) {
		log.debug << "No sprites found\n";
	}
	
	packlog.append("%s", output.str().c_str());
}

void SpritePackerState::onBackClick() {
	quit();
}

void SpritePackerState::handle(sf::Event const & event) {
	ImGui::SFML::ProcessEvent(event);
	
	switch (event.type) {
		case sf::Event::Closed:
			onBackClick();
			break;
			
		default:
			break;
	}
}

void SpritePackerState::update(sf::Time const & elapsed) {
	ImGui::SFML::Update();
	
	auto size = getApplication().getWindow().getSize();
	ImGui::SetNextWindowSize(ImVec2(size * 3u / 4u));
	ImGui::SetNextWindowPosCenter();
	
	ImGui::Begin("Sprite Packer");
	
		ui::InputText("Source path", source);
		ui::InputText("Target path", target);
		if (ImGui::Button("Pack")) {
			onPackClick();
		}
		if (ImGui::Button("Back")) {
			onBackClick();
		}
		
		if (!packlog.empty()) {
			ImGui::Text("Result: %s", result.c_str());
				ImGui::BeginChild("Log", ImVec2(0, 0), true);
				ImGui::TextUnformatted(packlog.begin());
			ImGui::EndChild();
		}
		
	ImGui::End();
}

} // ::tool
