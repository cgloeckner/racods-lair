#include <ui/button.hpp>
#include <ui/checkbox.hpp>
#include <ui/select.hpp>

#include <state/title.hpp>
#include <state/settings.hpp>

namespace state {

static std::size_t const RESOLUTION = 0u;
static std::size_t const FULLSCREEN = 1u;
static std::size_t const LIGHTING = 2u;
static std::size_t const AUTOCAM = 3u;
static std::size_t const AUTOSAVE = 4u;
static std::size_t const SOUND = 5u;
static std::size_t const MUSIC = 6u;
static std::size_t const APPLY = 10u;
static std::size_t const DISCARD = 11u;
static std::size_t const BACK = 12u;

// --------------------------------------------------------------------

SettingsState::SettingsState(App& app)
	: State{app}
	, menu{}
	, sound{}
	, title_label{} 
	, resolution_label{}
	, lighting_label{}
	, sound_label{}
	, music_label{}
	, latest{} {
	auto& context = app.getContext();
	latest = context.settings;
	
	// setup widgets
	setupTitle(title_label, "settings.title", context);
	setupLabel(resolution_label, "settings.resolution", context);
	auto& resolution_select = menu.acquire<ui::Select>(RESOLUTION);
	auto& fullscreen_check = menu.acquire<ui::Checkbox>(FULLSCREEN);
	setupSelect(resolution_select, context);
	setupCheckbox(fullscreen_check, "settings.fullscreen", context);
	setupLabel(lighting_label, "settings.lighting.level", context);
	setupLabel(sound_label, "settings.sound", context);
	setupLabel(music_label, "settings.music", context);
	auto& lighting_select = menu.acquire<ui::Select>(LIGHTING);
	auto& autocam_check = menu.acquire<ui::Checkbox>(AUTOCAM);
	auto& autosave_check = menu.acquire<ui::Checkbox>(AUTOSAVE);
	auto& sound_select = menu.acquire<ui::Select>(SOUND);
	auto& music_select = menu.acquire<ui::Select>(MUSIC);
	auto& apply_btn = menu.acquire<ui::Button>(APPLY);
	auto& discard_btn = menu.acquire<ui::Button>(DISCARD);
	auto& back_btn = menu.acquire<ui::Button>(BACK);
	setupSelect(lighting_select, context);
	setupSelect(sound_select, context);
	setupSelect(music_select, context);
	setupCheckbox(autocam_check, "settings.autocam", context);
	setupCheckbox(autosave_check, "settings.autosave", context);
	setupButton(apply_btn, "general.apply", context);
	setupButton(discard_btn, "general.discard", context);
	setupButton(back_btn, "general.back", context);
	resolution_select.change = [&]() { onSetResolution(); };
	fullscreen_check.activate = [&]() { onSetFullscreen(); };
	lighting_select.change = [&]() { onSetLighting(); };
	autocam_check.activate = [&]() { onSetAutoCam(); };
	autosave_check.activate = [&]() { onSetAutoSave(); };
	sound_select.change = [&]() { onSetSound(); };
	music_select.change = [&]() { onSetMusic(); };
	apply_btn.activate = [&]() { onApplyClick(); };
	discard_btn.activate = [&]() { onDiscardClick(); };
	back_btn.activate = [&]() { onDiscardClick(); };
	apply_btn.setVisible(false);
	discard_btn.setVisible(false);
	menu.setFocus(resolution_select);
	
	// set fullscreen and autocam flags
	fullscreen_check.setChecked(latest.fullscreen);
	autocam_check.setChecked(latest.autocam);
	autosave_check.setChecked(latest.autosave);
	// offer supported resolutions
	std::size_t index{0u};
	auto modes = sf::VideoMode::getFullscreenModes();
	utils::reverse(modes);
	if (modes.front().width > MIN_SCREEN_WIDTH
		&& modes.front().height > MIN_SCREEN_HEIGHT) {
		// workaround: something is broken with display modes
		resolution_select.push_back(std::to_string(MIN_SCREEN_WIDTH)
			+ "x" + std::to_string(MIN_SCREEN_HEIGHT) + ":32");
	}
	for (auto const & mode: modes) {
		auto dump = std::to_string(mode.width) + "x"
			+ std::to_string(mode.height) + ":"
			+ std::to_string(mode.bitsPerPixel);
		if (mode.width < MIN_SCREEN_WIDTH || mode.height < MIN_SCREEN_HEIGHT) {
			context.log.debug << "[State/Settings] Resolution " << dump
				<< " skipped because of minimum resolution\n";
			continue;
		}
		resolution_select.push_back(dump);
		if (mode == latest.resolution) {
			index = resolution_select.size() - 1u;
		}
	}
	resolution_select.setIndex(index);
	sound_select.reserve(11u);
	music_select.reserve(11u);
	for (auto i = 0u; i <= 10; ++i) {
		auto str = std::to_string(i * 10) + "%";
		sound_select.push_back(str);
		music_select.push_back(str);
	}
	sound_select.setIndex(static_cast<std::size_t>(latest.sound / 10));
	music_select.setIndex(static_cast<std::size_t>(latest.music / 10));
	
	// offer supported lighting levels
	lighting_select.push_back(context.locale("settings.lighting.disabled"));
	lighting_select.push_back(context.locale("settings.lighting.simple"));
	lighting_select.push_back(context.locale("settings.lighting.complex"));
	lighting_select.setIndex(latest.lighting);
	
	// prepare audio preview
	sound.setBuffer(context.mod.get<sf::SoundBuffer>(context.globals.sfx_volume_preview));
}

void SettingsState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(menu, states);
	target.draw(title_label, states);
	target.draw(resolution_label, states);
	target.draw(lighting_label, states);
	target.draw(sound_label, states);
	target.draw(music_label, states);
}

