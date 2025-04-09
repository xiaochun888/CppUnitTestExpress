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
#include <map>
#include <string>
#include <typeinfo>
#include <stdlib.h>
#ifdef _WIN32 //VC++ 7.0
#include <windows.h>
#pragma warning (disable:4996)
#endif

#define UNIT_TEST_STATES \
X(SUCCESS, "Success") \
X(COMMENT, "Comment") \
X(SETTING, "Setting") \
X(TESTING, "Testing") \
X(TEARING, "Tearing") \
X(UNKNOWN, "Unknown") \
X(FAILURE, "Failure")

class UnitTest
{
public:
	enum STATE {
		#define X(e, s) e,
			UNIT_TEST_STATES
		#undef X
			LASTONE
	};

	static const char* stateText(STATE state) {
		#define X(e, s) if(e==state) return(const char*)s;
			UNIT_TEST_STATES
		#undef X
		return "";
	};

	/*****************************************************************************
	* Initialization
	******************************************************************************/
	UnitTest()
	{
		units = 0;
		elapsed = 0;
		whats = "";
		worse = SUCCESS;
	}

	UnitTest(STATE state, std::string what) : UnitTest()
	{
		whats = what;
		worse = state;
	}

	~UnitTest() {
		//Last declared and first destroyed
		if (worse == LASTONE) {
			worse = SUCCESS;
			runAll();
		}
	}
	/*****************************************************************************
	* Utilities
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

	static long usElapse(long uold)
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
			struct timeval tv;
			gettimeofday(&tv, NULL);
		#endif

		return (tv.tv_sec * 1000000 + tv.tv_usec - uold);
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
	virtual void report(std::string name, STATE state, std::string what)
	{
		whats += ssprintf("\t%s : %s - %s\n", stateText(state), name.c_str(), what.c_str());
	}

	virtual int resume()
	{
		time_t now = time(0);
		struct tm tmLocal = *localtime(&now);
		char sDateISO[sizeof "2022-08-23T10:40:20Z"];
		strftime(sDateISO, sizeof sDateISO, "%Y-%m-%d %H:%M:%S", &tmLocal);

		dprintf(whats.c_str());
		dprintf("\t----------------------------------------\n"
				"\tExecuted: %d %s, %lgs at %s\n"
				"\tResulted: %s\n\n",
				units,
				units > 1 ? "units":"unit",
				elapsed / 1e6,
				sDateISO,
				stateText(worse));
		return worse;
	}

	/*****************************************************************************
	* Test execution
	******************************************************************************/
	//pattern possiblly includes the wildcard characters  ?  and  *.
	virtual int runAll(std::string pattern = "")
	{
		std::map<std::string, test_func>::iterator it;
		for (it = runTests().begin(); it != runTests().end(); it++){
			if(pattern.empty() || wcMatch(it->first.c_str(), pattern.c_str())) {
				it->second(this);
			}
		}

		return this->resume();
	}

	template <class T> friend class Unit;
protected:
	int units;
	long elapsed;
	std::string whats;
	STATE worse;

	static  UnitTest*& lastOne() {
		static UnitTest* _lastOne = NULL;
		return _lastOne;
	}

	int result(STATE state)
	{
		if (state > worse) worse = state;
		return worse;
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

	//Match wild card characters  ?  and  *.
	static bool wcMatch(const char* str, const char* pattern)
	{
		if (*pattern == '\0' && *str == '\0')
			return true;

		if (*pattern == '*')
			while (*(pattern + 1) == '*') pattern++;

		if (*pattern == '*' && *(pattern + 1) != '\0' && *str == '\0')
			return false;

		if (*pattern == '?' || *pattern == *str)
			return wcMatch(str + 1, pattern + 1);

		if (*pattern == '*')
			return wcMatch(str, pattern + 1) || wcMatch(str + 1, pattern);

		return false;
	}

	typedef void (*test_func)(UnitTest* _this);
	static std::map<std::string, test_func>& runTests()
	{
		static std::map<std::string, test_func> funcs;
		return funcs;
	}
};

template<class T>
class Unit : public UnitTest {
public:
	virtual void Test() = 0;

