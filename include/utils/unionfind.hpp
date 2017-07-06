#pragma once
#include <vector>
#include <memory>

namespace utils {

class Unionfind {
  public:
	struct Set {
		int rank;
		Set* parent;
	};

  protected:
	std::vector<std::unique_ptr<Set>> buffer;

  public:
	Set& make();
	Set& find(Set& child);
	void join(Set& first, Set& second);
};

}  // ::utils
