// CppUnitTestExpress.cpp�: d�finit le point d'entr�e pour l'application console.
//

#include "stdafx.h"
#include "CppUnitTestExpress.h"

int _tmain(int argc, _TCHAR* argv[])
{
	UnitTest ut;
	ut.runAll("Test*");
	return ut.resume();
}