	virtual ~Unit()
	{
		/* force specialization */
		_t;
	}

	#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L) //>=C++11
		//nested type using typename
		template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
		static T c_arg(const T& value)
		{
			//(integral, floating point, boolean, or char
			return value;
		}
		//std::string or const char*
		static const char* c_arg(const std::string& value)
		{
			return value.c_str();
		}

		template <class ... Arg>
		static void dprintf(const std::string& format, const Arg& ... arg)
		{
			UnitTest::dprintf(c_arg(format), c_arg(arg) ...);
		}

		template <class A, class ... Arg>
		static void _assert(const A& expression, const std::string& shouldbe, const Arg&... arg)
		{
			UnitTest::_assert(expression, c_arg(shouldbe), c_arg(arg) ...);
		}
	#endif

protected:
	static std::string name()
	{
		std::string className = typeid(T).name();
		std::string typeName = className.substr(0, 6);
		if(typeName == "struct") return className.substr(7); //remove "struct "
		return className.substr(6); //remove "class "
	}

	static void runTest(UnitTest* _this)
	{
		//Disable last one
		if(lastOne() != _this) lastOne()->worse = SUCCESS;

		++_this->units;
		UnitTest ut;
		long elapsed = _this->usElapse(0);
		try
		{
			ut.worse = SETTING;
			T t;

			ut.worse = TESTING;
			//To access private method Test()
			Unit<T>* p = &t;
			p->Test();

			ut.worse = TEARING;
		}
		catch(UnitTest& e)
		{
			ut.elapsed = _this->usElapse(elapsed);
			ut.worse = e.worse;
			ut.whats = e.whats;
		}
		catch(...)
		{
			ut.elapsed = _this->usElapse(elapsed);
			switch (ut.worse) {
			case SETTING:
				ut.whats = name() + "()";
				break;
			case TESTING:
				ut.whats = "Test()";
				break;
			case TEARING:
				ut.whats = "~" + name() + "()";
				break;
			}
			ut.worse = UNKNOWN;
		}

		if (ut.worse == TEARING && ut.elapsed == 0) {
			ut.elapsed = _this->usElapse(elapsed);
			ut.worse = SUCCESS;
			ut.whats = ssprintf("%lgs", ut.elapsed / 1e6);
		}

		_this->elapsed += ut.elapsed;
		_this->result(ut.worse);
		_this->report(name(), ut.worse, ut.whats);
	}

	//Save last declared
	static void setLastOne()
	{
		static UnitTest ut(LASTONE, "");
		//Disable previous
		if (lastOne()) lastOne()->worse = SUCCESS;
		lastOne() = &ut;
	}

	static T* initialize()
	{
		runTests()[name()] = runTest;
		setLastOne();
		return NULL;
	}

	static T* _t;
};

template<class T>
T* Unit<T>::_t = Unit<T>::initialize();

/// For VC++ 6.0, the default internal heap limit(/Zm100,50MB) can reach to 1259 tests in total; 
/// use /Zm to specify a higher limit
/// 
/// For VC++ 7.0 and later, add "template<>" before every unit test structure
/// Tested on SunOS 5.5.1
///
/// To access private methods via public abstract interface or using friend
///
/// Example:
#if 0
/* Add this header file into your project */
#include "CppUnitTestExpress.h"

/* Write a unit test given a name. */
class TestOne : public Unit<TestOne>
{
public:
	TestOne()
	{
		/* Set up */
	}

	void Test()
	{
		/* throw comment */
		throw UnitTest(SUCCESS, "No implementation");
		throw UnitTest(COMMENT, "No implementation");

		/* throw unknown exception */
		throw this;

		/* Assert : _assert */
		_assert(true, "It should be true.");
	}

	~TestOne()
	{
		/* Tear down */
	}
};

int main(int argc, char* argv[])
{
	UnitTest ut;
	ut.runAll("TestOne"); //Run and report your unit test. 
	ut.runAll("Test*");  //Run and report all unit tests whose names begin with Test.
	return ut.runAll(); //Run and report all unit tests.
}
#endif
#endif //_CPP_UNIT_TEST_EXPRESS_H_
