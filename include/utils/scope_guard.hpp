#pragma once
#include <functional>

namespace utils {

struct ScopeGuard final {
	std::function<void(void)> exit;

	ScopeGuard(std::function<void(void)> enter, std::function<void(void)> exit);
	~ScopeGuard();
};

}  // ::utils
