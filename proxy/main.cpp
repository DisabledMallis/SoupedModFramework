/*
Written by DisabledMallis (https://github.com/DisabledMallis)

Originally made for the NKHook5 project, then repurposed for Round8, and finally used in SMF

Usage:
	This DLL is loaded when the game loads. This allows us to run our own code before the game's
	code so we can modify how it loads without patching the binary. This DLL searches for a relative
	directory called "loaders" which will contain any DLLs that should be injected before the game starts.
	These DLLs should be 'tweaker' DLLs that prepare & patch the game for mod loading.
*/
#include <Windows.h>
#include <string>
#include <iostream>
#include <shlobj_core.h>
#include <filesystem>
#include <cstdint>
#include <fmt/core.h>

//Used to ensure the functions required are properly exported
#define EXPORT comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

//HMODULE for the original dll
HMODULE WinHttp;
bool(__stdcall* oWinHttpGetIEProxyConfigForCurrentUser)(void*);

extern "C" __declspec(dllexport) bool __stdcall WinHttpGetIEProxyConfigForCurrentUser(void* proxyConfig) {
	int result = oWinHttpGetIEProxyConfigForCurrentUser(proxyConfig);
	return result;
}
extern "C" __declspec(dllexport) void __stdcall RestoreSelf() {
	//Move proxy dll back into 'proxies' folder
	std::filesystem::path cd = std::filesystem::current_path();
	std::filesystem::rename("./Winhttp.dll", "./proxies/Winhttp.dll");
}

auto initialize(HMODULE proxyDll) -> int {
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	//Find the original wininet.dll
	char sys32Path[MAX_PATH];

	//We need to use diff dirs on diff platforms
#if INTPTR_MAX == INT64_MAX
	SHGetFolderPathA(nullptr, CSIDL_SYSTEM, nullptr, SHGFP_TYPE_CURRENT, sys32Path);
#elif INTPTR_MAX == INT32_MAX
	SHGetFolderPathA(nullptr, CSIDL_SYSTEMX86, nullptr, SHGFP_TYPE_CURRENT, sys32Path);
#else
#error Unknown pointer size or missing size macros!
#endif

	std::string sys32Str(sys32Path);
	std::string winhttpPath = sys32Str + "\\Winhttp.dll";

	//Load the original wininet dll
	WinHttp = LoadLibraryA(winhttpPath.c_str());
	if (WinHttp == NULL) {
		fmt::print("Failed to load original Winhttp\n");
		MessageBoxA(0, "Failed to find Winhttp.dll in System32", "Proxy Error", MB_OK);
		exit(1);
	}
	else {
		fmt::print("Loaded original wininet\n");
	}
	//Get the original function to call when requested
	oWinHttpGetIEProxyConfigForCurrentUser = (bool(__stdcall*)(void*))GetProcAddress(WinHttp, "WinHttpGetIEProxyConfigForCurrentUser");
	if (oWinHttpGetIEProxyConfigForCurrentUser == NULL) {
		fmt::print("Failed to find WinHttpGetIEProxyConfigForCurrentUser\n");
		MessageBoxA(0, "Failed to find WinHttpGetIEProxyConfigForCurrentUser", "Proxy Error", MB_OK);
		exit(1);
	}

	//Load tweaker DLLs
	try {
		std::string modsDir = "loaders/";
		for (const auto& tweaker : std::filesystem::directory_iterator(modsDir)) {
			LoadLibraryA(tweaker.path().string().c_str());
			fmt::print("Loaded tweaker '{}'\n", tweaker.path().filename().string());
		}
	}
	catch (std::exception ex) {
		MessageBoxA(nullptr, ex.what(), "Error (Missing 'loaders' folder?)", MB_OK);
		exit(1);
	}

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
		initialize(hinstDLL);
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}