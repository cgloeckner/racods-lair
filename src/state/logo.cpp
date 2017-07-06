#include <ui/common.hpp>

#include <state/logo.hpp>
#include <state/story.hpp>

namespace state {

LogoContext::LogoContext(Context& context)
	: sprite{}
	, gfx{}
	, index{0u}
	, alpha{0.f}
	, wait{sf::Time::Zero} {
	gfx.reserve(context.globals.logo.size());
	for (auto const & fname: context.globals.logo) {
		gfx.push_back(&context.mod.get<sf::Texture>(fname));
	}
	
	alpha.repeat = 1;
	alpha.rise = true;
	alpha.speed = 0.0005f;
	
	next();
}

void LogoContext::next() {
	if (index < gfx.size()) {
		sprite.setTexture(*gfx[index]);
		auto size = gfx[index]->getSize();
		sprite.setTextureRect({0, 0, static_cast<int>(size.x), static_cast<int>(size.y)});
		ui::centerify(sprite);
	}
	++index;
}

bool LogoContext::finished() const {
	return index > gfx.size();
}

void LogoContext::update(sf::Time const & elapsed) {
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
	
	thor::setAlpha(sprite, static_cast<sf::Uint8>(alpha.current * 255));
}

// --------------------------------------------------------------------

LogoState::LogoState(App& app)
	: State{app}
	, logo{app.getContext()} {
}

void LogoState::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(logo.sprite, states);
}

void LogoState::onResize(sf::Vector2u screen_size) {
	State::onResize(screen_size);
	
	logo.sprite.setPosition(sf::Vector2f{screen_size} / 2.f);
}

void LogoState::onContinue() {
	auto& app = getApplication();
	auto launch = std::make_unique<StoryState>(app);
	app.push(launch);
}

void LogoState::onQuit() {
	quit();
}

void LogoState::handle(sf::Event const& event) {
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

void LogoState::update(sf::Time const& elapsed) {
	logo.update(elapsed);
	if (logo.finished()) {
		onContinue();
	}
	
	getContext().update(elapsed);
}

} // ::state
