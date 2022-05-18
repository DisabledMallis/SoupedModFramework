#include <logger.h>
#include <memory.h>
#include <Windows.h>
#include "hookManager.h"
#include <polyhook2/Detour/x64Detour.hpp>
#include <ipc.h>
#include <SoupSTL.h>
#include "patcher/patchers/BattleMenuPatcher.h"
#include <stdjs.h>

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

	Logger::Print("Killing launcher");
	int retval = system("taskkill /F /T /IM launcher.exe");
	Logger::Print("Launcher killed & pipe closed");

	//Set up basic JS funcs
	JSUtils::OnInitialize([]() {
		JSUtils::JsValue& global = JSUtils::GetGlobalObject();
		global["console"] = JSUtils::JsValue("console", true);

		global["console"]["print"] = JSUtils::JsValue((JsNativeFunction)StdJs::print<Logger::DEFAULT>);
		global["console"]["info"] = JSUtils::JsValue((JsNativeFunction)StdJs::info);
		global["console"]["log"] = JSUtils::JsValue((JsNativeFunction)StdJs::log);
		global["console"]["warn"] = JSUtils::JsValue((JsNativeFunction)StdJs::warn);
		global["console"]["error"] = JSUtils::JsValue((JsNativeFunction)StdJs::error);

		Logger::Print("Added missing default JS functions");
	});
	//Set up souped api
	JSUtils::OnInitialize([]() {
		JSUtils::JsValue souped("souped", true);
		JSUtils::JsValue jsRegisterPatcher("registerPatcher", (JsNativeFunction)Patchers::registerPatcher);
		souped["registerPatcher"] = jsRegisterPatcher;
		Logger::Print("souped API ready");

		//Run the souped.js file
		JSUtils::RunFile("./scripts/souped.js");
	});
	//Create the runtime & do all init work
	JSUtils::SetupRuntime();

	BattleMenuPatcher* battleMenuPatcher = new BattleMenuPatcher();
	Patchers::RegisterPatcher(battleMenuPatcher);

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