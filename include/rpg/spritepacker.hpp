#pragma once
#include <string>
#include <utils/logger.hpp>

namespace rpg {

class SpritePacker {
  private:
	std::string const source, target;
	utils::Logger& log;
	
  public:
	SpritePacker(std::string const & source, std::string const & target, utils::Logger& logger);
	
	bool operator()(std::string const & fname);
};

} // ::rpg
