/****************************************************************************
*				 CppUnitTestExpress.h
* Description:
*
*		CppUnitTestExpress - C++ Unit Test Express
*
* Designed:
	* Only a header file, only use C++ compiler
	* Any member method or function, any C++ platform and down-level C++ compilers (e.g. VC6.0)
	* No config, no test macro, no graphic interface, no extrernal library
	* Auto record, grouping by keyword, extended easily
* How to do:
	Please see an example in the end of this file.
* Created: 30/05/2008 03:00 AM
* Author: XCZ
* Email: xczhong@free.fr
*
*****************************************************************************/

#pragma once 
#ifndef _CPP_UNIT_TEST_EXPRESS_H_
#define _CPP_UNIT_TEST_EXPRESS_H_

#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <ctime>
#include <typeinfo>
#include <stdexcept>
#include <string>
#include <map>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

/**
Ensures symbol remains in object file even if unused
Prevents compiler optimization from removing the instantiation
*/
#if defined(__GNUC__) || defined(__clang__)
#define FORCE_USED __attribute__((used))
//#elif defined(_MSC_VER)
//#define FORCE_USED __declspec(dllexport)
#else
#define FORCE_USED
#endif

/**
* X(state, stage)
*/
#define UNIT_TEST_STATES(X) \
X(SETTING, "ctor()") \
X(TESTING, "Test()") \
X(TEARING, "dtor()") \
X(SUCCESS, "") \
X(ANOMALY, "") \
X(UNKNOWN, "") \
X(FAILURE, "")

class UnitTest : public std::exception
{
public:
	enum eSTATE {
		SETTING = -3
		#define X(state, stage) state,
		#define SETTING
			UNIT_TEST_STATES(X)
		#undef X
		#undef SETTING
	};

	typedef struct Colonne {
		const char* label;
		const char* stage;
	} Colonne;

	static Colonne STATUS(eSTATE state) {
		static Colonne _states[] = {
			#define X(state, stage) {#state, stage},
				UNIT_TEST_STATES(X)
			#undef X
		};
		return _states[state + 3];
	}

	UnitTest() : units(0), spent(0), worse(SETTING) {}

	UnitTest(std::string name) : UnitTest() { title = name; }

	UnitTest(eSTATE state, std::string what) : UnitTest() { setState(state, what); }

	virtual ~UnitTest() { if (runner() == this) runAll(); }

	virtual const char* what() const throw()
	{
		return whats.c_str();
	}

	/*****************************************************************************
	* Suite filter  (wildcards: ? * ^ !)
	******************************************************************************/
	struct Suite { Suite(std::string wc) { suite(wc); } };

	/*****************************************************************************
	* Tools
	******************************************************************************/
	static void dprintf(const char* format, ...)
	{
		if (format)
		{
			va_list args;
			va_start(args, format);
			int size = vprintf(format, args);
			va_end(args);

			if (size > 0) {
				std::string sOut(size, '\0');
				va_start(args, format);
				vsnprintf(&sOut[0], size + 1, format, args);
				va_end(args);

				#if defined(_WIN32)
				OutputDebugStringA(sOut.c_str());
				#endif
			}
		};
	}

	static long usElapse(long usOld)
	{
		long long usNow = 0;

		#ifdef _WIN32
		LARGE_INTEGER frequency, counter;
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&counter);
		usNow = (counter.QuadPart * 1000000LL) / frequency.QuadPart;
		#else
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		usNow = (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
		#endif

		return (long)(usNow - usOld);
	}

	/*****************************************************************************
	* Assert
	* Use strcmp() or wcscmp() to compare deux arrays of characters.
	******************************************************************************/
	template <class A>
	static void _assert(const A& expression, const char* shouldbe = 0, ...)
	{
		if (!expression)
		{
			if (shouldbe)
			{
				va_list args;
				va_start(args, shouldbe);
				int size = vsnprintf(0, 0, shouldbe, args);
				va_end(args);

				if (size > 0) {
					std::string what(size, '\0');
					va_start(args, shouldbe);
					vsnprintf(&what[0], size + 1, shouldbe, args);
					va_end(args);
					throw UnitTest(FAILURE, what);
				}
			};

			throw UnitTest(FAILURE, "");
		};
	}

	/*****************************************************************************
	* Report / Resume
	******************************************************************************/
	virtual std::string report(eSTATE state, std::string where, std::string what)
	{
		return ssprintf("\t%s : %s - %s\n", STATUS(state).label, where.c_str(), what.c_str());
	}

