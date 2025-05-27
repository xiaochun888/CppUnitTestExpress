#include "..\stdafx.h"
#include "..\CppUnitTestExpress.h"

#ifdef _WIN32
#include <windows.h>
#define SLEEP(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#define SLEEP(seconds) sleep(seconds)
#endif

#define VALUES \
V(3.14f, 3.14, 3)

class TestAuto_ctor : public Unit<TestAuto_ctor> {
public:
	TestAuto_ctor() {
		throw this;
	}

	void Test() {}
};

class TestAuto_Test : public Unit<TestAuto_Test> {
	void Test() {
		throw this;
	}
};

class TestAuto_enum_STATE : public Unit<TestAuto_enum_STATE> {
	void Test() {
		_assert(SUCCESS == 0, "STATE.SUCCESS shouble be 0.");
	}
};

class TestAuto_usElapse : public Unit<TestAuto_usElapse> {
	void Test() {
		long us = usElapse(0);
		SLEEP(1);
		long seconds = usElapse(us) / 1e6;
		_assert(seconds == 1, "Time elapsed shouble be 1s but %lds.", seconds);
		SLEEP(1);
		seconds = usElapse(us) / 1e6;
		_assert(seconds == 2, "Time elapsed shouble be 2s but %lds.", seconds);
	}
};

class TestAuto_dprintf_assert : public Unit<TestAuto_dprintf_assert> {
	void Test() {
		#define V(f, d, i) \
			char a[6] = {#f}; \
			dprintf("\tenum: %d, float: %f, double: %lf, integer: %d, char*: %s, char[]: %s\n", SUCCESS, f, d, i, #f, a); \
			_assert(f != d, "assert float %f and double %lf", f, d); \
			_assert(f != i, "assert float %f and integer %d", f, i); \
			_assert(strcmp(a, #f) == 0, "assert char[] %s and char* %s", a, #f); \
			_assert(strcmp(#f, #d), "assert compare char* between %s and %s", std::string(#f), #d);
			VALUES
		#undef V
	}
};

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L) //>=C++11
class TestAuto_dprintf_assert_c11 : public Unit<TestAuto_dprintf_assert_c11> {
	void Test() {
		#define V(f, d, i) \
			char a[6] = {#f}; \
			dprintf(std::string("\tenum: %d, float: %f, double: %lf, integer: %d, char*: %s, char[]: %s, string: %s\n"), SUCCESS, f, d, i, #f, a, std::string(#f)); \
			dprintf("\tenum: %d, float: %f, double: %lf, integer: %d, char*: %s, char[]: %s, string: %s\n", SUCCESS, f, d, i, #f, a, std::string(#f)); \
			_assert(f != d, std::string("assert float %f and double %lf"), f, d); \
			_assert(f != i, std::string("assert float %f and integer %d"), f, i); \
			_assert(strcmp(a, #f) == 0, std::string("assert char[] %s and char* %s"), a, #f); \
			_assert(strcmp(#f, #d), std::string("assert compare char* between %s and %s"), std::string(#f), #d);
			VALUES
		#undef V
	}
};
#endif
