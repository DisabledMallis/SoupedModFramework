#include <logger.h>
#include <memory.h>
#include <Windows.h>
#include "hookManager.h"
#include <polyhook2/Detour/x64Detour.hpp>
#include <ipc.h>
#include "patcher/patchers/BattleMenuPatcher.h"
#include "patcher/overrides.h"
#include <stdjs.h>
#include <config.h>
#include <ModFS.h>
#include "ui/ui.h"
#include "modRegistry.h"

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
	std::filesystem::rename("./Winhttp.dll", "./proxies/Winhttp.dll");

	//Load config
	Config::GetConfig();
	Logger::Print("Config loaded");

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

	Logger::Print("Loading internal patchers...");
	BattleMenuPatcher* battleMenuPatcher = new BattleMenuPatcher();
	Patchers::RegisterPatcher(battleMenuPatcher);
	Logger::Print("Internal patchers loaded");

	//Set up basic JS funcs
	JSUtils::OnInitialize([]() {
		JSUtils::JsValue& global = JSUtils::GetGlobalObject();
		Logger::Print("Global obj ref: {}", (void*)global.internalRef);
		JSUtils::JsValue console = JSUtils::JsValue("console", true);

		JSUtils::JsValue print = JSUtils::JsValue((JsNativeFunction)&StdJs::print<Logger::DEFAULT>);
		JSUtils::JsValue info = JSUtils::JsValue((JsNativeFunction)&StdJs::info);
		JSUtils::JsValue log = JSUtils::JsValue((JsNativeFunction)&StdJs::log);
		JSUtils::JsValue warn = JSUtils::JsValue((JsNativeFunction)&StdJs::warn);
		JSUtils::JsValue error = JSUtils::JsValue((JsNativeFunction)&StdJs::error);
		console.SetProperty("print", print);
		console.SetProperty("info", info);
		console.SetProperty("log", log);
		console.SetProperty("warn", warn);
		console.SetProperty("error", error);

		global.SetProperty("console", console);

		Logger::Print("Added missing default JS functions");
	});
	//Set up souped api
	JSUtils::OnInitialize([]() {
		JSUtils::JsValue& global = JSUtils::GetGlobalObject();
		JSUtils::JsValue souped = JSUtils::JsValue("souped", true);

		JSUtils::JsValue modfs = JSUtils::JsValue("mfs", true);
		JSUtils::JsValue readFile = JSUtils::JsValue((JsNativeFunction)ModRegistry::JS::readFile);
		modfs.SetProperty("readFile", readFile);
		souped.SetProperty("mfs", modfs);

		JSUtils::JsValue registerPatcher = JSUtils::JsValue((JsNativeFunction)Patchers::registerPatcher);
		JSUtils::JsValue registerOverride = JSUtils::JsValue((JsNativeFunction)Overrides::registerOverride);
		JSUtils::JsValue notify = JSUtils::JsValue((JsNativeFunction)UI::JsNotify);
		souped.SetProperty("registerPatcher", registerPatcher);
		souped.SetProperty("registerOverride", registerOverride);
		souped.SetProperty("notify", notify);
		global.SetProperty("souped", souped);

		Logger::Print("souped API ready");

		//Run the souped.js file
		JSUtils::RunFile("./scripts/souped.js");
	});
	//Create the runtime & do all init work
	JSUtils::SetupRuntime();

	//Load all mods
	ModRegistry::LoadAllMods(cd);
	std::vector<ModRegistry::Mod*> allMods = ModRegistry::GetMods();
	for (auto mod : allMods) {
		mod->Run();
	}

	return PLH::FnCast(owWinMain, hkwWinMain)(
		hInstance,
		hPrevInstance,
		lpCmdLine,
		nShowCmd);
}

static PLH::x64Detour* plhFreeConsole = nullptr;
static uint64_t oFreeConsole = (uint64_t)FreeConsole;
bool hkFreeConsole() {
	Logger::Debug("The game tried to free the console window");
	return false;
}

int initialize() {
	SetConsoleTitleA("Console");
	
	Logger::Print("Welcome to SoupedModFramework");

	//Get winmain func
	
	//Patch out FreeConsole
	uint64_t pCreateWindow = Memory::FindSig("?? 89 ?? ?? ?? ?? 89 ?? ?? ?? ?? 89 ?? ?? ?? 55 41 ?? 41 ?? 41 ?? 41 ?? 48 ?? ?? ?? ?? ?? ?? ?? 48 81 ?? ?? ?? ?? ?? 48 8B ?? ?? ?? ?? ?? 48 33 ?? ?? 89 ?? ?? ?? ?? ?? 4C 8B ?? 4C 63");
	Logger::Debug("Patching WinMain...");
	uint8_t* pFreeConCall = (uint8_t*)(pCreateWindow + 0x5C);
	uint64_t callSize = 6;
	DWORD oldProt;
	VirtualProtect((LPVOID)pFreeConCall, callSize, PAGE_EXECUTE_READWRITE, &oldProt);
	for (int i = 0; i < callSize; i++) {
		pFreeConCall[i] = 0x90;
	}
	Logger::Debug("WinMain Patched @ {}", (void*)pFreeConCall);

	Logger::Debug("Hooking WinMain...");
	uint64_t pwWinMain = Memory::FindSig("?? 89 ?? ?? ?? 55 56 57 41 ?? 41 ?? 41 ?? 41 ?? 48 ?? ?? ?? ?? ?? ?? ?? 48 81 ?? ?? ?? ?? ?? 48 8B ?? ?? ?? ?? ?? 48 33 ?? ?? 89 ?? ?? ?? ?? ?? 4D 8B ?? ?? 89 ?? ?? 4D");
	plhwWinMain = new PLH::x64Detour((uint64_t)pwWinMain, (uint64_t)hkwWinMain, &owWinMain);
	if (!plhwWinMain->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook main");
		return false;
	}
	Logger::Debug("WinMain hooked");

	
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