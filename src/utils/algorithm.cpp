#include <utils/algorithm.hpp>

namespace utils {

unsigned int distance(unsigned int u, unsigned int v) {
	if (u >= v) {
		return u - v;
	} else {
		return v - u;
	}
}

}  // ::utils
