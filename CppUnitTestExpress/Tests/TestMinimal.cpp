#include "../CppUnitTestExpress.h"

//The minimal test
struct TestMinimal : Unit<TestMinimal> {
	void Test() {
		_assert(true, "It is true.");
	}
};

