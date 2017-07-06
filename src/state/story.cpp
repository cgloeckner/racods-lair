#include <state/story.hpp>
#include <state/title.hpp>

namespace state {

StoryContext::StoryContext(Context& context)
	: context{context}
	, label{}
	, lines{}
	, index{0u}
	, alpha{0.f}
	, wait{sf::Time::Zero} {
	utils::split(context.locale("general.story"), "\\n", [&](std::string const & line) {
		lines.push_back(line);
	});
	alpha.repeat = 1;
	alpha.rise = true;
	alpha.speed = 0.0005f;
	
	next();
}

void StoryContext::next() {
	if (index < lines.size()) {
		setupTitle(label, "", context, lines[index]);
	}
	++index;
}

bool StoryContext::finished() const {
	return index > lines.size();
}

void StoryContext::update(sf::Time const & elapsed) {
	bool unused;
	utils::updateInterval(alpha, elapsed, unused);
	
	if (alpha.current == 1.f) {
		wait += elapsed;
		if (wait > sf::seconds(1.f)) {
			wait = sf::Time::Zero;
			alpha.repeat = 1;
			alpha.rise = false;
			alpha.speed = 0.001f;
		}
	} else if (alpha.current == 0.f) {
		wait += elapsed;
		if (wait > sf::seconds(0.25f)) {
			wait = sf::Time::Zero;
			next();
			alpha.repeat = 1;
			alpha.rise = true;
			alpha.speed = 0.0005f;
		}
	}
	
	thor::setAlpha(label, static_cast<sf::Uint8>(alpha.current * 255));
}

// --------------------------------------------------------------------

StoryState::StoryState(App& app)
	: State{app}
	, story{app.getContext()} {
	auto& context = app.getContext();
	// start title theme
	context.theme.setVolume(context.settings.music);
	context.theme.openFromFile(context.mod.get_filename<sf::Music>(context.globals.title_theme));
	context.theme.play();
}

void StoryState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	getContext().drawBackground(target, states);
	
	target.draw(story.label, states);
}

void StoryState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	story.label.setPosition(sf::Vector2f{screen_size} / 2.f);
}

void StoryState::onContinue() {
	auto& app = getApplication();
	auto launch = std::make_unique<TitleState>(app);
	app.push(launch);
}

void StoryState::onQuit() {
	quit();
}

void StoryState::handle(sf::Event const& event) {
	switch (event.type) {
		case sf::Event::Resized:
			onResize({event.size.width, event.size.height});
			break;
			
		case sf::Event::Closed:
			quit();
			break;
			
		case sf::Event::KeyPressed:
		case sf::Event::JoystickButtonPressed:
			onContinue();
			break;
			
		default:
			break;
	}
}

void StoryState::update(sf::Time const& elapsed) {
	story.update(elapsed);
	if (story.finished()) {
		onContinue();
	}
	
	getContext().update(elapsed);
}

} // ::state
