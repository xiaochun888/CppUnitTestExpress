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
#include <time.h>
#include <typeinfo>
#include <string>
#include <map>

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

#define UNIT_TEST_STATES \
X(SETTING, "::ctor()") \
X(TESTING, "::Test()") \
X(TEARING, "::dtor()") \
X(SUCCESS, "") \
X(ANOMALY, "") \
X(UNKNOWN, "") \
X(FAILURE, "")

class UnitTest
{
public:
	enum STATE {
		SETTING = -3
		#define X(e, s) e,
		#define SETTING
			UNIT_TEST_STATES
		#undef X
		#undef SETTING
	};

	static const char** NAMES(STATE state) {
		static const char* _names[][2] = {
			#define X(e,s) {#e, s},
				UNIT_TEST_STATES
			#undef X
		};
		return _names[state + 3];
	}

	UnitTest()
	{
		units = 0;
		spent = 0;
		setState(SETTING);
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
	* Test tools
	******************************************************************************/
	static void dprintf(const char* format, ...)
	{
		if (format)
		{
			va_list args;
			va_start(args, format);
			vprintf(format, args);

			#if defined(_WIN32)
				OutputDebugStringA(vssprintf(format, args).c_str());
			#endif

			va_end(args);
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
			std::string what;
			if (shouldbe)
			{
				va_list args;
				va_start(args, shouldbe);
				what = vssprintf(shouldbe, args);
				va_end(args);
			};

			throw UnitTest(FAILURE, what);
		};
	}

	/*****************************************************************************
	* Test report
	******************************************************************************/
	virtual std::string report(std::string where, STATE state, std::string what)
	{
		return ssprintf("\t%s : %s - %s\n", NAMES(state)[0], where.c_str(), what.c_str());
	}

	virtual void resume(int count, int total, long usec, STATE state, std::string whats, std::string match)
	{
		std::string sMatch = match.empty() ? "" : "Matching: " + match + "\n";

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
				NAMES(state)[0],
				sMatch.c_str());
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
		where = "";
		which = wildcard.empty() ? match() : wildcard;

		std::map<std::string, test_func>::iterator it;
		for (it = tests().begin(); it != tests().end(); it++){
			if(which.empty() || wcMatch(it->first.c_str(), which.c_str())) {
				it->second(this);
			}
		}

		resume(units, tests().size(), spent, worse, whats, which);
		return worse;
	}

	/*****************************************************************************
	* Utilities
	******************************************************************************/
	static std::string localDate() {
		time_t ttNow = time(0);
		struct tm tmDay;

		#ifdef _WIN32
		localtime_s(&tmDay, &ttNow);
		#else
		localtime_r(&ttNow, &tmDay);
		#endif

		char strDate[20];
		strftime(strDate, sizeof strDate, "%x %H:%M:%S", &tmDay);
		return strDate;
	}

	static std::string vssprintf(const char* format, va_list args)
	{
		std::string sOut;
		if (format)
		{
			int size = vsnprintf(0, 0, format, args) + 1;
			char* buf = new char[size];
			vsnprintf(buf, size, format, args);
			sOut = buf;
			delete[] buf;
		};
		return sOut;
	}

	static std::string ssprintf(const char* format, ...)
	{
		std::string sOut;
		if (format)
		{
			va_list args;
			va_start(args, format);
			sOut = vssprintf(format, args);
			va_end(args);
		};
		return sOut;
	}

	// The wildcard characters optionally include ? , *, ^ and !.
	static bool wcMatch(const char* str, const char* wildcard)
	{
		if (*wildcard == '\0' && *str == '\0')
			return true;

		// negate entire str
		if (*wildcard == '!' && *(wildcard + 1) != '\0') {
			return !wcMatch(str, wildcard + 1);
		}

		if (*wildcard == '*')
			while (*(wildcard + 1) == '*') wildcard++;

		if (*wildcard != '\0' && *str == '\0')
			return (*wildcard == '*' && *(wildcard + 1) == '\0');

		if (*wildcard == '?' || *wildcard == *str)
			return wcMatch(str + 1, wildcard + 1);

		// negate first character
		if (*wildcard == '^' && *(wildcard + 1) != '\0') {
			if (*str == *(wildcard + 1))
				return false;
			return wcMatch(str + 1, wildcard + 2);
		}

		if (*wildcard == '*')
			return wcMatch(str, wildcard + 1) || wcMatch(str + 1, wildcard);

		return false;
	}

	template <class T> friend class Unit;
private:
	int units;
	long spent;
	STATE worse;
	std::string whats;
	std::string which;
	std::string where;

	void setState(STATE state, std::string what = "") {
		worse = state;
		whats = what;
	}

	void addResult(UnitTest& result) {
		spent += result.spent;
		whats += report(result.where, result.worse, result.whats);
		if (result.worse > worse) worse = result.worse;
	}

	// The wildcard characters optionally include ? , *, ^ and !.
	static std::string match(std::string wildcard = "") {
		static std::string _match;
		if (!wildcard.empty()) {
			if (_match.empty()) {
				_match = wildcard;
			}
		}
		return _match;
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

//Only this test
class Only {};

template<class T>
class Unit : public UnitTest {
public:
	virtual void Test() = 0;

	void setResult(STATE state, std::string what) {
		if (_result->where.empty()) {
			_result->where = name() + NAMES(worse)[1];
			_result->setState(state, what);
		}
	}

	Unit() {
		// Not thread-safe here
		_result = _car;
		spent = usElapse(0);
	}

	virtual ~Unit()
	{
		spent = usElapse(spent);
		if (!std::uncaught_exception()) {
			UnitTest::setState(SUCCESS, ssprintf("%.3fs", spent / 1e6));
		}

		//Avoid double destruction : the original object and the thrown copy
		if (_result->spent == 0) {
			_result->spent = spent;
			setResult(worse, whats);
		}

		/* FORCE_USED */
		_car;
	}

	#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L) //>=C++11
		
		//Match integral, floating point, boolean, char or enum.
		template <typename A, typename std::enable_if<!std::is_base_of<std::string, A>::value>::type* = nullptr>
		static A c_val(const A& value)
		{
			//nested type using typename
			return value;
		}

		//Match std::string, const char* or char[].
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
	void runTest() {
		UnitTest::setState(TESTING);
		Test();
		UnitTest::setState(TEARING);
	}

	static void runTest(UnitTest* _this)
	{
		runner() = NULL;

		++_this->units;
		UnitTest result;
		_car = &result;

		try
		{
			T t;

			//To access private method Test()
			Unit<T>* p = &t;
			p->runTest();
		}
		catch (const UnitTest& e)
		{
			result.setState(e.worse, e.whats);
		}
		catch (const std::exception& e)
		{
			result.setState(ANOMALY, e.what());
		}
		catch(...)
		{
			result.setState(UNKNOWN, "unknown exception");
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
		if (std::is_base_of<Only, T>::value) match(name());

		tests()[name()] = runTest;
		//Last declared and first destroyed
		setRunner();
		return NULL;
	}

	UnitTest* _result;

	static UnitTest* FORCE_USED _car;
};

template<class T>
UnitTest* Unit<T>::_car = Unit<T>::initialize();

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
