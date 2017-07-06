#include <locale.h>
#include <iostream>
#include <vector>

#include <ui/imgui.hpp>
#include <state/app_launcher.hpp>

#include <engine/event.hpp>

//#include <tools.hpp>

#if defined(SFML_SYSTEM_LINUX)
#include <X11/Xlib.h>
#endif

int main(int argc, char** argv) {
#if defined(SFML_SYSTEM_LINUX) 
	XInitThreads();
#endif
	/*
	sf::RenderWindow window{{800, 600}, "test"};
	ImGui::SFML::Init(window);
	window.setFramerateLimit(60u);
	
	unsigned long long a{1337llu};
	unsigned long long b{46llu};
	
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}
		ImGui::SFML::Update();
		ui::InputNumber("test", a);
		ui::InputNumber("foo", b);
		
		window.clear(sf::Color::Black);
		ImGui::Render();
		window.display();
	}
	ImGui::SFML::Shutdown();
	*/
	
	/*
	if (argc == 2) {
		auto cmd = std::string(argv[1]);
		if (cmd == "verify") {
			//verifyMod();
		} else if (cmd == "pack") {
			packSprite();
		} else if (cmd == "preview") {
			previewSprite();
		}
	} else {
		state::App app{sf::VideoMode(state::MIN_SCREEN_WIDTH, state::MIN_SCREEN_HEIGHT), "Please wait"};
		auto launch = std::make_unique<state::LaunchAppState>(app);
		app.push(launch);
		app.run();
	}
	*/
	
	assert_impl::fname = engine::get_preference_dir() + "crash.log";
	
	setlocale(LC_NUMERIC, "");
	
	state::App app{sf::VideoMode(state::MIN_SCREEN_WIDTH, state::MIN_SCREEN_HEIGHT), "Please wait"};
	ImGui::SFML::Init(app.getWindow());
	auto launch = std::make_unique<state::AppLauncherState>(app, argc > 1);
	app.push(launch);
	app.run();
	ImGui::SFML::Shutdown();
}
