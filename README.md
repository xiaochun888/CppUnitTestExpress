# CppUnitTestExpress
 A enough simple and cross-platform unit test framework for C++, CuteX for short.
 It is just a very small header file that uses the C/C++ peculiar features (X marco, template specialization, recursive and variadic, ...) to simplify testing work.
 
 **There is only one assertion tool to complete all test cases, with no learning time.**
 
 **You just write a unit test no matter where, and the compiler does the rest for you.**
 
## Main features:

It is designed to let users focus on writing test scenarios. Each test unit is a native struct or class, so there are no "weird" test fixtures. The only assertion tool is normal method or function, so avoids using macros in a unit test.

1. Only a header file, only use C++ compiler.
2. Any member method or function can be tested.
3. No concept to learn, no heavy documentation, just one assert tool.
4. No config, no test macro, no graphic interface, no extrernal library.
5. One test or a group of tests can be executed by name in wild card.
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

/* Write a unit test given a name. */
struct TestMinimal : public Unit<TestMinimal>
{
	void Test()
	{
		_assert(true,"Should be true.");
	}
};

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
