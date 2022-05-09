#include <logger.h>
#include <memory.h>
#include <Windows.h>
#include "hookManager.h"
#include <polyhook2/Detour/x64Detour.hpp>
#include <ipc.h>
#include <SoupSTL.h>
#include <jswrapper.h>

static PLH::x64Detour* plhwWinMain;
static uint64_t owWinMain;
int __stdcall hkwWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	void* nShowCmd
) {
	//Move proxy dll back into 'proxies' folder
	std::filesystem::path cd = std::filesystem::current_path();
	std::filesystem::rename("./wininet.dll", "./proxies/wininet.dll");

	/*Game's main, do hooking/patching/etc here*/
	Logger::Print("Creating hooks...");
	if (HookManager::ApplyHooks()) {
		Logger::Print("Successfully applied all hooks");
	}
	else {
		Logger::Print<Logger::FAILURE>("Failed to apply hooks");
	}

	Logger::Print("Hooks ready");

	Logger::Print("Loading ChakraCore...");
	JSWrapper::InitializeRuntime();
	Logger::Print("ChakraCore ready");
	Logger::Print("Loading SMF API...");
	JSWrapper::InitializeAPI();
	Logger::Print("SMF API ready");

	Logger::Print("Killing launcher");
	HANDLE hPipe = IPC::OpenPipe(LAUNCH_STATUS_PIPE);
	Logger::Print("Pipe open");
	IPC::WriteMessage(hPipe, "exit");
	Logger::Print("Message sent");
	IPC::ClosePipe(hPipe);
	Logger::Print("Launcher killed & pipe closed");

	return PLH::FnCast(owWinMain, hkwWinMain)(
		hInstance,
		hPrevInstance,
		lpCmdLine,
		nShowCmd);
}

int initialize() {
	SetConsoleTitleA("Console");
	
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