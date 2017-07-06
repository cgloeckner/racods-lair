#include <string>

#include <ui/input.hpp>
#include <ui/select.hpp>
#include <ui/button.hpp>

#include <game/player.hpp>
#include <state/profile_creator.hpp>
#include <state/savegame_menu.hpp>

namespace state {

static std::size_t const FILENAME = 0u;
static std::size_t const CHARNAME = 1u;
static std::size_t const CHARCLASS = 2u;
static std::size_t const CREATE = 10u;
static std::size_t const BACK = 11u;

// --------------------------------------------------------------------

ProfileCreatorState::ProfileCreatorState(App& app, LobbyContext::Player& player)
	: State{app}
	, player{player}
	, menu{}
	, title_label{}
	, filename_label{}
	, charname_label{}
	, warning_label{} {
	auto& context = app.getContext();
	
	setupTitle(title_label, "profile.title", context);
	
	setupLabel(filename_label, "general.filename", context);
	setupLabel(charname_label, "general.charname", context);
	auto& filename_input = menu.acquire<ui::FilenameInput>(FILENAME);
	auto& charname_input = menu.acquire<ui::Input>(CHARNAME);
	auto& charclass_select = menu.acquire<ui::Select>(CHARCLASS);
	setupInput(filename_input, "profile.default_name", context);
	setupInput(charname_input, "profile.default_name", context);
	{
		auto& whitelist = charname_input.whitelist;
		whitelist.reserve(10u + 2u * 26u + 1u);
		for (sf::Uint32 i = 0u; i < 10u; ++i) {
			whitelist.push_back(i + u'0');
		}
		for (sf::Uint32 i = 0u; i < 26u; ++i) {
			whitelist.push_back(i + u'a');
			whitelist.push_back(i + u'A');
		}
		whitelist.push_back(u' ');
	}
	setupSelect(charclass_select, context);
	charclass_select.push_back(context.locale("profile.charclass.warrior"));
	charclass_select.push_back(context.locale("profile.charclass.rogue"));
	charclass_select.push_back(context.locale("profile.charclass.wizard"));
	charclass_select.setIndex(0u);
	setupWarning(warning_label.caption, context);
	
	auto& create_btn = menu.acquire<ui::Button>(CREATE);
	auto& back_btn = menu.acquire<ui::Button>(BACK);
	setupButton(create_btn, "profile.create", context);
	setupButton(back_btn, "general.back", context);
	create_btn.activate = [&]() { onCreateClick(); };
	back_btn.activate = [&]() { onBackClick(); };
}

void ProfileCreatorState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
	target.draw(filename_label, states);
	target.draw(charname_label, states);
	target.draw(warning_label.caption, states);
}

void ProfileCreatorState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	auto pad = context.globals.vertical_padding;
	auto hpad = context.globals.horizontal_padding;
	
	auto& filename_input = menu.query<ui::FilenameInput>(FILENAME);
	auto& charname_input = menu.query<ui::Input>(CHARNAME);
	auto& charclass_select = menu.query<ui::Select>(CHARCLASS);
	auto& create_btn = menu.query<ui::Button>(CREATE);
	auto& back_btn = menu.query<ui::Button>(BACK);
	
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	ui::setPosition(filename_label, {screen_size.x / 2.f - hpad, 100.f + 2.f * pad});
	filename_input.setPosition({screen_size.x / 2.f, 100.f + 2.f * pad});
	ui::setPosition(charname_label, {screen_size.x / 2.f - hpad, 100.f + 3.f * pad});
	charname_input.setPosition({screen_size.x / 2.f, 100.f + 3.f * pad});
	charclass_select.setPosition({screen_size.x / 2.f, 100.f + 4.f * pad});
	
	warning_label.caption.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - 2.f * pad});
	create_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - pad});
	back_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
}

