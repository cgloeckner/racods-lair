#include <utils/algorithm.hpp>

namespace utils {

template <typename T>
std::vector<typename DelaySystem<T>::Node> const& DelaySystem<T>::data() const {
	return wait;
}

template <typename T>
void DelaySystem<T>::reset() {
	wait.clear();
	ready.clear();
}

template <typename T>
void DelaySystem<T>::push(T const& value, sf::Time const& delay) {
	wait.emplace_back();
	auto& elem = wait.back();
	elem.value = value;
	elem.delay = delay;
}


template <typename T>
void DelaySystem<T>::operator()(sf::Time const& elapsed) {
	operator()(elapsed, [](T const &) {});
}

template <typename T>
void DelaySystem<T>::operator()(sf::Time const& elapsed, std::function<void(T const &)> handle) {
	utils::remove_if(wait, [&](Node& node) {
		handle(node.value);
		node.delay -= elapsed;
		if (node.delay <= sf::Time::Zero) {
			ready.push_back(std::move(node.value));
			// note: return true will remove from it the container
			return true;
		}
		return false;
	});
}

}  // ::utils
