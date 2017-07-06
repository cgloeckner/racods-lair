#include <cxxabi.h>

namespace engine {

template <typename T>
void EventLogger::handle(T const & event) {
	auto& node = at(std::type_index(typeid(T)));
	if (!node.enabled) {
		return;
	}
	
	int status;
	std::string key{abi::__cxa_demangle(typeid(T).name(), 0, 0, &status)};
	ASSERT(status == 0);
	
	node.stream << utils::now << key << "(" << event << ")\n";
}

template <typename T>
void EventLogger::clear() {
	auto& node = at(std::type_index(typeid(T)));
	node = Node{};
}

template <typename T>
void EventLogger::setEnabled(bool flag) {
	auto& node = at(std::type_index(typeid(T)));
	node.enabled = flag;
}

} // ::engine
