#include "../CppUnitTestExpress.h"

class TestStringComparaison : public Unit<TestStringComparaison> {
public:
	virtual void Test() {
		const char* const_chars = "";
		char const* chars_const = "";
		char* chars = "";
		char char_arr[10] = "";
		char char_arr2[20] = "";

		std::string std_str = "";
		wchar_t* wcs_str = L"";
		const wchar_t* const_wcs_str = L"";
		wchar_t const* wcs_str_const = L"";

		//Value Comparaison 
		UnitTest::_assert(const_chars == "", "const char*");
		UnitTest::_assert(chars_const == "", "char const *");
		UnitTest::_assert(chars == "", "char *");
		UnitTest::_assert(strcmp(char_arr, "") == 0, "char []");

		UnitTest::_assert(std_str == "", "std::string");
		UnitTest::_assert(wcs_str == L"", "wchar_t *");
		UnitTest::_assert(const_wcs_str == L"", "const wchar_t *");
		UnitTest::_assert(wcs_str_const == L"", "wchar_t const *");

		char_arr[0] = '1';
		char_arr[1] = '1';
		char_arr[2] = ':';
		char_arr[3] = '2';
		char_arr[4] = '0';
		char_arr[5] = '\0';

		char_arr2[0] = '1';
		char_arr2[1] = '1';
		char_arr2[2] = ':';
		char_arr2[3] = '2';
		char_arr2[4] = '0';
		char_arr2[5] = '\0';

		UnitTest::_assert(strcmp(char_arr, "11:20") == 0, "char [10] is equal to 11:20");
		UnitTest::_assert(strcmp(char_arr2, char_arr) == 0, "char [20] is equal to char [10]");
		UnitTest::_assert(strcmp(char_arr, "11:21") <= 0, "char_arr <= 11:21");
		UnitTest::_assert("11:21" > "11:20", "11:21 > 11:20");
		UnitTest::_assert("11:21" > "11:20", "11:21 > 11:20");
		UnitTest::_assert("11:20" < "11:21", "11:20 < 11:21");
		UnitTest::_assert("11:21" <= "11:21", "11:21 <= 11:21");
		UnitTest::_assert("11:21" == "11:21", "11:21 >= 11:21");
	}
};

class TestAssertExpression : public Unit<TestAssertExpression> {
	void Test() {
		float ft = 1.0;
		int it = 1;
		unsigned ut = 1;
		char char_arr[10] = { "11:20" };
		char char_arr2[20] = { "11:20" };

		UnitTest::_assert(ft == it, "ft is equal to it");

		UnitTest::_assert(ft == it, "ft == it");
		UnitTest::_assert(ft == ut, "ft == ut");

		wchar_t const* wcs_str_const = L"test";
		UnitTest::_assert(wcs_str_const == L"test", "wchar_t const *");
		UnitTest::_assert(wcs_str_const == L"test", "wchar_t const *");

		const char* const_chars = "test";
		UnitTest::_assert(const_chars == "test", "const_chars == test");

		UnitTest::_assert(char_arr != "11:20", "char [] != char *");
		UnitTest::_assert(char_arr != char_arr2, "char [10] != char [20]");
		UnitTest::_assert(char_arr2 < char_arr, "char [20] < char [10]");
		UnitTest::_assert(char_arr == char_arr, "char [10] == char [10]");
		UnitTest::_assert("11:35" <= "12:00", "11:35 < 12:00");
		UnitTest::_assert("11:35" < "12:00", "11:35 < 12:00");

		std::string str = "11:35";
		_assert(str < "12:00", "11:35 < 12:00");
		_assert(str > "11:00", "11:35 > 11:00");
	}
};

class TestVariadicArgument : public Unit<TestVariadicArgument> {
	void Test() {
		char char_arrary[20] = { "test" };
		std::string std_string = char_arrary;
		const char* const_chars = std_string.c_str();

		dprintf("SUCCESS:%d, %s, %s, %s\n", SUCCESS, const_chars, char_arrary, std_string);

		_assert(0 == strcmp(c_arg(std_string), std_string.c_str()), "Should be same: %s == %s", std_string, std_string.c_str());
		_assert(0 == strcmp(c_arg(const_chars), char_arrary), "Should be same");
		_assert(c_arg(1) == 1, "Should be equal");
		_assert(c_arg(1.1) == 1.1, "Should be equal");
		_assert(c_arg(SUCCESS) == SUCCESS, "Should be equal");
	}
};

class TestIgnoreTest : public Unit<TestIgnoreTest> {
	void Test() {
		throw this;
	}
};