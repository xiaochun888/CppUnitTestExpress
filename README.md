# CppUnitTestExpress
 A enough simple and cross-platform unit test framework for C++, CuteX for short.
 It is just a very small header file that uses the C/C++ peculiar features (template specialization, recursive and variadic, ...) to simplify testing work.
 
 **Only one assertion template, zero learning time.**
 
 **You just write a test unit, no matter where, and the compiler does the rest for you.**
 
## Main features:

It is designed to let users focus on writing test scenarios. Each test unit is a native struct or class, so there are no "weird" test fixtures. The only assertion tool is normal method, so avoids using macros in a unit test.

1. Only a header file, only use C++ compiler.
2. Any member method or function can be tested.
3. No concept to learn, no heavy documentation, just one assertion tool.
4. No config, no test macro, no graphic interface, no extrernal library.
5. Can run a test unit or group by wildcard name.
6. Extended easily and integrated easily into an application.

## Assertions:
```
template <class A>
static void _assert(const A& expression, const char* shouldbe=0, ...);
```
## A minimal example:

The below code snippet demonstrates the least amount of code required to write an executable test: 
```
/* Add this header file into your project */
#include "CppUnitTestExpress.h"

/* Write a test unit given a name. */
class TestMinimal : public Unit<TestMinimal>
{
	void Test()
	{
		_assert(true,"Should be true.");
	}
};

/* Or only this test is executed. */
class TestMinimal : public Unit<TestMinimal>::Only
{
	void Test()
	{
		_assert(true,"Should be true.");
	}
};

/* Or skip this test. */
class TestMinimal : public Unit<TestMinimal>::Skip
{
	void Test()
	{
		_assert(true,"Should be true.");
	}
};

/* Or add a test suite with wild card parttern. */
UnitTest::Suite("Test*");

/* Run the unit test. */
int main(int argc, char* argv[])
{
	return 0;
}

/* Display the result of test . */
	SUCCESS : TestMinimal - 0s
	----------------------------------------
	Executed: 1 unit, 0s at 2025-01-12 20:30:47
	Resulted: SUCCESS
```