	virtual void resume(int count, int total, long usec, eSTATE state, std::string reports, std::string filters)
	{
		dprintf("\n");
		dprintf(reports.c_str());
		dprintf("\t----------------------------------------\n"
			"\tExecuted: %d/%d %s, %.3fs at %s\n"
			"\tResulted: %s\n"
			"\tFiltered: %s\n",
			count,
			total,
			count > 1 ? "units" : "unit",
			usec / 1e6,
			localDate().c_str(),
			STATUS(state).label,
			filters.c_str());
	}

	/*****************************************************************************
	* Run
	******************************************************************************/
	int runAll(std::string wildcard = "")
	{
		units = 0;
		spent = 0;
		worse = SETTING;
		whats = "";
		issue = "";
		which = wildcard.empty() ? suite() : wildcard;

		std::vector<std::string> tokens;
		split(which, ';', tokens);

		std::map<std::string, test_func>::iterator it;
		for (it = tests().begin(); it != tests().end(); it++) {
			for (size_t i = 0; i < tokens.size(); ++i) {
				if (wcMatch(it->first.c_str(), tokens[i].c_str())) {
					it->second(this);
					break;
				}
			}

			if(tokens.size() == 0) it->second(this);
		}

		if (units == 0) worse = SUCCESS;
		resume(units, tests().size(), spent, worse, whats, which);
		return worse;
	}

	/*****************************************************************************
	* Utilities
	******************************************************************************/
	static std::string localDate()
	{
		// use environment's locale for %x
		setlocale(LC_TIME, "");
		time_t now = time(0);
		char buf[20];
		struct tm timeinfo;

#ifdef _MSC_VER
		localtime_s(&timeinfo, &now);
#else
		localtime_r(&now, &timeinfo);
#endif

		strftime(buf, sizeof(buf), "%x %H:%M:%S", &timeinfo);
		return buf;
	}

	static std::string ssprintf(const char* format, ...)
	{
		if (format)
		{
			va_list args;
			va_start(args, format);
			int size = vsnprintf(0, 0, format, args);
			va_end(args);

			if (size > 0) {
				std::string sOut(size, '\0');
				va_start(args, format);
				vsnprintf(&sOut[0], size + 1, format, args);
				va_end(args);
				return sOut;
			}
		};
		return "";
	}

	static void split(const std::string& s, char delim, std::vector<std::string>& out)
	{
		out.clear();
		for (size_t start = 0, end; start < s.size(); start = end + 1) {
			end = s.find(delim, start);
			if (end == std::string::npos) { out.push_back(s.substr(start)); break; }
			if (end > start) out.push_back(s.substr(start, end - start));
		}
	}

	static bool wcMatch(const char* str, const char* wc)
	{
		switch (*wc) {
		case '\0': return *str == '\0';
		case '!': return wc[1] && !wcMatch(str, wc + 1);//negate entire
		case '*': while (wc[1] == '*') ++wc;
			return wc[1] == '\0' || (*str && wcMatch(str + 1, wc)) || wcMatch(str, wc + 1); //any sequence
		case '^': return wc[1] && *str && *str != wc[1] && wcMatch(str + 1, wc + 2); //negate single
		case '?': return *str && wcMatch(str + 1, wc + 1);//any single
		default: return *str == *wc && wcMatch(str + 1, wc + 1);
		}
	}

	template <class T> friend class Unit;
private:
	int units;
	long spent;
	eSTATE worse;
	std::string whats, which, issue, title;

	bool setState(eSTATE state, std::string what = "", std::string stage = "") {
		if (issue.empty()) {
			worse = state;
			whats = what;
			issue = stage;
			return true;
		}
		return false;
	}

	void addResult(UnitTest& result) {
		std::string where = result.title;
		if (!result.issue.empty()) where += "::" + result.issue;

		spent += result.spent;
		whats += report(result.worse, where, result.whats);
		if (result.worse > worse) worse = result.worse;
	}

	// Wildcards separated by ';'
	static std::string suite(std::string wildcard = "") {
		static std::string _pattern;
		if (!wildcard.empty()) {
			if (_pattern.empty()) _pattern = wildcard;
			else {
				if (_pattern.find_first_of("?*^!") != std::string::npos) {
					if (wildcard.find_first_of("?*^!") != std::string::npos)
						_pattern += ";" + wildcard;
					else _pattern = wildcard; // Only
				}
			}
		}
		return _pattern;
	}

	static UnitTest*& runner() {
		static UnitTest* _runner = NULL;
		return _runner;
	}

