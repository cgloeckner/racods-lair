#include <cxxabi.h>

namespace tool {

template <typename T>
bool createInspector(InspectorMap& map, core::ComponentManager<T>& system, core::LogContext& log, engine::Engine& engine, core::ObjectID id) {
	if (!system.has(id)) {
		return false;
	}
	int status;
	std::string key{abi::__cxa_demangle(typeid(T).name(), 0, 0, &status)};
	ASSERT(status == 0);
	auto uptr = std::make_unique<ComponentInspector<T>>(log, engine, id);
	map[key] = std::move(uptr);
	return true;
}

} // ::tool
