#pragma once
#include <utils/logger.hpp>

namespace utils {

struct Verifier {
	bool result;
	utils::Logger& log;

	Verifier(utils::Logger& log);

	void operator()(bool condition, std::string const& error);
};

}  // ::utils
