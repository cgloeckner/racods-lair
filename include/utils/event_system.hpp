#pragma once
#include <vector>

namespace utils {

template <typename T>
class SingleEventListener;

template <typename T>
class SingleEventSender;

template <typename... Tail>
class MultiEventSenderImpl;

template <typename... Tail>
class MultiEventListenerImpl;

// inherit from multiple event senders
template <typename Head, typename... Tail>
class MultiEventSenderImpl<Head, Tail...>
	: public SingleEventSender<Head>, public MultiEventSenderImpl<Tail...> {
  public:
	void propagateAll();
};

// inherit from multiple event listeners
template <typename Head, typename... Tail>
class MultiEventListenerImpl<Head, Tail...>
	: public SingleEventListener<Head>,
	  public MultiEventListenerImpl<Tail...> {
  public:
	template <typename H>
	void dispatchAll(H& handler);
};

template <>
class MultiEventSenderImpl<> {
  public:
	void propagateAll();
};

template <>
class MultiEventListenerImpl<> {
  public:
	template <typename H>
	void dispatchAll(H& handler);
};

// ---------------------------------------------------------------------------

template <typename T>
class SingleEventSender {
  private:
	std::vector<SingleEventListener<T>*> listeners;

  protected:
	std::vector<T> queue;

  public:
	void send(T const& event);
	void propagate();
	void clear();

	std::vector<T> const& data() const;

	void bind(SingleEventListener<T>& listener);
	void unbind(SingleEventListener<T> const& listener);
};

template <typename T>
class SingleEventListener {
	friend class SingleEventSender<T>;

  protected:
	std::vector<T> queue;

  public:
	void receive(T const& event);
	void clear();

	std::vector<T> const& data() const;

	template <typename H>
	void dispatch(H& handler);
};

// ---------------------------------------------------------------------------

template <typename... Events>
class EventSender : public MultiEventSenderImpl<Events...> {

  public:
	template <typename T>
	void send(T event);

	template <typename T>
	void propagate();
	
	void propagateAll();

	template <typename T>
	void bind(SingleEventListener<T>& receiver);

	template <typename T>
	void unbind(SingleEventListener<T> const& receiver);
};

// ---------------------------------------------------------------------------

template <typename... Events>
class EventListener : public MultiEventListenerImpl<Events...> {

  public:
	template <typename T>
	void receive(T event);

	template <typename T, typename Handler>
	void dispatch(Handler& handler);
	
	template <typename Handler>
	void dispatchAll(Handler& handler);
};

}  // ::utils

// include implementation details
#include <utils/event_system.inl>
