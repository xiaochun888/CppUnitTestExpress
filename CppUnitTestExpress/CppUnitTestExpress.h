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
	Please see an example in the end of this page.
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
#ifdef _WIN32 //VC++ 7.0
#include <windows.h>
#pragma warning (disable:4996)
#endif

#define UNIT_TEST_RESULTS \
X(OK, "Success") \
X(ZB, "Teardown failed") \
X(BT, "Setup failed") \
X(EX, "Exception") \
X(KO, "Failed")

static const char* UNIT_TEST_RESULT_TEXT[] =
{
	#define X(e, s) (const char*)s,
	UNIT_TEST_RESULTS
	#undef X
};

class UnitTest
{
public:
	/*****************************************************************************
	* Initialization
	******************************************************************************/
	UnitTest()
	{
		units=0;
		state=0;
		worst=0;
		elapsed=0;
		what="";
	};

	UnitTest(int state, const char* what)
	{
		UnitTest();
		this->state=state;
		this->what=what;
		result();
	}

	enum {
		#define X(e, s) e,
			UNIT_TEST_RESULTS
		#undef X
	};

	/*****************************************************************************
	* Utilities
	******************************************************************************/
	static long usElapse(long uold)
	{
		#ifdef _WIN32
		struct timeval 
		{
		  long tv_sec;
		  long tv_usec;
		} tv;

		FILETIME time;
		double   timed;
		
		GetSystemTimeAsFileTime( &time );
		timed = ((time.dwHighDateTime * 4294967296e-7) - 11644473600.0) +
				(time.dwLowDateTime  * 1e-7);
		
		tv.tv_sec  = (long) timed;
		tv.tv_usec = (long) ((timed - tv.tv_sec) * 1e6);
		#else
		struct timeval tv;
		gettimeofday(&tv,NULL);
		#endif

		return (tv.tv_sec * 1000000 + tv.tv_usec - uold);
	}

	static void dprintf(const char* format, ...)
	{
		if(format)
		{
			va_list body;
			va_start(body, format);

			#if defined(_WIN32)
			char output[4096]={0};
			vsprintf(output, format, body);
			OutputDebugStringA(output);
			#endif

			vprintf(format, body);

			va_end(body);
		};
	}

	//Match wild card characters  ?  and  *.
	bool wcMatch(const char* str, const char* pattern)
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
	/*****************************************************************************
	* Test assert
	* Use strcmp() or wcscmp() to compare deux arrays of characters.
	******************************************************************************/
	template <class A>
	static void _assert(const A& expression, const char* shouldbe=0, ...)
	{
		if(!expression)
		{ 
			char what[4096]={0};
			if(shouldbe)
			{
				va_list args;
				va_start(args, shouldbe);
				vsprintf(what, shouldbe, args);
				va_end(args);
			};

			throw UnitTest(UnitTest::KO, what);
		};
	}

	/*****************************************************************************
	* Test report
	******************************************************************************/
	virtual int result()
	{
		if(state>worst) worst=state;
		return worst;
	}

	virtual void report(std::string unitName)
	{
		dprintf("\t%s : %s - %s\n",
				unitName.c_str(), 
				UNIT_TEST_RESULT_TEXT[state],
				what.c_str());
	}

	virtual int resume()
	{
		dprintf("\t----------------------------------------\n"
				"\tExecuted: %d unit(s), %ld us\n"
				"\tResulted: %s\n\n",
				units,
				elapsed,
				UNIT_TEST_RESULT_TEXT[worst]);
		return worst;
	}

	/*****************************************************************************
	* Test execution
	******************************************************************************/
	virtual void runAll()
	{
		std::map<std::string, func>::iterator it;
		for (it = unitTests().begin(); it != unitTests().end(); it++){
			it->second(this);
		}
	}

	//pattern possiblly includes the wildcard characters  ?  and  *.
	virtual void runAll(std::string pattern)
	{
		std::map<std::string, func>::iterator it;
		for (it = unitTests().begin(); it != unitTests().end(); it++){
			if(wcMatch(it->first.c_str(), pattern.c_str())) {
				it->second(this);
			}
		}
	}

	template <class T> friend class Unit;
protected:
	int units;
	int state;
	int worst;
	long elapsed;
	std::string what;

	typedef void (*func)(UnitTest* _this);
	static std::map<std::string, func>& unitTests() {
		static std::map<std::string, func> tests;
		return tests;
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
		int which=0;
		std::string what="";
		long elapsed = _this->usElapse(0);

		try
		{
			++_this->units;

			which=UnitTest::BT; what=name()+"()";
			T t;

			which=UnitTest::EX; what="Test()";
			//To access private method Test()
			Unit<T>* p = &t;
			p->Test();

			which=UnitTest::ZB; what="~"+name()+"()";
		}
		catch(UnitTest& e)
		{
			if(e.state == UnitTest::OK)
			{
				--_this->units;
				return;
			}

			elapsed=usElapse(elapsed);
			_this->elapsed += elapsed;
			_this->state=e.state;
			_this->result();

			_this->what = e.what;
			_this->report(name().c_str());
			return;
		}
		catch(...)
		{
			elapsed=usElapse(elapsed);
			_this->elapsed += elapsed;
			_this->state=which;
			_this->result();

			_this->what="Exception in "+what;
			_this->report(name().c_str());
			return;
		}

		elapsed=usElapse(elapsed);
		_this->elapsed += elapsed;
		_this->state=UnitTest::OK;
		_this->result();

		char desc[1024] = {0};
		sprintf(desc,"Test OK, time elapsed %ld us", elapsed);
		_this->what = desc;
		_this->report(name().c_str());
	}

	/**Record this test */
	static T* initialize()
	{
		unitTests()[name()] = runTest;
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
		_assert(true, "It is true.");
	}

	~TestOne()
	{
		/* Tear down */
	}
};

int main(int argc, char* argv[])
{
	/* Run and report your unit test. */
	UnitTest ut;
	ut.runAll();
	return ut.resume();
}
#endif
#endif //_CPP_UNIT_TEST_EXPRESS_H_
