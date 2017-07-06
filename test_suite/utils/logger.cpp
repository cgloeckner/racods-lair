#include <sstream>
#include <boost/test/unit_test.hpp>

#include <utils/logger.hpp>

BOOST_AUTO_TEST_SUITE(logger_test)

BOOST_AUTO_TEST_CASE(ostream_dumps_Color) {
	std::stringstream s;
	s << sf::Color::Red;
}

BOOST_AUTO_TEST_CASE(ostream_dumps_Key) {
	std::stringstream s;
	s << sf::Keyboard::Space;
}

BOOST_AUTO_TEST_CASE(ostream_dumps_MouseButton) {
	std::stringstream s;
	s << sf::Mouse::Button::Left;
}

BOOST_AUTO_TEST_CASE(ostream_dumps_JoystickAxis) {
	std::stringstream s;
	s << sf::Joystick::Axis::X;
}

BOOST_AUTO_TEST_CASE(ostream_dumps_Vector2) {
	std::stringstream s;
	s << sf::Vector2f{};
}

BOOST_AUTO_TEST_CASE(ostream_dumps_Vector3) {
	std::stringstream s;
	s << sf::Vector3f{};
}

BOOST_AUTO_TEST_CASE(ostream_dumps_Rect) {
	std::stringstream s;
	s << sf::FloatRect{};
}

BOOST_AUTO_TEST_CASE(ostream_dumps_Time) {
	std::stringstream s;
	s << sf::Time::Zero;
}

BOOST_AUTO_TEST_CASE(ostream_dumps_VideMode) {
	std::stringstream s;
	s << sf::VideoMode{};
}

BOOST_AUTO_TEST_CASE(ostream_dumps_current_time) {
	std::stringstream s;
	s << utils::now;
}

BOOST_AUTO_TEST_SUITE_END()
