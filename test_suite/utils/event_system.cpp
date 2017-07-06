#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/event_system.hpp>

struct TestEvent {
	std::size_t id;

	TestEvent(std::size_t id) : id{id} {}
};

using TestSender = utils::EventSender<TestEvent>;
using TestListener = utils::EventListener<TestEvent>;

struct TestHandler {
	std::vector<TestEvent> handled;

	void handle(TestEvent const& event) { handled.push_back(event); }
};

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(event_unit_test)

BOOST_AUTO_TEST_CASE(dispatch_without_bound_listener) {
	TestSender sender;
	TestListener listener;
	TestHandler handler;

	//sender.propagate<TestEvent>();
	sender.propagateAll();
	//listener.dispatch<TestEvent>(handler);
	listener.dispatchAll(handler);
	BOOST_CHECK(handler.handled.empty());
}

BOOST_AUTO_TEST_CASE(push_single_event_to_bound_listener) {
	TestSender sender;
	TestListener listener;
	TestHandler handler;
	sender.bind<TestEvent>(listener);

	sender.send(TestEvent{3u});

	//sender.propagate<TestEvent>();
	sender.propagateAll();
	//listener.dispatch<TestEvent>(handler);
	listener.dispatchAll(handler);
	BOOST_REQUIRE_EQUAL(1u, handler.handled.size());
	BOOST_CHECK_EQUAL(3u, handler.handled.at(0).id);
}

BOOST_AUTO_TEST_CASE(push_multiple_events_to_bound_listener) {
	TestSender sender;
	TestListener listener;
	TestHandler handler;
	sender.bind<TestEvent>(listener);

	sender.send(TestEvent{3u});
	sender.send(TestEvent{2u});
	sender.send(TestEvent{7u});

	//sender.propagate<TestEvent>();
	sender.propagateAll();
	//listener.dispatch<TestEvent>(handler);
	listener.dispatchAll(handler);
	BOOST_REQUIRE_EQUAL(3u, handler.handled.size());
	BOOST_CHECK_EQUAL(3u, handler.handled.at(0).id);
	BOOST_CHECK_EQUAL(2u, handler.handled.at(1).id);
	BOOST_CHECK_EQUAL(7u, handler.handled.at(2).id);
}

BOOST_AUTO_TEST_CASE(push_multiple_events_to_suddenly_unbound_listener) {
	TestSender sender;
	TestListener listener;
	TestHandler handler;
	sender.bind<TestEvent>(listener);

	sender.send(TestEvent{3u});
	sender.send(TestEvent{2u});
	sender.send(TestEvent{7u});
	sender.unbind<TestEvent>(listener);

	//sender.propagate<TestEvent>();
	sender.propagateAll();
	//listener.dispatch<TestEvent>(handler);
	listener.dispatchAll(handler);
	BOOST_CHECK(handler.handled.empty());
}

BOOST_AUTO_TEST_CASE(push_multiple_events_to_multiple_listener) {
	TestSender sender;
	TestListener l1, l2;
	TestHandler h1, h2;
	sender.bind<TestEvent>(l1);
	sender.bind<TestEvent>(l2);

	sender.send(TestEvent{3u});
	sender.send(TestEvent{2u});
	sender.send(TestEvent{7u});

	//sender.propagate<TestEvent>();
	sender.propagateAll();
	//l1.dispatch<TestEvent>(h1);
	l1.dispatchAll(h1);
	BOOST_REQUIRE_EQUAL(3u, h1.handled.size());
	BOOST_CHECK_EQUAL(3u, h1.handled.at(0).id);
	BOOST_CHECK_EQUAL(2u, h1.handled.at(1).id);
	BOOST_CHECK_EQUAL(7u, h1.handled.at(2).id);
	//l2.dispatch<TestEvent>(h2);
	l2.dispatchAll(h2);
	BOOST_REQUIRE_EQUAL(3u, h2.handled.size());
	BOOST_CHECK_EQUAL(3u, h2.handled.at(0).id);
	BOOST_CHECK_EQUAL(2u, h2.handled.at(1).id);
	BOOST_CHECK_EQUAL(7u, h2.handled.at(2).id);
}

