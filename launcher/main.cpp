#include "launcher.h"
#include <Windows.h>

int main() {
	AllocConsole();
	SetConsoleTitleA("Launcher");
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	Launcher app;
	app.Run();
	return 0;
}