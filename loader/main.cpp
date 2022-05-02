#include <Windows.h>
#include <logger.h>

int initialize() {
	AllocConsole();
	SetConsoleTitleA("Console");
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	Logger::print("Welcome to SoupedModFramework");

	return 0;
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