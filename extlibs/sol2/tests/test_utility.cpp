// sol2 

// The MIT License (MIT)

// Copyright (c) 2013-2017 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#define SOL_CHECK_ARGUMENTS 1
#define SOL_ENABLE_INTEROP 1

#include <sol.hpp>

#include <catch.hpp>

#include <mutex>
#include <thread>

#ifdef SOL_CXX17_FEATURES
#include <string_view>
#include <variant>
#endif

std::mutex basic_init_require_mutex;

void basic_initialization_and_lib_open() {

	sol::state lua;
	try {
		lua.open_libraries();
		lua["a"] = 24;
		int a = lua["a"];
		{
			std::lock_guard<std::mutex> lg(basic_init_require_mutex);
			REQUIRE(a == 24);
		}
	}
	catch (const sol::error& e) {
		std::lock_guard<std::mutex> lg(basic_init_require_mutex);
		INFO(e.what());
		REQUIRE(false);
	}
	catch (...) {
		std::lock_guard<std::mutex> lg(basic_init_require_mutex);
		REQUIRE(false);
	}
}

TEST_CASE("utility/variant", "test that variant can be round-tripped") {
#ifdef SOL_CXX17_FEATURES
	SECTION("okay") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.set_function("f", [](int v) {
			return v == 2;
		});
		lua.set_function("g", [](std::variant<float, int, std::string> vv) {
			int v = std::get<int>(vv);
			return v == 2;
		});
		lua["v"] = std::variant<float, int, std::string>(2);
		REQUIRE_NOTHROW([&]() {
			lua.safe_script("assert(f(v))");
			lua.safe_script("assert(g(v))");
		}());
	}
	SECTION("throws") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.set_function("f", [](int v) {
			return v == 2;
		});
		lua.set_function("g", [](std::variant<float, int, std::string> vv) {
			int v = std::get<int>(vv);
			return v == 2;
		});
		lua["v"] = std::variant<float, int, std::string>(std::string("bark"));
		{
			auto result = lua.safe_script("assert(f(v))", sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};
		{
			auto result = lua.safe_script("assert(g(v))", sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};
	}
#else
	REQUIRE(true);
#endif // C++17
}

TEST_CASE("utility/string_view", "test that string_view can be taken as an argument") {
#ifdef SOL_CXX17_FEATURES
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set_function("f", [](std::string_view v) {
		return v == "bark!";
	});
	lua["v"] = "bark!";

	REQUIRE_NOTHROW([&]() {
		lua.safe_script("assert(f(v))");
	}());
#else
	REQUIRE(true);
#endif // C++17
}

TEST_CASE("utility/thread", "fire up lots of threads at the same time to make sure the initialization changes do not cause horrible crashing data races") {
	REQUIRE_NOTHROW([]() {
		std::thread thrds[16];
		for (int i = 0; i < 16; i++) {
			thrds[i] = std::thread(&basic_initialization_and_lib_open);
		}

		for (int i = 0; i < 16; i++) {
			thrds[i].join();
		}
	}());
}

TEST_CASE("utility/this_state", "Ensure this_state argument can be gotten anywhere in the function.") {
	struct bark {
		int with_state(sol::this_state l, int a, int b) {
			lua_State* L = l;
			int c = lua_gettop(L);
			return a + b + (c - c);
		}

		static int with_state_2(int a, sol::this_state l, int b) {
			INFO("inside with_state_2");
			lua_State* L = l;
			INFO("L is" << (void*)L);
			int c = lua_gettop(L);
			return a * b + (c - c);
		}
	};

	sol::state lua;
	INFO("created lua state");
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<bark>("bark",
		"with_state", &bark::with_state);

	INFO("setting b and with_state_2");
	bark b;
	lua.set("b", &b);
	lua.set("with_state_2", bark::with_state_2);
	INFO("finished setting");
	INFO("getting fx");
	sol::function fx = lua["with_state_2"];
	INFO("calling fx");
	int a = fx(25, 25);
	INFO("finished setting fx");
	INFO("calling a script");
	lua.safe_script("a = with_state_2(25, 25)");
	INFO("calling c script");
	lua.safe_script("c = b:with_state(25, 25)");
	INFO("getting a");
	int la = lua["a"];
	INFO("getting b");
	int lc = lua["c"];

	REQUIRE(lc == 50);
	REQUIRE(a == 625);
	REQUIRE(la == 625);
}
