#include <fstream>

#include <utils/assert.hpp>

namespace assert_impl {

std::string fname{};

void dump_crash(std::string const & msg) {
	if (!fname.empty()) {
		std::ofstream dump{fname, std::ios::app};
		dump << msg << std::endl;
		std::cout << "Crash log saved to " << fname << std::endl;
	}
}

} // ::assert_impl