BOOST_AUTO_TEST_CASE(push_multiple_events_from_multiple_senders) {
	TestSender s1, s2;
	TestListener listener;
	TestHandler handler;
	s1.bind<TestEvent>(listener);
	s2.bind<TestEvent>(listener);

	s1.send(TestEvent{3u});
	s1.send(TestEvent{2u});
	s2.send(TestEvent{1u});
	s1.send(TestEvent{7u});
	s2.send(TestEvent{5u});
	s2.send(TestEvent{0u});
	//s2.propagate<TestEvent>();
	//s1.propagate<TestEvent>();
	s2.propagateAll();
	s1.propagateAll();

	//listener.dispatch<TestEvent>(handler);
	listener.dispatchAll(handler);
	BOOST_REQUIRE_EQUAL(6u, handler.handled.size());
	BOOST_CHECK_EQUAL(1u, handler.handled.at(0).id);
	BOOST_CHECK_EQUAL(0u, handler.handled.at(2).id);
	BOOST_CHECK_EQUAL(3u, handler.handled.at(3).id);
	BOOST_CHECK_EQUAL(7u, handler.handled.at(5).id);
}

BOOST_AUTO_TEST_SUITE_END()

// ----------------------------------------------------------------------------

struct FooEvent {
	std::string msg;

	FooEvent(std::string const& msg) : msg{msg} {}
};

struct BarEvent {
	float value;
	int id;

	BarEvent(float value, int id) : value{value}, id{id} {}
};

using FooSender = utils::EventSender<FooEvent>;
using MultiSender = utils::EventSender<FooEvent, BarEvent>;

struct FooListener : utils::EventListener<FooEvent> {
	std::vector<FooEvent> foo;

	void handle(FooEvent const& event) { foo.push_back(event); }
};

struct BarListener : utils::EventListener<BarEvent> {
	std::vector<BarEvent> bar;

	void handle(BarEvent const& event) { bar.push_back(event); }
};

struct MultiListener : utils::EventListener<FooEvent, BarEvent> {
	std::vector<FooEvent> foo;
	std::vector<BarEvent> bar;

	void handle(FooEvent const& event) { foo.push_back(event); }

	void handle(BarEvent const& event) { bar.push_back(event); }
};

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(event_integration_test)

BOOST_AUTO_TEST_CASE(single_sender_to_single_listener) {
	FooSender sender;
	FooListener listener;
	sender.bind<FooEvent>(listener);

	sender.send(FooEvent{"hello world"});
	//sender.propagate<FooEvent>();
	sender.propagateAll();
	//listener.dispatch<FooEvent>(listener);
	listener.dispatchAll(listener);
	BOOST_REQUIRE_EQUAL(1u, listener.foo.size());
	BOOST_CHECK_EQUAL("hello world", listener.foo.at(0).msg);
}

BOOST_AUTO_TEST_CASE(single_sender_to_multi_listener) {
	FooSender sender;
	MultiListener listener;
	sender.bind<FooEvent>(listener);

	sender.send(FooEvent{"hello world"});
	//sender.propagate<FooEvent>();
	sender.propagateAll();
	//listener.dispatch<FooEvent>(listener);
	listener.dispatchAll(listener);
	BOOST_REQUIRE_EQUAL(1u, listener.foo.size());
	BOOST_CHECK_EQUAL("hello world", listener.foo.at(0).msg);
}

BOOST_AUTO_TEST_CASE(multi_sender_to_disjoint_listeners) {
	MultiSender sender;
	FooListener foo;
	BarListener bar;
	sender.bind<FooEvent>(foo);
	sender.bind<BarEvent>(bar);

	sender.send(FooEvent{"hello world"});
	sender.send(BarEvent{3.14f, 12});
	//sender.propagate<FooEvent>();
	//sender.propagate<BarEvent>();
	sender.propagateAll();
	//foo.dispatch<FooEvent>(foo);
	//bar.dispatch<BarEvent>(bar);
	foo.dispatchAll(foo);
	bar.dispatchAll(bar);
	BOOST_REQUIRE_EQUAL(1u, foo.foo.size());
	BOOST_CHECK_EQUAL("hello world", foo.foo.at(0).msg);
	BOOST_REQUIRE_EQUAL(1u, bar.bar.size());
	BOOST_CHECK_CLOSE(3.14f, bar.bar.at(0).value, 0.0001f);
	BOOST_CHECK_EQUAL(12, bar.bar.at(0).id);
}