void ProfileCreatorState::onCreateClick() {
	auto& context = getContext();
	
	auto filename = menu.query<ui::FilenameInput>(FILENAME).getString();
	auto charname = menu.query<ui::Input>(CHARNAME).getString();
	auto class_index = menu.query<ui::Select>(CHARCLASS).getIndex();
	if (filename.size() < 3u) {
		context.log.debug << "[State/ProfileCreator] Too short filename\n";
		setWarning(context.locale("profile.too_short_filename"));
		return;
	}
	player.filename = filename;
	auto sav_path = player.getSavegameName();
	auto key_path = player.getKeybindingName();
	if (utils::file_exists(sav_path) || utils::file_exists(key_path)) {
		// Rollback filename assignment
		player.filename.clear();
		context.log.debug << "[State/ProfileCreator] " << filename << " does already exist\n";
		setWarning(context.locale("profile.filename_already_used"));
		return;
	}
	if (charname.size() < 3u) {
		// Rollback filename assignment
		player.filename.clear();
		context.log.debug << "[State/ProfileCreator] Too short charname\n";
		setWarning(context.locale("profile.too_short_charname"));
		return;
	}
	
	// create initial profile
	player = LobbyContext::Player{};
	player.tpl = game::createPlayer(charname, 0u);
	
	// --- HARDCODED SESSION (ALPHA DEMO CONTENT) ---------------------
	// setup character class
	switch (class_index) {
		case 0u:
			// create warrior
			player.tpl.inventory.emplace_back("sword", 1u, nullptr);
			player.tpl.inventory.emplace_back("armor", 1u, nullptr);
			player.tpl.inventory.emplace_back("shield", 1u, nullptr);
			player.tpl.inventory.emplace_back("ceremonial-helmet", 1u, nullptr);
			player.tpl.equipment[rpg::EquipmentSlot::Weapon] = "sword";
			player.tpl.equipment[rpg::EquipmentSlot::Body] = "armor";
			player.tpl.equipment[rpg::EquipmentSlot::Extension] = "shield";
			player.tpl.equipment[rpg::EquipmentSlot::Head] = "ceremonial-helmet";
			context.log.debug << "[State/ProfileCreator] Created warrior\n";
			break;
			
		case 1u:
			// create rogue
			player.tpl.inventory.emplace_back("bow", 1u, nullptr);
			player.tpl.inventory.emplace_back("leather", 1u, nullptr);
			player.tpl.inventory.emplace_back("cowl", 1u, nullptr);
			player.tpl.equipment[rpg::EquipmentSlot::Weapon] = "bow";
			player.tpl.equipment[rpg::EquipmentSlot::Body] = "leather";
			player.tpl.equipment[rpg::EquipmentSlot::Head] = "cowl";
			
			context.log.debug << "[State/ProfileCreator] Created rogue\n";
			break;
			
		case 2u:
			// create wizard
			player.tpl.inventory.emplace_back("staff", 1u, nullptr);
			player.tpl.inventory.emplace_back("leather", 1u, nullptr);
			player.tpl.inventory.emplace_back("hat", 1u, nullptr);
			player.tpl.equipment[rpg::EquipmentSlot::Weapon] = "staff";
			player.tpl.equipment[rpg::EquipmentSlot::Body] = "leather";
			player.tpl.equipment[rpg::EquipmentSlot::Head] = "hat";
			player.tpl.perks.emplace_back("fireball", 1u, nullptr);
			for (auto& slot: player.tpl.slots) {
				std::get<1>(slot) = "fireball";
			}
			context.log.debug << "[State/ProfileCreator] Created wizard\n";
			break;
			
		default:
			context.log.error << "[State/ProfileCreator] Invalid character class\n";
			quit();
	}
	// ----------------------------------------------------------------
	
	// setup input binding and save profile
	player.keys = context.globals.default_keyboard;
	ASSERT(player.tpl.saveToFile(sav_path));
	ASSERT(player.keys.saveToFile(key_path));
	player.filename = filename;
	
	context.log.debug << "[State/ProfileCreator] '" << player.filename
		<< "' created\n";
	
	auto& app = getApplication();
	auto launch = std::make_unique<SavegameMenuState>(app, player, true);
	app.push(launch);
}

void ProfileCreatorState::onBackClick() {
	quit();
}

void ProfileCreatorState::setWarning(std::string const & msg) {
	warning_label.caption.setString(msg);
	ui::centerify(warning_label.caption);
	warning_label.max_age = sf::seconds(5.f);
	warning_label.decay = 1.f / warning_label.max_age.asMilliseconds();
	warning_label.alpha = 1.f;
}

void ProfileCreatorState::handle(sf::Event const& event) {
	menu.handle(event);
	
	switch (event.type) {
		case sf::Event::JoystickConnected:
		case sf::Event::JoystickDisconnected:
			menu.refreshMenuControls();
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

void ProfileCreatorState::update(sf::Time const& elapsed) {
	getContext().update(elapsed);
	warning_label.update(elapsed);
	
	menu.update(elapsed);
}

} // ::state
