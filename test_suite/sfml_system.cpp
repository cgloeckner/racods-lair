#include <testsuite/sfml_system.hpp>

namespace unittest {

std::string matrix2string(sf::Transform const & matrix) {
	std::string s = "[";
	for (auto i = 0u; i < 16u; ++i) {
		s += std::to_string(matrix.getMatrix()[i]);
		if (i % 4 == 3) {
			if (i < 15u) {
				s += ";";
			}
		} else {
			s += ",";
		}
	}
	return s + "]";
}

} // ::unittest
