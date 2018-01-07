#include <utils/algorithm.hpp>
#include <utils/filesystem.hpp>

#include <ui/imgui.hpp>
#include <state/tool/savegameviewer.hpp>

namespace tool {

SavegameState::SavegameState(game::Mod& mod, std::string const & filename)
	: mod{mod}
	, filename{filename}
	, player{} {
	ASSERT(player.loadFromFile(engine::get_preference_dir() + "saves/"
		+ filename + ".sav"));
}

void SavegameState::update() {
	ImGui::Columns(2, "savegame-columns");
	ImGui::Separator();
	
	ui::showPair("Display Name", player.display_name);
	ui::showPair("Entity Name", player.entity_name);
	
	ImGui::Text("Inventory");
	ImGui::NextColumn();
	for (auto& node: player.inventory) {
		ImGui::BulletText("%s %ux", std::get<0>(node).c_str(), std::get<1>(node));
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Text("Equipment");
	ImGui::NextColumn();
	for (auto& pair: player.equipment) {
		if (pair.first == rpg::EquipmentSlot::None) {
			continue;
		}
		ImGui::BulletText("%s: %s", to_string(pair.first).c_str(), pair.second.c_str());
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Text("Perks");
	ImGui::NextColumn();
	for (auto& node: player.perks) {
		ImGui::BulletText("%s lvl %u", std::get<0>(node).c_str(), std::get<1>(node));
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Text("Attributes");
	ImGui::NextColumn();
	for (auto& pair: player.attributes) {
		ImGui::BulletText("%s: %u", to_string(pair.first).c_str(), pair.second);
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ImGui::Text("Quickslots");
	ImGui::NextColumn();
	std::size_t i{0u};
	for (auto& node: player.slots) {
		ImGui::BulletText("#%lu: %s", i++, std::get<0>(node).empty() ? std::get<1>(node).c_str() : std::get<0>(node).c_str());
	}
	ImGui::NextColumn();
	ImGui::Separator();
	
	ui::showPair("Current Slot ID", std::to_string(player.slot_id));
	ui::showPair("Level", std::to_string(player.level));
	ui::showPair("Experience", std::to_string(player.exp));
	ui::showPair("Attribute Points", std::to_string(player.attrib_points));
	ui::showPair("Perk Points", std::to_string(player.perk_points));
	
	ImGui::Columns();
}

// --------------------------------------------------------------------

SavegameViewerState::SavegameViewerState(state::App& app)
	: state::State{app}
	, log{}
	, cache{}
	, modpath{"./data"}
	, index{-1}
	, profiles{}
	, mod{nullptr}
	, current{nullptr} {
	auto& context = app.getContext();
	
	// fetch all savegame names
	auto saves_dir = engine::get_preference_dir() + "saves";
	utils::for_each_file(saves_dir, ".sav", [&](std::string const & p, std::string const & fname) {
		if (!utils::file_exists(saves_dir + "/" + fname + ".xml")) {
			context.log.warning << "[State/SavegameViewer] Profile '"
				<< fname << "' lacks keybinding\n";
			return;
		}
		profiles.push_back(fname);
	});
	
	onModSpecify();
}

void SavegameViewerState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	ImGui::Render();
}

void SavegameViewerState::onModSpecify() {
	try {
		mod = std::make_unique<game::Mod>(log, cache, modpath);
	} catch (...) {
		mod = nullptr;
	}
}

void SavegameViewerState::onProfileSelect() {
	current = std::make_unique<SavegameState>(*mod, profiles[index]);
}

void SavegameViewerState::onBackClick() {
	// tba: check saved
	
	quit();
}

void SavegameViewerState::handle(sf::Event const & event) {
	ImGui::SFML::ProcessEvent(event);
	
	switch (event.type) {
		case sf::Event::Closed:
			onBackClick();
			break;
			
		default:
			break;
	}
}

void SavegameViewerState::update(sf::Time const & elapsed) {
	auto& window = getApplication().getWindow();
	
	ImGui::SFML::Update(window, elapsed);
	
	auto size = window.getSize();
	ImGui::SetNextWindowSize(ImVec2(size * 3u / 4u));
	ImGui::SetNextWindowPosCenter();
	
	ImGui::Begin("Savegame Viewer");
		if (ui::InputText("Mod path", modpath)) {
			onModSpecify();
		}
		if (ui::Combo("Profiles", index, profiles) && mod != nullptr && index >= 0) {
			onProfileSelect();
		}
		
		if (mod != nullptr && current != nullptr) {
			current->update();
		}
		
		if (ImGui::Button("Back")) {
			onBackClick();
		}
	ImGui::End();
}

} // ::tool
