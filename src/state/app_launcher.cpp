#include <utils/algorithm.hpp>

#include <state/app_launcher.hpp>
#include <state/logo.hpp>
#include <state/tool/toolmenu.hpp>

namespace state {

AppLauncherState::AppLauncherState(App& app, bool tools)
	: LoadThreadState{app}
	, tools{tools}
	, label{}
	, gradient{}
	, progress{0.f} {
	gradient[0.f] = sf::Color::White;
	gradient[0.25f] = sf::Color::Yellow;
	gradient[0.5f] = sf::Color::Red;
	gradient[0.75f] = sf::Color::Yellow;
	gradient[1.f] = sf::Color::White;
	
	auto& context = app.getContext();
	
	// prepare logging
	context.log.debug.add(std::cout);
	context.log.warning.add(std::cout);
	context.log.error.add(std::cout);
	
	// prepare filesystem
	auto path = engine::get_preference_dir();
	if (!utils::file_exists(path)) {
		utils::create_dir(path);
	}
	if (!utils::file_exists(path + "cache/")) {
		utils::create_dir(path + "cache/");
	}
	if (!utils::file_exists(path + "saves/")) {
		utils::create_dir(path + "saves/");
	}
	// load globals
	if (!context.globals.loadFromFile(context.mod.name + "/xml/" + context.globals.getFilename())) {
		context.log.error << "[State/AppLauncher] " << "Cannot load global config: " << context.globals.last_error << "\n";
		context.log.error.flush();
		std::abort();
	}
	
	setupTitle(label, "", context, "Please Wait");
	
	// load settings
	if (!context.settings.loadFromFile(path + context.settings.getFilename())) {
		context.log.debug << "[State/AppLauncher] " << "No settings found, using default values\n";
		context.settings = Settings{};
		if (!context.settings.saveToFile(path + context.settings.getFilename())) {
			context.log.error << "[State/AppLauncher] " << "Cannot save settings!\n";
		}
	}
	
	apply(context.log, app.getWindow(), context.settings, context.globals.framelimit);
	
	// start thread
	start();
}

void AppLauncherState::load() {
	// note: context is not accessed outside while thread isn't finished
	auto& context = getContext();
	auto path = engine::get_preference_dir();
	
	// load localization
	if (!context.locale.loadFromFile(context.mod.name + "/xml/" + context.locale.getFilename())) {
		context.log.error << "[State/AppLauncher] " << "No localization found\n";
	}
	
	// prepare menu stuff
	context.background.setTexture(context.mod.get<sf::Texture>(context.globals.menu_background));
	ui::centerify(context.background);
}

void AppLauncherState::postload() {
	// enter next state
	auto& app = getApplication();
	if (tools) {
		auto launch = std::make_unique<tool::ToolMenuState>(app);
		app.push(launch);
	} else {
		auto launch = std::make_unique<state::LogoState>(app);
		app.push(launch);
	}
}

void AppLauncherState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(label);
}

void AppLauncherState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	label.setPosition({screen_size.x / 2.f, screen_size.y / 2.f});
}

void AppLauncherState::handle(sf::Event const& event) {
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		default:
			break;
	}
}

void AppLauncherState::update(sf::Time const& elapsed) {
	LoadThreadState::update(elapsed);
	
	progress += elapsed.asSeconds() * 0.25f;
	while (progress > 1.f) {
		progress -= 1.f;
	}
	
	label.setColor(gradient.sampleColor(progress));
}

} // ::state
