#include "launcher.h"
#include <logger.h>
#include <Windows.h>
#include <filesystem>

int main() {
	AllocConsole();
	SetConsoleTitleA("Launcher");
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	using namespace Logger;

	if (!std::filesystem::exists("mods")) {
		std::filesystem::create_directory("mods");
		Print("No mods directory found, so a new one was made");
	}

	Launcher app;
	app.Run();
	return 0;
}