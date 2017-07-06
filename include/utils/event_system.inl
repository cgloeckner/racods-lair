#include <utils/algorithm.hpp>

namespace utils {

template <typename T>
void SingleEventSender<T>::send(T const& event) {
	queue.push_back(event);
}

template <typename T>
void SingleEventSender<T>::propagate() {
	for (auto ptr : listeners) {
		auto& target = *ptr;
		for (auto const& event : queue) {
			target.receive(event);
		}
	}
	clear();
}

template <typename T>
void SingleEventSender<T>::clear() {
	queue.clear();
}

template <typename T>
std::vector<T> const& SingleEventSender<T>::data() const {
	return queue;
}

template <typename T>
void SingleEventSender<T>::bind(SingleEventListener<T>& listener) {
	listeners.push_back(&listener);
}

template <typename T>
void SingleEventSender<T>::unbind(SingleEventListener<T> const& listener) {
	// workaround: cannot pop pointer directly because of const (-.-)
	utils::pop_if(listeners, [&listener](SingleEventListener<T>* other) {
		return other == &listener;
	});
}

// ---------------------------------------------------------------------------

template <typename T>
void SingleEventListener<T>::receive(T const& event) {
	queue.push_back(event);
}

template <typename T>
template <typename H>
void SingleEventListener<T>::dispatch(H& handler) {
	for (auto const& event : queue) {
		handler.handle(event);
	}
	clear();
}

template <typename T>
std::vector<T> const& SingleEventListener<T>::data() const {
	return queue;
}

template <typename T>
void SingleEventListener<T>::clear() {
	queue.clear();
}

// ---------------------------------------------------------------------------

template <typename Head, typename... Tail>
void MultiEventSenderImpl<Head, Tail...>::propagateAll() {
	SingleEventSender<Head>::propagate();
	MultiEventSenderImpl<Tail...>::propagateAll();
}

template <typename Head, typename... Tail>
template <typename H>
void MultiEventListenerImpl<Head, Tail...>::dispatchAll(H& handler) {
	SingleEventListener<Head>::dispatch(handler);
	MultiEventListenerImpl<Tail...>::dispatchAll(handler);
}

template <typename H>
void MultiEventListenerImpl<>::dispatchAll(H& handler) {
}

// ---------------------------------------------------------------------------

template <typename... Events>
template <typename T>
void EventSender<Events...>::send(T event) {
	static_assert(
		pack_contains<T, Events...>::value, "Event must be supported");
	SingleEventSender<T>::send(event);
}

template <typename... Events>
template <typename T>
void EventSender<Events...>::propagate() {
	static_assert(
		pack_contains<T, Events...>::value, "Event must be supported");
	SingleEventSender<T>::propagate();
}

template <typename... Events>
template <typename T>
void EventSender<Events...>::bind(utils::SingleEventListener<T>& receiver) {
	static_assert(
		pack_contains<T, Events...>::value, "Event must be supported");
	SingleEventSender<T>::bind(receiver);
}

template <typename... Events>
template <typename T>
void EventSender<Events...>::unbind(
	utils::SingleEventListener<T> const& receiver) {
	static_assert(
		pack_contains<T, Events...>::value, "Event must be supported");
	SingleEventSender<T>::unbind(receiver);
}

template <typename... Events>
void EventSender<Events...>::propagateAll() {
	MultiEventSenderImpl<Events...>::propagateAll();
}

// ---------------------------------------------------------------------------

template <typename... Events>
template <typename T>
void EventListener<Events...>::receive(T event) {
	static_assert(
		pack_contains<T, Events...>::value, "Event must be supported");
	SingleEventListener<T>::receive(event);
}

template <typename... Events>
template <typename T, typename Handler>
void EventListener<Events...>::dispatch(Handler& handler) {
	static_assert(
		pack_contains<T, Events...>::value, "Event must be supported");
	SingleEventListener<T>::dispatch(handler);
}

template <typename... Events>
template <typename Handler>
void EventListener<Events...>::dispatchAll(Handler& handler) {
	MultiEventListenerImpl<Events...>::dispatchAll(handler);
}

}  // ::utils
