// CppUnitTestExpress.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"
#include "CppUnitTestExpress.h"

int _tmain(int argc, _TCHAR* argv[])
{
	UnitTest ut;
	ut.runAll("Test*");
	ut.resume();
	return ut.result();
}