void SettingsState::onSettingsChanged() {
	auto& apply_btn = menu.query<ui::Button>(APPLY);
	auto& discard_btn = menu.query<ui::Button>(DISCARD);
	auto& back_btn = menu.query<ui::Button>(BACK);
	
	if (getContext().settings != latest) {
		apply_btn.setVisible(true);
		discard_btn.setVisible(true);
		back_btn.setVisible(false);
	} else {
		apply_btn.setVisible(false);
		discard_btn.setVisible(false);
		back_btn.setVisible(true);
	}
}

void SettingsState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	auto& context = getContext();
	auto pad = context.globals.vertical_padding;
	auto hpad = context.globals.horizontal_padding;
	
	auto& resolution_select = menu.query<ui::Select>(RESOLUTION);
	auto& fullscreen_check = menu.query<ui::Checkbox>(FULLSCREEN);
	auto& lighting_select = menu.query<ui::Select>(LIGHTING);
	auto& autocam_check = menu.query<ui::Checkbox>(AUTOCAM);
	auto& autosave_check = menu.query<ui::Checkbox>(AUTOSAVE);
	auto& sound_select = menu.query<ui::Select>(SOUND);
	auto& music_select = menu.query<ui::Select>(MUSIC);
	auto& apply_btn = menu.query<ui::Button>(APPLY);
	auto& discard_btn = menu.query<ui::Button>(DISCARD);
	auto& back_btn = menu.query<ui::Button>(BACK);
	ui::setPosition(title_label, {screen_size.x / 2.f, 100.f});
	ui::setPosition(resolution_label, {screen_size.x / 2.f - hpad, 100.f + 2.f * pad});
	resolution_select.setPosition({screen_size.x / 2.f, 100.f + 2.f * pad});
	fullscreen_check.setPosition({screen_size.x / 2.f, 100.f + 3.f * pad});
	ui::setPosition(lighting_label, {screen_size.x / 2.f - hpad, 100.f + 4.f * pad});
	lighting_select.setPosition({screen_size.x / 2.f, 100.f + 4.f * pad});
	autocam_check.setPosition({screen_size.x / 2.f, 100.f + 5.f * pad});
	autosave_check.setPosition({screen_size.x / 2.f, 100.f + 6.f * pad});
	ui::setPosition(sound_label, {screen_size.x / 2.f - hpad, 100.f + 7.f * pad});
	sound_select.setPosition({screen_size.x / 2.f, 100.f + 7.f * pad});
	ui::setPosition(music_label, {screen_size.x / 2.f - hpad, 100.f + 8.f * pad});
	music_select.setPosition({screen_size.x / 2.f, 100.f + 8.f * pad});
	apply_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f - pad});
	discard_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
	back_btn.setPosition({screen_size.x / 2.f, screen_size.y - 100.f});
}

