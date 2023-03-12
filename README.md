# CppUnitTestExpress
 A minimal and cross-platform unit test framework for C++, CuteX for short.
 
## Main features:

It is designed for user to concentrate on writing a test scenario, and tests organization is automatically completed by compiler. By the way, it follows the KISS principle so as to easily read and extend.
Each test unit is a natural structure or class, so no “strange” test fixture.

1. No configuration, just a header file.
2. No concept to learn, no heavy documentation.
3. No test macro, so no pitfall of macro possible.
4. No graphic interface and library, so no runtime overhead and overload.
5. Auto-registration, auto-grouping in compile-time.

## Assertions:

void _assert(const A& expression, const char* shouldbe=0, ...)

## A minimal example:

The below code snippet demonstrates the least amount of code required to write an executable test: 
<code>
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
</code>