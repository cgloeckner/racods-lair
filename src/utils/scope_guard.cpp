#include <utils/scope_guard.hpp>

namespace utils {

ScopeGuard::ScopeGuard(
	std::function<void(void)> enter, std::function<void(void)> exit)
	: exit{exit} {
	enter();
}

ScopeGuard::~ScopeGuard() { exit(); }

}  // ::utils
