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

	static const char* stateName(STATE state) {
		#define X(e, s) if(e == state) return #e;
			UNIT_TEST_STATES
		#undef X
		return "";
	};

	static const char* stageName(STATE state) {
		#define X(e, s) if(e == state) return s;
			UNIT_TEST_STATES
		#undef X
		return "";
	};

	UnitTest()
	{
		units = 0;
		spent = 0;
		ended = false;
		whats = "";
		where = "";
		worse = SUCCESS;
	}

	UnitTest(STATE state, std::string what) : UnitTest()
	{
		worse = state;
		whats = what;
	}

	~UnitTest() {
		//Last declared and first destroyed
		if (ended) {
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
		#ifdef _WIN32
			struct timeval
			{
				long tv_sec;
				long tv_usec;
			} tv;

			FILETIME time;
			GetSystemTimeAsFileTime(&time);

			double timed = ((time.dwHighDateTime * 4294967296e-7) - 11644473600.0) +
				(time.dwLowDateTime * 1e-7);

			tv.tv_sec = (long)timed;
			tv.tv_usec = (long)((timed - tv.tv_sec) * 1e6);
		#else
			struct timeval
			{
				long tv_sec;
				long tv_usec;
			} tv;

			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			tv.tv_sec = ts.tv_sec;
			tv.tv_usec = ts.tv_nsec / 1000;
			// Handle potential overflow (though unlikely with normal values)
			if (ts.tv_nsec % 1000 >= 500) {
				tv.tv_usec += 1;  // Round up if necessary
			}
		#endif

		return (tv.tv_sec * 1000000 + tv.tv_usec - usOld);
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
	virtual std::string report(std::string stage, STATE state, std::string what)
	{
		return ssprintf("\t%s : %s - %s\n", stateName(state), stage.c_str(), what.c_str());
	}

	virtual void resume(int count, int total, long microseconds, STATE state, std::string reports, std::string wildcard)
	{
		std::string sWhich = wildcard.empty() ? "" : "Matching: " + wildcard + "\n";

		dprintf("\n");
		dprintf(reports.c_str());
		dprintf("\t----------------------------------------\n"
				"\tExecuted: %d/%d %s, %lgs at %s\n"
				"\tResulted: %s\n"
				"\t%s\n",
				count,
				total,
				count > 1 ? "units" : "unit",
				microseconds / 1e6,
				localDate().c_str(),
				stateName(state),
				sWhich.c_str());
	}

	/*****************************************************************************
	* Test execution
	******************************************************************************/
	//The wildcard characters optionally include ? , *, and ^.
	int runAll(std::string wildcard = "")
	{
		which = wildcard.empty() ? pattern() : wildcard;

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
		memset(&tmDay, 0, sizeof(struct tm));

		#ifdef _WIN32
		localtime_s(&tmDay, &ttNow);
		#else
		localtime_r(&ttNow, &tmDay);
		#endif

		char strDate[sizeof "2022-08-23T10:40:20Z"];
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

	//The wildcard characters optionally include ? , *, and ^.
	static bool wcMatch(const char* str, const char* wildcard)
	{
		if (*wildcard == '\0' && *str == '\0')
			return true;

		if (*wildcard == '*')
			while (*(wildcard + 1) == '*') wildcard++;

		if (*wildcard == '*' && *(wildcard + 1) != '\0' && *str == '\0')
			return false;

		if (*wildcard == '?' || *wildcard == *str)
			return wcMatch(str + 1, wildcard + 1);

		if (*wildcard == '*')
			return wcMatch(str, wildcard + 1) || wcMatch(str + 1, wildcard);

		if (*wildcard == '^')
			return !wcMatch(str, wildcard + 1);

		return false;
	}

	template <class T> friend class Unit;
private:
	int units;
	long spent;
	bool ended;
	STATE worse;
	std::string whats;
	std::string where;
	std::string which;

	STATE evolve(STATE state)
	{
		if (state > worse) worse = state;
		return worse;
	}

	//The wildcard characters optionally include ? , *, and ^.
	static std::string pattern(std::string wildcard = "") {
		static std::string _pattern;
		if (!wildcard.empty()) {
			if (_pattern.empty()) {
				_pattern = wildcard;
			}
		}
		return _pattern;
	}

	static void setFinal(UnitTest* ut) {
		static UnitTest* _final = NULL;
		if (_final) _final->ended = false;
		if (ut) ut->ended = true;
		_final = ut;
	}

	typedef void (*test_func)(UnitTest* _this);
	static std::map<std::string, test_func>& tests()
	{
		static std::map<std::string, test_func> _tests;
		return _tests;
	}
};

template <class T> class Only;
template <class T> class Skip;

template<class T>
class Unit : public UnitTest {
public:
	virtual void Test() = 0;

	Unit() {
		_ut = NULL;
		spent = usElapse(0);
	}

	Unit(STATE state, std::string what) : Unit() {
		worse = state;
		whats = what;
	}

	virtual ~Unit()
	#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L) //>=C++11
		noexcept(false)
	#else
		throw(std::exception)
	#endif
	{
		spent = usElapse(spent);
		if (_ut) {
			if (!std::uncaught_exception()) {
				//Avoid double destruction : the original object and the thrown copy
				if (_ut->worse < SUCCESS) {
					_ut->spent = spent;
					_ut->worse = SUCCESS;
					_ut->where = name() + stageName(_ut->worse);
					_ut->whats = ssprintf("%lgs", spent / 1e6);
				}
			}
		}

		/* force used */
		_t;
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
	UnitTest* _ut;

	static void runTest(UnitTest* _this)
	{
		UnitTest::setFinal(NULL);

		++_this->units;
		UnitTest ut;
		try
		{
			ut.worse = SETTING;
			T t;

			ut.worse = TESTING;
			//To access private method Test()
			Unit<T>* p = &t;
			p->Test();

			ut.worse = TEARING;
			t._ut = &ut;
		}
		catch (const UnitTest& e)
		{
			ut.where = name() + stageName(ut.worse);
			ut.worse = e.worse;
			ut.whats = e.whats;
		}
		catch (const std::exception& e)
		{
			ut.where = name() + stageName(ut.worse);
			ut.worse = ANOMALY;
			ut.whats = e.what();
		}
		catch(...)
		{
			ut.where = name() + stageName(ut.worse);
			ut.worse = UNKNOWN;
			ut.whats = "unknown exception";
		}

		_this->spent += ut.spent;
		_this->whats += _this->report(ut.where, ut.worse, ut.whats);
		_this->evolve(ut.worse);
	}

	static void setFinal()
	{
		static UnitTest ut;
		UnitTest::setFinal(&ut);
	}

	static T* initialize()
	{
		if (std::is_base_of<Only<T>, T>::value) pattern(name());
		if (std::is_base_of<Skip<T>, T>::value) pattern("^" + name());

		tests()[name()] = runTest;
		setFinal();
		return NULL;
	}

	static T* FORCE_USED _t;
};

template<class T>
T* Unit<T>::_t = Unit<T>::initialize();

//Only this test
template<class T>
class Only : public Unit<T> {};

//Skip this test
template<class T>
class Skip : public Unit<T> {};

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