BOOST_AUTO_TEST_CASE(multi_sender_to_multi_listener_incompletly_bound) {
	MultiSender sender;
	MultiListener listener;
	sender.bind<FooEvent>(listener);

	sender.send(FooEvent{"hello world"});
	sender.send(BarEvent{3.14f, 12});
	//sender.propagate<FooEvent>();
	//sender.propagate<BarEvent>();
	sender.propagateAll();
	//listener.dispatch<FooEvent>(listener);
	//listener.dispatch<BarEvent>(listener);
	listener.dispatchAll(listener);
	BOOST_REQUIRE_EQUAL(1u, listener.foo.size());
	BOOST_CHECK_EQUAL("hello world", listener.foo.at(0).msg);
	BOOST_REQUIRE(listener.bar.empty());
}

BOOST_AUTO_TEST_CASE(multi_sender_to_multi_listener_completly_bound) {
	MultiSender sender;
	MultiListener listener;
	sender.bind<FooEvent>(listener);
	sender.bind<BarEvent>(listener);

	sender.send(FooEvent{"hello world"});
	sender.send(BarEvent{3.14f, 12});
	//sender.propagate<FooEvent>();
	//sender.propagate<BarEvent>();
	sender.propagateAll();
	//listener.dispatch<FooEvent>(listener);
	//listener.dispatch<BarEvent>(listener);
	listener.dispatchAll(listener);
	BOOST_REQUIRE_EQUAL(1u, listener.foo.size());
	BOOST_CHECK_EQUAL("hello world", listener.foo.at(0).msg);
	BOOST_REQUIRE_EQUAL(1u, listener.bar.size());
	BOOST_CHECK_CLOSE(3.14f, listener.bar.at(0).value, 0.0001f);
	BOOST_CHECK_EQUAL(12, listener.bar.at(0).id);
}

BOOST_AUTO_TEST_CASE(multi_sender_to_overlapping_listeners) {
	MultiSender sender;
	MultiListener multi;
	FooListener foo;
	sender.bind<FooEvent>(foo);
	sender.bind<FooEvent>(multi);
	sender.bind<BarEvent>(multi);

	sender.send(FooEvent{"hello world"});
	sender.send(BarEvent{3.14f, 12});
	//sender.propagate<FooEvent>();
	//sender.propagate<BarEvent>();
	sender.propagateAll();
	//foo.dispatch<FooEvent>(foo);
	foo.dispatchAll(foo);
	//multi.dispatch<FooEvent>(multi);
	//multi.dispatch<BarEvent>(multi);
	multi.dispatchAll(multi);
	BOOST_REQUIRE_EQUAL(1u, foo.foo.size());
	BOOST_CHECK_EQUAL("hello world", foo.foo.at(0).msg);
	BOOST_REQUIRE_EQUAL(1u, multi.foo.size());
	BOOST_CHECK_EQUAL("hello world", multi.foo.at(0).msg);
	BOOST_REQUIRE_EQUAL(1u, multi.bar.size());
	BOOST_CHECK_CLOSE(3.14f, multi.bar.at(0).value, 0.0001f);
	BOOST_CHECK_EQUAL(12, multi.bar.at(0).id);
}

BOOST_AUTO_TEST_CASE(standalone_simple_listener_receive) {
	FooListener foo;
	foo.receive(FooEvent{"hello world"});
	foo.receive(FooEvent{"bar baz bol"});
	//foo.dispatch<FooEvent>(foo);
	foo.dispatchAll(foo);

	BOOST_REQUIRE_EQUAL(2u, foo.foo.size());
	BOOST_CHECK_EQUAL("hello world", foo.foo.at(0).msg);
	BOOST_CHECK_EQUAL("bar baz bol", foo.foo.at(1).msg);
}

BOOST_AUTO_TEST_CASE(standalone_multi_listener_receive) {
	MultiListener listener;
	listener.receive(FooEvent{"hello world"});
	listener.receive(FooEvent{"bar baz bol"});
	listener.receive(BarEvent{3.14f, 12});
	//listener.dispatch<FooEvent>(listener);
	//listener.dispatch<BarEvent>(listener);
	listener.dispatchAll(listener);

	BOOST_REQUIRE_EQUAL(2u, listener.foo.size());
	BOOST_CHECK_EQUAL("hello world", listener.foo.at(0).msg);
	BOOST_CHECK_EQUAL("bar baz bol", listener.foo.at(1).msg);
	BOOST_REQUIRE_EQUAL(1u, listener.bar.size());
	BOOST_CHECK_CLOSE(3.14, listener.bar.at(0).value, 0.0001f);
	BOOST_CHECK_EQUAL(12, listener.bar.at(0).id);
}

BOOST_AUTO_TEST_SUITE_END()
