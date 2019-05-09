// dbgext.cpp : Defines the entry point for the application.
//

#include "dbgext.h"

using namespace std;

__declspec(dllexport) int mymain()
{
	void* pDebugClient;
	DebugCreate(__uuidof(IDebugClient), (void**)&pDebugClient);
	cout << "Hello CMake." << endl;
	return 0;
}
