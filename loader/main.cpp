#include <logger.h>
#include <memory.h>
#include <Windows.h>
#include "hookManager.h"
#include <polyhook2/Detour/x64Detour.hpp>

static PLH::x64Detour* plhwWinMain;
static uint64_t owWinMain;
int __stdcall hkwWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	void* nShowCmd
) {
	/*Game's main, do hooking/patching/etc here*/
	Logger::Print("Creating hooks...");
	if (HookManager::ApplyHooks()) {
		Logger::Print("Successfully applied all hooks");
	}
	else {
		Logger::Print<Logger::FAILURE>("Failed to apply hooks");
	}

	Logger::Print("Hooks ready");
	std::cin.get();

	return PLH::FnCast(owWinMain, hkwWinMain)(
		hInstance,
		hPrevInstance,
		lpCmdLine,
		nShowCmd);
}

int initialize() {
	AllocConsole();
	SetConsoleTitleA("Console");
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	Logger::Print("Welcome to SoupedModFramework");

	Logger::Print("Hooking WinMain...");
	uint64_t pwWinMain = Memory::FindSig("?? 89 ?? ?? ?? 55 56 57 41 ?? 41 ?? 41 ?? 41 ?? 48 ?? ?? ?? ?? ?? ?? ?? 48 81 ?? ?? ?? ?? ?? 48 8B ?? ?? ?? ?? ?? 48 33 ?? ?? 89 ?? ?? ?? ?? ?? 4D 8B ?? ?? 89 ?? ?? 4D");
	plhwWinMain = new PLH::x64Detour((uint64_t)pwWinMain, (uint64_t)hkwWinMain, &owWinMain);
	if (!plhwWinMain->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook main");
		return false;
	}
	Logger::Print("WinMain hooked");
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