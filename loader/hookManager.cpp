#include "hookManager.h"

#include <logger.h>
#include <memory.h>
#include <steam.h>
#include <ZipCpp.h>
#include <Bin2.h>
#include "dumper/dumper.h"
#include <patchers.h>

#include <Windows.h>
#include <stack>
#include <polyhook2/Detour/x64Detour.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Profile.h>

std::stack<std::string> patchworkStack;
std::mutex patchworkMutex;
static bool initUi = false;

static PLH::x64Detour* plhDecompressFile;
static uint64_t oDecompressFile = Memory::FindSig(Soup::Signatures::SIG_ZIPCPP_DECOMPRESSFILE);
void* hkDecompressFile(Soup::ZipIterator* pZipIterator, char* lpReadBuffer, uint32_t bufferSize) {
	//Get the bundle (.jet) file path
	std::filesystem::path bundlePath = pZipIterator->psArchivePath->cpp_str();
	//Get the bundle information
	Soup::ZipEntry* bundleData = pZipIterator->pZipEntry;
	//Get the entry & path for the current file
	std::string fileName = bundleData->GetName().cpp_str();
	void* ret = PLH::FnCast(oDecompressFile, hkDecompressFile)(pZipIterator, lpReadBuffer, bufferSize);
	Logger::Debug("Decompressed {}", fileName);

	static char bin2_header[] = "%BIN_2.0";
	static char jpng_header[] = "%JPNG001";
	if (memcmp(bin2_header, lpReadBuffer, sizeof(bin2_header) - 1) == 0)
	{
		Logger::Debug("{} IS BIN2 encrypted", fileName);
		Dumper::DumpToDisk(fileName, bundlePath, std::string(lpReadBuffer, bufferSize));
		patchworkMutex.lock();
		patchworkStack.push(fileName);
		patchworkMutex.unlock();
		return ret;
	}
	if (memcmp(jpng_header, lpReadBuffer, sizeof(jpng_header) - 1) == 0)
	{
		Logger::Debug("{} IS JPNG format", fileName);
		Dumper::DumpToDisk(fileName, bundlePath, std::string(lpReadBuffer, bufferSize));
		return ret;
	}
	Logger::Debug("{} is not BIN2 nor JPNG format", fileName);
	return ret;
}

static PLH::x64Detour* plhDecryptBytes;
static uint64_t oDecryptBytes = Memory::FindSig(Soup::Signatures::SIG_BIN2_DECRYPTBYTES);
void* hkDecryptBytes(uint8_t** bytes) {
	void* result = PLH::FnCast(oDecryptBytes, hkDecryptBytes)(bytes);

	patchworkMutex.lock();
	//If theres nothing to patch we're done
	if (patchworkStack.empty()) {
		patchworkMutex.unlock();
		return result;
	}
	//Get the first file in the stack
	std::string targetFile = patchworkStack.top();
	//Pop it off the stack when the name is stored
	patchworkStack.pop();

	//Print we are patching it
	Logger::Debug("Patching {}", targetFile);

	/*Patch the file*/
	std::string fileContent = std::string(*(char**)bytes, (size_t)(bytes[1] - bytes[0]));
	Patchers::PatchData(targetFile, fileContent);

	//Safety checks
	size_t bufferSize = bytes[1] - bytes[0];
	if (bufferSize < fileContent.size()) {
		Logger::Debug("WARNING: There is not enough space allocated to apply this patch!");
	}

	//Place the patched data back into the buffer
	memset(bytes[0], 0, bufferSize);
	memcpy_s(bytes[0], bufferSize, fileContent.c_str(), fileContent.size());

	//Print we finished patching
	Logger::Debug("Patched {}", targetFile);

	patchworkMutex.unlock();
	return result;
}

