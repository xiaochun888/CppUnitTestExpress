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

using namespace std;

#define UNIT_TEST_RESULTS \
X(OK, "Success") \
X(GN, "Ignored") \
X(ZB, "TearDown") \
X(BT, "SetUp") \
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
			#else
			vprintf(format, body);
			#endif

			va_end(body);
		};
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

	template <class A>
	static void _ignore(const A&, const char* shouldbe=0,...)
	{
		char what[4096]={0};
		if(shouldbe)
		{
			va_list body;
			va_start(body, shouldbe);
			vsprintf(what, shouldbe, body);
			va_end(body);
		};
		throw UnitTest(UnitTest::GN, what);
	}

	/*****************************************************************************
	* Test report
	******************************************************************************/
	virtual int result()
	{
		if(state>worst) worst=state;
		return worst;
	}

	virtual void report(string unitName)
	{
		dprintf("\t%s : %s - %s\n",
				unitName.c_str(), 
				UNIT_TEST_RESULT_TEXT[state],
				what.c_str());
	}

	virtual int resume()
	{
		dprintf("\t----------------------------------------\n"
				"\tExecuted: %d unit(s), %ld ms\n"
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
		map<std::string, func>::iterator it;
		for (it = unitTests().begin(); it != unitTests().end(); it++){
			it->second(this);
		}
	}

	virtual void runAll(string keyword)
	{
		map<std::string, func>::iterator it;
		for (it = unitTests().begin(); it != unitTests().end(); it++){
			if(it->first.find(keyword) != string::npos) {
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
	string what;

	typedef void (*func)(UnitTest* _this);
	static map<string, func>& unitTests() {
		static map<string, func> tests;
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
	static string name()
	{
		string className = typeid(T).name();
		string typeName = className.substr(0, 6);
		if(typeName == "struct") return className.substr(7); //remove "struct "
		return className.substr(6); //remove "class "
	}

	static void runTest(UnitTest* _this)
	{
		int which=0;
		string what = "Exception in ";
		_this->elapsed=_this->usElapse(0);

		try
		{
			++_this->units;

			which=UnitTest::BT; what+=name()+"()";
			T t;

			which=UnitTest::EX; what+="Test()";
			t.Test();

			which=UnitTest::ZB; what+="~"+name()+"()";
		}
		catch(UnitTest& e)
		{
			if(e.state == UnitTest::OK)
			{
				--_this->units;
				return;
			}

			_this->elapsed=usElapse(_this->elapsed);
			_this->state=e.state;
			_this->result();

			_this->what = e.what;
			_this->report(name().c_str());
			return;
		}
		catch(...)
		{
			_this->elapsed=usElapse(_this->elapsed);
			_this->state=which;
			_this->result();

			_this->what = what;
			_this->report(name().c_str());
			return;
		}

		_this->elapsed=usElapse(_this->elapsed);
		_this->state=UnitTest::OK;
		_this->result();

		char desc[1024] = {0};
		sprintf(desc,"Test OK, time elapsed %ld ms", _this->elapsed);
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
		/* Assert : _assert, _ignore */
		_ignore("Code snippet","Not implemented.");
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
