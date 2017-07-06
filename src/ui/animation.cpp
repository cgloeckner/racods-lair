#include <ui/animation.hpp>

namespace ui {

WidgetAnimation::WidgetAnimation()
	: scale{1.f} {
	scale.repeat = 0;
}

void WidgetAnimation::startAnimation() {
	scale.current = 1.f;
	scale.min = 0.9f;
	scale.max = 1.2f;
	scale.speed = 0.00075f;
	scale.rise = true;
	scale.repeat = -1;
}

void WidgetAnimation::stopAnimation() {
	scale.repeat = 1;
	if (scale.current > 1.f) {
		scale.rise = false;
		scale.min = 1.f;
	} else {
		scale.rise = true;
		scale.max = 1.f;
	}
}

void WidgetAnimation::operator()(sf::Text& label, sf::Time const & elapsed) {
	{
		bool changed{false};
		utils::updateInterval(scale, elapsed, changed);
		if (changed) {
			label.setScale({scale.current, scale.current});
		}
	}
}

} // :: ui
