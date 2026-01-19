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
#include <ctime>
#include <typeinfo>
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

class UnitTest
{
public:
	enum STATE {
		SETTING = -3
		#define X(state, stage) state,
		#define SETTING
			UNIT_TEST_STATES(X)
		#undef X
		#undef SETTING
	};


	static const char* STATUS(STATE state) {
		static const char* _states[] = {
			#define X(state, stage) #state,
				UNIT_TEST_STATES(X)
			#undef X
		};
		return _states[state + 3];
	}

	static const char* STAGE(STATE state) {
		static const char* _stages[] = {
			#define X(state, stage) stage,
				UNIT_TEST_STATES(X)
			#undef X
		};
		return _stages[state + 3];
	}

	UnitTest()
	{
		units = 0;
		spent = 0;
		setState(SETTING);
	}

	UnitTest(std::string name) : UnitTest()
	{
		title = name;
	}

	UnitTest(STATE state, std::string what) : UnitTest()
	{
		setState(state, what);
	}

	~UnitTest() {
		if (runner() == this) {
			runAll();
		}
	}

	/*****************************************************************************
	* Test suite
	* wildcard characters : ? , *, ^ and !
	******************************************************************************/
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

	/*****************************************************************************
	* Test tools
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
	* Test assert
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
	* Test state and report
	******************************************************************************/
	void setState(STATE state, std::string what = "", std::string when = "") {
		if (issue.empty()) {
			worse = state;
			whats = what;
			issue = when;
		}
	}

	virtual std::string report(std::string where, STATE state, std::string what)
	{
		return ssprintf("\t%s : %s - %s\n", STATUS(state), where.c_str(), what.c_str());
	}

	virtual void resume(int count, int total, long usec, STATE state, std::string whats, std::string match)
	{
		std::string pattern = match.empty() ? "" : "SetSuite: " + match + "\n";

		dprintf("\n");
		dprintf(whats.c_str());
		dprintf("\t----------------------------------------\n"
				"\tExecuted: %d/%d %s, %.3fs at %s\n"
				"\tResulted: %s\n"
				"\t%s\n",
				count,
				total,
				count > 1 ? "units" : "unit",
				usec / 1e6,
				localDate().c_str(),
				STATUS(state),
				pattern.c_str());
	}

	/*****************************************************************************
	* Test execution
	******************************************************************************/
	//The wildcard characters optionally include ? , *, and ^.
	int runAll(std::string wildcard = "")
	{
		units = 0;
		spent = 0;
		worse = SETTING;
		whats = "";
		issue = "";
		which = wildcard.empty() ? suite() : wildcard;
		
		std::string str = which;
		std::vector<std::string> tokens;
		split(str, ';', tokens);
		
		std::map<std::string, test_func>::iterator it;
		for (it = tests().begin(); it != tests().end(); it++){
			bool matched = true;
			for (size_t i = 0; i < tokens.size() && matched; ++i) {
				matched = wcMatch(it->first.c_str(), tokens[i].c_str());
			}

			if (matched) {
				it->second(this);
			}
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
		time_t now = time(0);
		char buf[20];
		strftime(buf, sizeof(buf), "%x %H:%M:%S", localtime(&now));
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

		size_t start = 0;
		size_t end;

		while ((end = s.find(delim, start)) != std::string::npos) {
			if (end > start) {
				out.push_back(s.substr(start, end - start));
			}
			start = end + 1;
		}

		if (start < s.size()) {
			out.push_back(s.substr(start));
		}
	}

	// The wildcard characters optionally include ? , *, ^ and !.
	static bool wcMatch(const char* str, const char* wc)
	{
		switch (*wc)
		{
		case '\0':
			return *str == '\0';
		case '!': // negate entire pattern
			return wc[1] ? !wcMatch(str, wc + 1) : false;
		case '*': // any sequence
			while (wc[1] == '*') ++wc;
			if (wc[1] == '\0') return true;
			return (*str && wcMatch(str + 1, wc)) || wcMatch(str, wc + 1);
		case '^': // single-character negation
			if (!wc[1] || !*str || *str == wc[1]) return false;
			return wcMatch(str + 1, wc + 2);
		case '?': // any single character
			return *str && wcMatch(str + 1, wc + 1);
		default: // literal character
			return (*str == *wc) && wcMatch(str + 1, wc + 1);
		}
	}

	template <class T> friend class Unit;
private:
	int units;
	long spent;
	STATE worse;
	std::string whats;
	std::string which;
	std::string issue;
	std::string title;

	void addResult(UnitTest& result) {
		std::string where = result.title;
		if(!result.issue.empty()) where += "::" + result.issue;

		spent += result.spent;
		whats += report(where, result.worse, result.whats);
		if (result.worse > worse) worse = result.worse;
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

	virtual void Test() = 0;

	Unit() {
		spent = usElapse(0);
	}

	virtual ~Unit()
	{
		//Attention: double destruction : the original object and the thrown copy

		spent = usElapse(spent);
		if (!std::uncaught_exception()) {
			setState(SUCCESS, ssprintf("%.3fs", spent / 1e6), STAGE(SUCCESS));
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
		std::string className = typeid(T).name();
		std::string typeName = className.substr(0, 6);
		if(typeName == "struct") return className.substr(7); //remove "struct "
		return className.substr(6); //remove "class "
	}

private:

	static void runTest(UnitTest* _this)
	{
		runner() = NULL;

		++_this->units;
		UnitTest result(name());

		try
		{	T t;

			//To access private method Test()
			Unit<T>* p = &t;

			p->setState(TESTING); //throw *this
			result.setState(TESTING); //where
			p->Test();
			p->pResult = &result;
		}
		catch (const UnitTest& e)
		{
			result.setState(e.worse, e.whats, STAGE(result.worse));
		}
		catch (const std::exception& e)
		{
			result.setState(ANOMALY, e.what(), STAGE(result.worse));
		}
		catch(...)
		{
			result.setState(UNKNOWN, "unknown exception", STAGE(result.worse));
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