	typedef void (*test_func)(UnitTest* _this);
	static std::map<std::string, test_func>& tests()
	{
		static std::map<std::string, test_func> _tests;
		return _tests;
	}
};

template<class T>
class Unit : public UnitTest {
public:
	//Only this test
	class Only : public Unit {};
	//Skip this test
	class Skip : public Unit {};
	//Reject this test
	void reject(std::string what = "") {
		if (pResult) {
			setState(TEARING, what, STATUS(TEARING).stage);
		}
		else {
			whats = what;
			throw* static_cast<T*>(this);
		}
	}

	virtual void Test() = 0;

	Unit() : pResult(NULL) {
		spent = usElapse(0);
	}

	virtual ~Unit()
	{
		//Attention: double destruction : the original object and the thrown copy

		spent = usElapse(spent);
		if (!std::uncaught_exception()) {
			setState(SUCCESS, ssprintf("%.3fs", spent / 1e6), STATUS(SUCCESS).stage);
			if (pResult) {
				pResult->setState(worse, whats, issue);
			}
		}

		/* FORCE_USED */
		_pax;
	}

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L) //>=C++11

	//Match integral, floating point, boolean, char, enum, char*, char[] and const char*.
	template <typename A,
		typename std::enable_if<!std::is_base_of<std::string, typename std::decay<A>::type>::value>::type* = nullptr>
		static const A& c_val(const A& value)
	{
		//nested type using typename
		return value;
	}

	//Match std::string.
	static const char* c_val(const std::string& value)
	{
		return value.c_str();
	}

	template <class ... Arg>
	static void dprintf(const std::string& format, const Arg& ... arg)
	{
		UnitTest::dprintf(c_val(format), c_val(arg) ...);
	}

	template <class A, class ... Arg>
	static void _assert(const A& expression, const std::string& shouldbe, const Arg&... arg)
	{
		UnitTest::_assert(expression, c_val(shouldbe), c_val(arg) ...);
	}
#endif

	static std::string name()
	{
		std::string n = typeid(T).name();
		if (n.substr(0, 7) == "struct ") return n.substr(7);
		if (n.substr(0, 6) == "class ") return n.substr(6);
		return n;
	}

private:

	static void runTest(UnitTest* _this)
	{
		runner() = NULL;

		++_this->units;
		UnitTest result(name());

		try
		{
			T t;

			//To access private method Test()
			Unit<T>* p = &t;

			if (p->setState(TESTING)) {
				result.setState(TESTING);//stage
				p->Test();
			}

			p->pResult = &result;
		}
		catch (const std::exception& e)
		{
			//Catch UnitTest instances duplicated 
			const UnitTest* ut = dynamic_cast<const UnitTest*>(&e);
			result.setState(ut ? ut->worse : ANOMALY, e.what(), STATUS(result.worse).stage);
		}
		catch (...)
		{
			result.setState(UNKNOWN, "unknown exception", STATUS(result.worse).stage);
		}

		_this->addResult(result);
	}

	static void setRunner()
	{
		static UnitTest ut;
		runner() = &ut;
	}

	static UnitTest* initialize()
	{
		if (std::is_base_of<Only, T>::value) suite(name());
		if (std::is_base_of<Skip, T>::value) suite("!" + name());

		tests()[name()] = runTest;
		//Last declared and first destroyed
		setRunner();
		return NULL;
	}

	UnitTest* pResult;

	static UnitTest* FORCE_USED _pax;
};

template<class T>
UnitTest* Unit<T>::_pax = Unit<T>::initialize();

/// For VC++ 6.0, the default internal heap limit(/Zm100,50MB) can reach to 1259 tests in total; 
/// use /Zm to specify a higher limit
/// 
/// To access private methods via public abstract interface or using friend
///
/// Example:
#if 0
/* Add this header file into your project */
#include "CppUnitTestExpress.h"

/* Write a unit test given a name. */
class Example : public Unit<Example>
{
public:
	Example()
	{
		/* Set up */
	}

	void Test()
	{
		/* Throw FAILURE if false.*/
		_assert(true, "It should be true.");
	}

	~Example()
	{
		/* Tear down */
	}
};

/* Run and report your unit test. */
int main(int argc, char* argv[])
{
	UnitTest ut;
	ut.runAll("Example"); //Run and report your unit test. 
	ut.runAll("Ex*");  //Run and report all unit tests whose names begin with "Ex".
	ut.runAll(); //Run and report all unit tests.
	return 0;
}

/* Run and report all unit tests by default. */
int main(int argc, char* argv[])
{
	return 0;
}
#endif
#endif //_CPP_UNIT_TEST_EXPRESS_H_