void SettingsState::onSetResolution() {
	auto& select = menu.query<ui::Select>(RESOLUTION);
	auto value = select[select.getIndex()];
	auto x = value.find("x");
	auto y = value.find(":");
	ASSERT(x != std::string::npos);
	latest.resolution.width = std::stoul(value.substr(0, x));
	latest.resolution.height = std::stoul(value.substr(x+1, y));
	latest.resolution.bitsPerPixel = std::stoul(value.substr(y+1));
	
	onSettingsChanged();
}

void SettingsState::onSetFullscreen() {
	auto& check = menu.query<ui::Checkbox>(FULLSCREEN);
	latest.fullscreen = check.isChecked();
	
	onSettingsChanged();
}

void SettingsState::onSetLighting() {
	auto& select = menu.query<ui::Select>(LIGHTING);
	latest.lighting = select.getIndex();
	
	onSettingsChanged();
}

void SettingsState::onSetAutoCam() {
	auto& check = menu.query<ui::Checkbox>(AUTOCAM);
	latest.autocam = check.isChecked();
	
	onSettingsChanged();
}

void SettingsState::onSetAutoSave() {
	auto& check = menu.query<ui::Checkbox>(AUTOSAVE);
	latest.autosave = check.isChecked();
	
	onSettingsChanged();
}

void SettingsState::onSetSound() {
	auto& context = getContext();
	latest.sound = menu.query<ui::Select>(SOUND).getIndex() * 10.f;
	
	context.sfx.setVolume(latest.sound);
	sound.setVolume(latest.sound);
	if (sound.getStatus() != sf::SoundSource::Status::Playing) {
		sound.play();
	}
	
	onSettingsChanged();
}

void SettingsState::onSetMusic() {
	auto& context = getContext();
	latest.music = menu.query<ui::Select>(MUSIC).getIndex() * 10.f;
	
	context.theme.setVolume(latest.music);
	
	onSettingsChanged();
}

void SettingsState::onApplyClick() {
	auto& context = getContext();
	
	// apply settings
	if (context.settings.resolution != latest.resolution || context.settings.fullscreen != latest.fullscreen) {
		apply(context.log, getApplication().getWindow(), latest, context.globals.framelimit);
	}
	
	// save settings
	context.settings = latest;
	auto path = engine::get_preference_dir();
	if (!context.settings.saveToFile(path + context.settings.getFilename())) {
		context.log.error << "[State/Settings] " << "Cannot save settings\n";
		context.log.error.flush();
	}
	
	// apply settings to game
	if (context.game != nullptr) {
		context.game->applySettings();
	}
	
	quit();
}

void SettingsState::onDiscardClick() {
	auto& context = getContext();
	
	context.sfx.setVolume(context.settings.sound);
	context.theme.setVolume(context.settings.music);
	
	quit();
}

void SettingsState::handle(sf::Event const& event) {
	menu.handle(event);
	
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		case sf::Event::Closed:
			onDiscardClick();
			break;
			
		case sf::Event::JoystickConnected:
		case sf::Event::JoystickDisconnected:
			menu.refreshMenuControls();
			break;
			
		default:
			break;
	}
}

void SettingsState::update(sf::Time const& elapsed) {
	
	getContext().update(elapsed);
	menu.update(elapsed);
}

} // ::state
