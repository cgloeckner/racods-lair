#include <utils/verifier.hpp>

namespace utils {

Verifier::Verifier(utils::Logger& log) : result{true}, log{log} {}

void Verifier::operator()(bool condition, std::string const& error) {
	if (!condition) {
		result = false;
		log << error << "\n";
	}
}

}  // ::utils
