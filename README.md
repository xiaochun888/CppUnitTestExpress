# CppUnitTestExpress
 A enough simple and cross-platform unit test framework for C++, CuteX for short.
 It is just a very small header file but uses the C/C++ peculiar features (X marco, template specialization and recursive, ...) to simplify test work, and there is only one assert tool to do all test cases without learning time.
 You just write a test case, compiler do the rest for you.
 
## Main features:

It is designed for user to concentrate on writing a test scenario, and tests organization is automatically completed by compiler. Each test unit is a natural structure or class, so no “strange” test fixture.

1. Only a header file, only use C++ compiler.
2. Any member method or function can be tested.
3. No concept to learn, no heavy documentation, just one assert tool.
4. No config, no test macro, no graphic interface, no extrernal library.
5. One test or a group of tests can be executed by name in wild card.
6. extended easily and integrated easily into an application.

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
struct TestMinimal : Unit<TestMinimal>
{
	void Test()
	{
		_assert(true,"It is true.");
	}
};

/* Run a unit test. */
int main(int argc, char* argv[])
{
	/* Run and report your unit test. */
	UnitTest ut;
	ut.runAll();
	return ut.resume();
}

/* Display the result of test . */
	TestMinimal : Success - Test OK, time elapsed 3953976 us
	----------------------------------------
	Executed: 1 unit(s), 3953976 us
	Resulted: Success
```
