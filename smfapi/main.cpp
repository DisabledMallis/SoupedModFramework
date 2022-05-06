#include <Windows.h>
#include <jswrapper.h>

void initialize() {
	JSWrapper::InitializeRuntime();
}

extern "C" __declspec(dllexport) bool __stdcall DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason, // reason for calling function
	LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		initialize();
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}