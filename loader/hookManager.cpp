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
#include "ui/webui.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

std::stack<std::string> patchworkStack;
std::mutex patchworkMutex;
static bool initUi = false;

static PLH::x64Detour* plhDecompressFile;
static uint64_t oDecompressFile = Memory::FindSig(Soup::Signatures::SIG_ZIPCPP_DECOMPRESSFILE);
int hkDecompressFile(Soup::ZipIterator* pZipIterator, char* lpReadBuffer, uint32_t bufferSize) {
	//Get the bundle (.jet) file path
	std::filesystem::path bundlePath = pZipIterator->psArchivePath->cpp_str();
	//Get the bundle information
	Soup::ZipEntry* bundleData = pZipIterator->pZipEntry;
	//Get the entry & path for the current file
	std::string fileName = bundleData->GetName().cpp_str();
	int ret = PLH::FnCast(oDecompressFile, hkDecompressFile)(pZipIterator, lpReadBuffer, bufferSize);
	Logger::Print("Decompressed {}", fileName);

	char file_header[] = "%BIN_2.0";
	if (bufferSize < 8 || memcmp(file_header, lpReadBuffer, sizeof(file_header) - 1) != 0)
	{
		//File isn't BIN2 encrypted
		Logger::Print("{} is not BIN2 encrypted", fileName);
	}
	else {
		Dumper::DumpToDisk(fileName, bundlePath, std::string(lpReadBuffer, bufferSize));
		patchworkMutex.lock();
		patchworkStack.push(fileName);
		patchworkMutex.unlock();
	}

	return ret;
}

static PLH::x64Detour* plhDecryptBytes;
static uint64_t oDecryptBytes = Memory::FindSig(Soup::Signatures::SIG_BIN2_DECRYPTBYTES);
void hkDecryptBytes(uint8_t** bytes) {
	PLH::FnCast(oDecryptBytes, hkDecryptBytes)(bytes);

	patchworkMutex.lock();
	//If theres nothing to patch we're done
	if (patchworkStack.empty()) {
		patchworkMutex.unlock();
		return;
	}
	//Get the first file in the stack
	std::string targetFile = patchworkStack.top();
	//Pop it off the stack when the name is stored
	patchworkStack.pop();

	//Print we are patching it
	Logger::Print("Patching {}", targetFile);

	/*Patch the file*/
	std::string fileContent = std::string(*(char**)bytes, (size_t)(bytes[1] - bytes[0]));
	Patchers::PatchData(targetFile, fileContent);

	//Safety checks
	size_t bufferSize = bytes[1] - bytes[0];
	if (bufferSize < fileContent.size()) {
		Logger::Print<Logger::WARNING>("WARNING: There is not enough space allocated to apply this patch!");
	}

	//Place the patched data back into the buffer
	memcpy(bytes[0], fileContent.c_str(), bytes[1] - bytes[0]);

	//Print we finished patching
	Logger::Print("Patched {}", targetFile);

	patchworkMutex.unlock();
}

static HWND hGameWindow;
static WNDPROC oWndProc;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (!oWndProc) {
		return 0;
	}

	RECT r;
	if (GetClientRect(hWnd, &r)) {
		WebUI::SetRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	WebUI::WndProc(hWnd, uMsg, wParam, lParam);

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

static PLH::x64Detour* plhSwapBuffers;
static uint64_t oSwapBuffers;
static HGLRC overlayContext;

bool hkSwapBuffers(HDC hdc, int b) {
	HGLRC originalContext = wglGetCurrentContext();
	if (!initUi) {
		overlayContext = wglCreateContext(hdc);

		WebUI::Init();
		WebUI::InitPlatform();
		WebUI::CreateRenderer();
		WebUI::CreateView("file:///assets/souped.html");

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

	if (!initUi || !WebUI::IsLoaded()) {
		return PLH::FnCast(oSwapBuffers, hkSwapBuffers)(hdc, b);
	}

	wglMakeCurrent(hdc, overlayContext);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	WebUI::RenderOneFrame();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	wglMakeCurrent(hdc, originalContext);

	return PLH::FnCast(oSwapBuffers, hkSwapBuffers)(hdc, b);
}

bool HookManager::ApplyHooks()
{
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

    return true;
}