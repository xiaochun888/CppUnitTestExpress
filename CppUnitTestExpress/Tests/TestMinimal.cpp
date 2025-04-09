#include "../CppUnitTestExpress.h"

//The minimal test
struct TestMinimal : public Unit<TestMinimal> {
	void Test() {
		_assert(true, "It is true.");
	}
};

