# CppUnitTestExpress
 A minimal and cross-platform unit test framework for C++, CuteX for short.
 
## Main features:

It is designed for user to concentrate on writing a test scenario, and tests organization is automatically completed by compiler. Each test unit is a natural structure or class, so no “strange” test fixture.

1. Only a header file, only use C++ compiler.
2. Any member method or function, any C++ platform and down-level C++ compilers (e.g. VC6.0)
3. No concept to learn, no heavy documentation, just one assert tool.
4. No config, no test macro, no graphic interface, no extrernal library.
5. Auto-registration, auto-grouping in compile-time.

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

int main(int argc, char* argv[])
{
	/* Run and report your unit test. */
	UnitTest ut;
	ut.runAll();
	return ut.resume();
}
```