static HWND hGameWindow;
static WNDPROC oWndProc;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (!oWndProc) {
		return 0;
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

static PLH::x64Detour* plhSwapBuffers;
static uint64_t oSwapBuffers;
static HGLRC overlayContext;
bool hkSwapBuffers(HDC hdc, int b) {
	HGLRC originalContext = wglGetCurrentContext();
	if (!initUi) {
		overlayContext = wglCreateContext(hdc);

		if (glfwInit() != GLFW_TRUE) {
			MessageBoxA(0, "SoupedModFramework couldn't initialize GLFW, and as a result it must exit", "GLFW Error", MB_OK);
			exit(0);
		}
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		gladLoadGL();

		hGameWindow = WindowFromDC(hdc);
		oWndProc = (WNDPROC)SetWindowLongPtr(hGameWindow, GWLP_WNDPROC, (__int3264)(LONG_PTR)hkWndProc);

		// Init glew, create imgui context, init imgui
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hGameWindow);
		ImGui_ImplOpenGL3_Init();
		ImGui::StyleColorsDark();
		ImGui::CaptureMouseFromApp();

		initUi = true;
	}

	if (!initUi) {
		return PLH::FnCast(oSwapBuffers, hkSwapBuffers)(hdc, b);
	}

	wglMakeCurrent(hdc, overlayContext);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	wglMakeCurrent(hdc, originalContext);

	return PLH::FnCast(oSwapBuffers, hkSwapBuffers)(hdc, b);
}

static PLH::x64Detour* plhIsTowerUnlocked = nullptr;
static uint64_t oIsTowerUnlocked = Memory::FindSig(Soup::Signatures::SIG_PROFILE_ISTOWERUNLOCKED);
bool hkIsTowerUnlocked(size_t param_1, int param_2) {
	return true;
}

static PLH::x64Detour* plhIsUpgradeUnlocked = nullptr;
static uint64_t oIsUpgradeUnlocked = Memory::FindSig(Soup::Signatures::SIG_PROFILE_ISUPGRADEUNLOCKED);;
bool hkIsUpgradeUnlocked(size_t param_1, size_t param_2, int param_3, int param_4) {
	return true;
}

bool HookManager::ApplyHooks()
{
	Config* config = Config::GetConfig();

	plhDecompressFile = new PLH::x64Detour(oDecompressFile, (uint64_t)hkDecompressFile, &oDecompressFile);
	if (!plhDecompressFile->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook ZipCpp::DecompressFile");
		return false;
	}

	plhDecryptBytes = new PLH::x64Detour(oDecryptBytes, (uint64_t)hkDecryptBytes, &oDecryptBytes);
	if (!plhDecryptBytes->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook Bin2::DecryptBytes");
		return false;
	}

	HMODULE hOpenGL = GetModuleHandleA("OPENGL32.dll");
	if (hOpenGL) {
		FARPROC pSwapBuffers = GetProcAddress(hOpenGL, "wglSwapBuffers");
		if (pSwapBuffers) {
			plhSwapBuffers = new PLH::x64Detour((uint64_t)pSwapBuffers, (uint64_t)hkSwapBuffers, &oSwapBuffers);
			if (!plhSwapBuffers->hook()) {
				Logger::Print<Logger::FAILURE>("Failed to hook wglSwapBuffers");
				return false;
			}
		}
		else {
			Logger::Print<Logger::FAILURE>("Couldn't find wglSwapBuffers address");
			return false;
		}
	}
	else {
		Logger::Print<Logger::FAILURE>("OpenGL was not found, but the game relies on it? Couldn't hook wglSwapBuffers");
		return false;
	}

	if (config->UnlockTowers()) {
		plhIsTowerUnlocked = new PLH::x64Detour(oIsTowerUnlocked, (uint64_t)hkIsTowerUnlocked, &oIsTowerUnlocked);
		if (!plhIsTowerUnlocked->hook()) {
			Logger::Print<Logger::FAILURE>("Failed to hook Profile::IsTowerUnlocked");
			return false;
		}
	}

	if (config->UnlockUpgrades()) {
		plhIsUpgradeUnlocked = new PLH::x64Detour(oIsUpgradeUnlocked, (uint64_t)hkIsUpgradeUnlocked, &oIsUpgradeUnlocked);
		if (!plhIsUpgradeUnlocked->hook()) {
			Logger::Print<Logger::FAILURE>("Failed to hook Profile::IsUpgradeUnlocked");
			return false;
		}
	}

    return true;
}