#include "..\stdafx.h"
#include "..\CppUnitTestExpress.h"

#ifdef _WIN32
#define SLEEP(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#define SLEEP(seconds) sleep(seconds)
#endif

#define VALUES \
V(3.14f, 3.14, 3)

class TestAuto_ctor : public Unit<TestAuto_ctor> {
public:
	TestAuto_ctor() : Unit<TestAuto_ctor>(SETTING, "throw *this") {
		throw *this;
	}

	void Test() {}
};

class TestAuto_dtor : public Unit<TestAuto_dtor> {
public:
	~TestAuto_dtor() {
		//Throwing *this on destruction is an error;
		throw UnitTest(TEARING, "throw UnitTest(worse, whats)");
	}

	void Test() {}
};

class TestAuto_Test : public Unit<TestAuto_Test> {
public:
	TestAuto_Test() : Unit<TestAuto_Test>(TESTING, "throw *this") {}

	void Test() {
		throw *this;
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

class TestAuto_wcMatch : public Unit<TestAuto_wcMatch> {
	void Test() {
		#define V(f, d, i) \
			_assert(wcMatch(#f, #d"*"), "%s matchs \"%s\"", #f, #d"*"); \
			_assert(wcMatch(#f, #d"?"), "%s matchs \"%s\"", #f, #d"?"); \
			_assert(wcMatch(#f, #i"*"), "%s matchs \"%s\"", #f, #i"*"); \
			_assert(wcMatch(#f, #i"????"), "%s matchs \"%s\"", #f, #i"????"); \
			_assert(!wcMatch(#f, "^"#f), "%s doesn't match \"%s\"", #f, "^"#f); \
			_assert(!wcMatch(#f, "^"#d"*"), "%s doesn't match \"%s\"", #f, "^"#d"*");
			VALUES
		#undef V
	}
};

class TestAuto_dprintf_assert : public Unit<TestAuto_dprintf_assert> {
	void Test() {
		#define V(f, d, i) \
			char a[6] = {#f}; \
			dprintf("enum: %d, float: %f, double: %lf, integer: %d, char*: %s, char[]: %s\n", SUCCESS, f, d, i, #f, a); \
			_assert(f != d, "assert float %f and double %lf", f, d); \
			_assert(f != i, "assert float %f and integer %d", f, i); \
			_assert(strcmp(a, #f) == 0, "assert char[] %s and char* %s", a, #f); \
			_assert(strcmp(#f, #d), "assert compare char* between %s and %s", #f, #d); \
			_assert(wcscmp(L#f, L#d), "assert compare char* between %s and %s", #f, #d);
			VALUES
		#undef V
	}
};

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L) //>=C++11
class TestAuto_dprintf_assert_c11 : public Unit<TestAuto_dprintf_assert_c11> {
	void Test() {
		#define V(f, d, i) \
			char a[6] = {#f}; \
			dprintf(std::string("enum: %d, float: %f, double: %lf, integer: %d, char*: %s, char[]: %s, string: %s\n"), SUCCESS, f, d, i, #f, a, std::string(#f)); \
			dprintf("enum: %d, float: %f, double: %lf, integer: %d, char*: %s, char[]: %s, string: %s\n", SUCCESS, f, d, i, #f, a, std::string(#f)); \
			_assert(f != d, std::string("assert float %f and double %lf"), f, d); \
			_assert(f != i, std::string("assert float %f and integer %d"), f, i); \
			_assert(strcmp(a, #f) == 0, std::string("assert char[] %s and char* %s"), a, #f); \
			_assert(strcmp(#f, #d), std::string("assert compare char* between %s and %s"), std::string(#f), #d); \
			_assert(wcscmp(L#f, L#d), "assert compare char* between %s and %s", std::string(#f), #d);
			VALUES
		#undef V
	}
};
#endif

class UnitTestEx : public UnitTest {
public:
	virtual std::string report(std::string stage, STATE state, std::string what) {
		return ssprintf("%s : %s - %s\n", stateName(state), stage.c_str(), what.c_str());
	}

	virtual void resume(int count, int total, long microseconds, STATE state, std::string reports, std::string wildcard) {
		std::string sWhich = wildcard.empty() ? "" : "Matching: " + wildcard + "\n";

		dprintf("\n");
		dprintf(reports.c_str());
		dprintf("----------------------------------------\n"
			"Executed: %d/%d %s, %lgs at %s\n"
			"Resulted: %s\n"
			"%s\n",
			count,
			total,
			count > 1 ? "units" : "unit",
			microseconds / 1e6,
			localDate().c_str(),
			stateName(state),
			sWhich.c_str());
	}
};

class TestAuto_extended : public Unit<TestAuto_extended> {
public:
	TestAuto_extended() : Unit<TestAuto_extended>(UNKNOWN, "throw *this") {}

	void Test() {
		static bool extended = false;
		if (extended == false) {
			extended = true;

			UnitTestEx ex;
			UnitTest* _ut = &ex;
			_assert(_ut->runAll(name()) == UNKNOWN, "throw UNKNOWN");
		}
		else {
			throw *this;
		}
	}
};
