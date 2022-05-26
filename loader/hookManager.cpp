#include "hookManager.h"

#include <logger.h>
#include <memory.h>
#include <steam.h>
#include <ZipCpp.h>
#include <Bin2.h>
#include "dumper/dumper.h"
#include "patcher/patchers.h"
#include "patcher/overrides.h"

#include <Windows.h>
#include <stack>
#include <polyhook2/Detour/x64Detour.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Profile.h>
#include "ui/ui.h"
#include <ShlObj_core.h>
#include <asmjit/asmjit.h>

std::mutex patchworkMutex;
std::stack<std::array<std::string, 2>> patchworkStack;
static bool initUi = false;
static auto fnResizeBuffer = (void(__fastcall*)(void**, long long))Memory::FindSig(Soup::Signatures::SIG_BIN2_RESIZEBUFFER);

/*
struct ParseHook {
	PLH::x64Detour* detour;
	uint64_t oFunction;
	void* parseResult;
};
static std::vector<ParseHook> parseHooks;
void* hkParseAsset(void* parseResult, Soup::ArrayList<uint8_t>* bytes) {
	Logger::Debug("Beginning patchwork");
	goto beginHook;
exitHook:
	for (ParseHook& parseHook : parseHooks) {
		if (parseHook.parseResult == parseResult) {
			return PLH::FnCast(parseHook.oFunction, hkParseAsset)(parseResult, bytes);
		}
	}

beginHook:
	patchworkMutex.lock();
	//If theres nothing to patch we're done
	if (patchworkStack.empty()) {
		patchworkMutex.unlock();
		goto exitHook;
	}
	//Get the first file in the stack
	std::array<std::string, 2> targetInfo = patchworkStack.top();
	std::string targetBundle = targetInfo[0];
	std::string targetFile = targetInfo[1];
	//Pop it off the stack when the name is stored
	patchworkStack.pop();

	Logger::Debug("Patching {}", targetFile);

	//Patch the file
	std::string fileContent = std::string((char*)bytes->at(0), bytes->count());
	Overrides::RunOverrides(targetBundle, targetFile, fileContent, false);
	Patchers::PatchData(targetBundle, targetFile, fileContent);
	Overrides::RunOverrides(targetBundle, targetFile, fileContent, true);

	//Safety checks
	size_t bufferSize = bytes->count();
	if (bufferSize < fileContent.size()) {
		Logger::Debug("WARNING: There is not enough space allocated to apply this patch!");
		fnResizeBuffer((void**)bytes, fileContent.size());
	}

	//Place the patched data back into the buffer
	memset(bytes->begin(), 0, bytes->count());
	memcpy_s(bytes->begin(), bytes->count(), fileContent.c_str(), fileContent.size());

	//Print we finished patching
	Logger::Debug("Patched {}", targetFile);

	patchworkMutex.unlock();
	goto exitHook;
}

static PLH::x64Detour* plhLoadAsset;
static uint64_t oLoadAsset = Memory::FindSig(Soup::Signatures::SIG_LOAD_ASSET);
static std::vector<uint64_t> patchedParsers;
void hkLoadAsset(int* param_1, Soup::WinFileIO* pWinFileIO, Soup::String* type, uint64_t parser,
	void* parseResult, Soup::String* pathInArchive, void* param_7, char isEncrypted) {
	bool isPatched = false;
	for (uint64_t pParser : patchedParsers) {
		if (parser == pParser) {
			isPatched = true;
		}
	}
	if (!isPatched) {
		uint64_t pfParseAsset = *(uint64_t*)(*(uint64_t*)parser + 0x10);
		uint64_t oParseAsset = 0;
		PLH::x64Detour* plhLoader = new PLH::x64Detour(pfParseAsset, (uint64_t)hkParseAsset, &oParseAsset);
		if (!plhLoader->hook()) {
			Logger::Debug("Failed to hook LoadAsset parser");
		}
		else {
			parseHooks.emplace_back(plhLoader, oParseAsset, parseResult);
		}
	}

	Logger::Debug("Patching {}", pathInArchive->cpp_str());
	patchworkMutex.lock();
	patchworkStack.push({ "", pathInArchive->cpp_str()});
	patchworkMutex.unlock();

	PLH::FnCast(oLoadAsset, hkLoadAsset)(param_1, pWinFileIO, type, parser, parseResult, pathInArchive, param_7, isEncrypted);
}
*/

//48 8b c1 49 2b c0 48 8d 50 ?? 48 3b d0 73 ?? 4a 8d 04 02

static PLH::x64Detour* plhPostDecrypt;
static uint64_t oPostDecrypt = Memory::FindSig("48 8b c1 49 2b c0 48 8d 50 ?? 48 3b d0 73 ?? 4a 8d 04 02");
void PostDecrypt(char* endPtr, size_t size, char* beginPtr) {
	patchworkMutex.lock();
	//If theres nothing to patch we're done
	if (patchworkStack.empty()) {
		patchworkMutex.unlock();
		return;
	}
	//Get the first file in the stack
	std::array<std::string, 2> targetInfo = patchworkStack.top();
	std::string targetBundle = targetInfo[0];
	std::string targetFile = targetInfo[1];
	//Pop it off the stack when the name is stored
	patchworkStack.pop();

	//Print we are patching it
	Logger::Debug("Patching {}", targetFile);

	//Patch the file
	std::string fileContent = std::string(beginPtr, size);
	Overrides::RunOverrides(targetBundle, targetFile, fileContent, false);
	Patchers::PatchData(targetBundle, targetFile, fileContent);
	Overrides::RunOverrides(targetBundle, targetFile, fileContent, true);

	//Safety checks
	if (size < fileContent.size()) {
		Logger::Debug("WARNING: There is not enough space allocated to apply this patch!");
		fnResizeBuffer((void**)beginPtr, size);
	}

	//Place the patched data back into the buffer
	memset(beginPtr, 0, size);
	memcpy_s(beginPtr, size, fileContent.c_str(), fileContent.size());

	//Print we finished patching
	Logger::Debug("Patched {}", targetFile);

	patchworkMutex.unlock();
	return;
}
void* hkPostDecrypt(char* endPtr, size_t size, char* beginPtr) {
	PostDecrypt(endPtr, size, beginPtr);
	return PLH::FnCast(oPostDecrypt, hkPostDecrypt)(endPtr, size, beginPtr);
}
//Derive key
//?? ?? ?? 71 ?? ?? ?? 33 ?? ?? ?? 0B ?? ?? ?? B5 ?? ?? ?? E5 ?? ?? ?? 23 ?? ?? ?? CF
//(THere are multiple instances)


static PLH::x64Detour* plhDecompressFile;
static uint64_t oDecompressFile = Memory::FindSig(Soup::Signatures::SIG_ZIPCPP_DECOMPRESSFILE);
void* hkDecompressFile(Soup::ZipReader* pZipReader) {
	Soup::ZipIterator* pZipIterator = pZipReader->pZipIterator;
	char* pReadBuffer = (char*)pZipReader->pReadBuffer;
	uint32_t bufferSize = pZipReader->bufferSize;
	//Get the bundle (.jet) file path
	std::filesystem::path bundlePath = pZipIterator->psArchivePath->cpp_str();
	//Get the bundle information
	Soup::ZipEntry* bundleData = pZipIterator->pZipEntry;
	//Get the entry & path for the current file
	std::string fileName = bundleData->GetName().cpp_str();
	void* ret = PLH::FnCast(oDecompressFile, hkDecompressFile)(pZipReader);
	Logger::Debug("Decompressed {}", fileName);

	static char bin2_header[] = "%BIN_2.0";
	static char jpng_header[] = "%JPNG001";
	if (memcmp(bin2_header, pReadBuffer, sizeof(bin2_header) - 1) == 0)
	{
		Logger::Debug("{} IS BIN2 encrypted", fileName);
		Dumper::DumpToDisk(fileName, bundlePath, std::string(pReadBuffer, bufferSize));
		patchworkMutex.lock();
		patchworkStack.push(std::array<std::string, 2>{bundlePath.filename().string(), fileName});
		patchworkMutex.unlock();
		return ret;
	}
	if (memcmp(jpng_header, pReadBuffer, sizeof(jpng_header) - 1) == 0)
	{
		Logger::Debug("{} IS JPNG format", fileName);
		Dumper::DumpToDisk(fileName, bundlePath, std::string(pReadBuffer, bufferSize));
		return ret;
	}
	Logger::Debug("{} is not BIN2 nor JPNG format", fileName);
	return ret;
}

static PLH::x64Detour* plhDecryptBytes;
//static uint64_t oDecryptBytes = Memory::FindSig(Soup::Signatures::SIG_BIN2_DECRYPTBYTES);
/*void* hkDecryptBytes(Soup::ArrayList<uint8_t>* bytes) {
	void* result = PLH::FnCast(oDecryptBytes, hkDecryptBytes)(bytes);

	patchworkMutex.lock();
	//If theres nothing to patch we're done
	if (patchworkStack.empty()) {
		patchworkMutex.unlock();
		return result;
	}
	//Get the first file in the stack
	std::array<std::string, 2> targetInfo = patchworkStack.top();
	std::string targetBundle = targetInfo[0];
	std::string targetFile = targetInfo[1];
	//Pop it off the stack when the name is stored
	patchworkStack.pop();

	//Print we are patching it
	Logger::Debug("Patching {}", targetFile);

	//Patch the file
	std::string fileContent = std::string((char*)bytes->at(0), bytes->count());
	Overrides::RunOverrides(targetBundle, targetFile, fileContent, false);
	Patchers::PatchData(targetBundle, targetFile, fileContent);
	Overrides::RunOverrides(targetBundle, targetFile, fileContent, true);

	//Safety checks
	size_t bufferSize = bytes->count();
	if (bufferSize < fileContent.size()) {
		Logger::Debug("WARNING: There is not enough space allocated to apply this patch!");
		fnResizeBuffer((void**)bytes, fileContent.size());
	}

	//Place the patched data back into the buffer
	memset(bytes->begin(), 0, bytes->count());
	memcpy_s(bytes->begin(), bytes->count(), fileContent.c_str(), fileContent.size());

	//Print we finished patching
	Logger::Debug("Patched {}", targetFile);

	patchworkMutex.unlock();
	return result;
}*/

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

		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		char fontsPath[MAX_PATH];
		SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, NULL, fontsPath);
		std::string arial_path = std::string(std::string(fontsPath) + "/Calibril.ttf");
		io.Fonts->AddFontFromFileTTF(arial_path.c_str(), 16);
		io.Fonts->AddFontFromFileTTF(arial_path.c_str(), 20);
		io.Fonts->Build();

		initUi = true;
	}

	if (!initUi) {
		return PLH::FnCast(oSwapBuffers, hkSwapBuffers)(hdc, b);
	}

	wglMakeCurrent(hdc, overlayContext);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	UI::Render();

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
static uint64_t oIsUpgradeUnlocked = Memory::FindSig(Soup::Signatures::SIG_PROFILE_ISUPGRADEUNLOCKED);
bool hkIsUpgradeUnlocked(size_t param_1, size_t param_2, int param_3, int param_4) {
	return true;
}

typedef void(*AsmFunc)();
bool HookManager::ApplyHooks()
{
	Config* config = Config::GetConfig();
	
	plhDecompressFile = new PLH::x64Detour(oDecompressFile, (uint64_t)hkDecompressFile, &oDecompressFile);
	if (!plhDecompressFile->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook ZipCpp::DecompressFile");
		return false;
	}
	
	using namespace asmjit;
	FileLogger logger(stdout);
	JitRuntime runtime;

	x86::Mem gameExec = x86::ptr(oPostDecrypt);
	x86::Mem gameRet = x86::ptr(oPostDecrypt+6);

	//Bootstrapper
	CodeHolder bsCode;
	bsCode.init(runtime.environment());
	bsCode.setLogger(&logger);
	x86::Assembler bsAsm(&bsCode);
	//Push registers that need to be saved
	bsAsm.push(x86::rcx);
	bsAsm.push(x86::rcx);
	bsAsm.push(x86::rsi);
	bsAsm.push(x86::rdi);
	bsAsm.push(x86::r8 );
	bsAsm.push(x86::r13);
	bsAsm.push(x86::r15);
	//Hook code here (call the callback function)

	//Pop registers for returning to game code
	bsAsm.pop(x86::rcx);
	bsAsm.pop(x86::rcx);
	bsAsm.pop(x86::rsi);
	bsAsm.pop(x86::rdi);
	bsAsm.pop(x86::r8);
	bsAsm.pop(x86::r13);
	bsAsm.pop(x86::r15);

	//Restore to game code
	bsAsm.mov(x86::rax, x86::rcx);
	bsAsm.sub(x86::rax, x86::r8);
	bsAsm.jmp(gameRet);
	//Create the bootstrap func
	AsmFunc bs;
	runtime.add(&bs, &bsCode);
	
	//Detour
	CodeHolder dCode;
	dCode.setLogger(&logger);
	dCode.init(runtime.environment());
	x86::Assembler dAsm(&dCode);
	Label lD = dAsm.newLabel();
	dAsm.jmp(bs);

	//Create detour func (TODO: Write to the game's mem instead)
	AsmFunc det;
	runtime.add(&det, &dCode);
	::Logger::Debug("Bootstrapper generated here: {}", (void*)bs);
	::Logger::Debug("Detour generated here: {}", (void*)det);
	std::cin.get();

	/*
	plhPostDecrypt = new PLH::x64Detour(oPostDecrypt, (uint64_t)hkPostDecrypt, &oPostDecrypt);
	if (!plhPostDecrypt->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook PostDecrypt");
		return false;
	}
	*/

	/*
	plhDecryptBytes = new PLH::x64Detour(oDecryptBytes, (uint64_t)hkDecryptBytes, &oDecryptBytes);
	if (!plhDecryptBytes->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook Bin2::DecryptBytes");
		return false;
	}
	*/

	HMODULE hOpenGL = GetModuleHandleA("OPENGL32.dll");
	if (hOpenGL) {
		FARPROC pSwapBuffers = GetProcAddress(hOpenGL, "wglSwapBuffers");
		if (pSwapBuffers) {
			plhSwapBuffers = new PLH::x64Detour((uint64_t)pSwapBuffers, (uint64_t)hkSwapBuffers, &oSwapBuffers);
			if (!plhSwapBuffers->hook()) {
				::Logger::Print<::Logger::FAILURE>("Failed to hook wglSwapBuffers");
				return false;
			}
		}
		else {
			::Logger::Print<::Logger::FAILURE>("Couldn't find wglSwapBuffers address");
			return false;
		}
	}
	else {
		::Logger::Print<::Logger::FAILURE>("OpenGL was not found, but the game relies on it? Couldn't hook wglSwapBuffers");
		return false;
	}

	if (config->UnlockTowers()) {
		plhIsTowerUnlocked = new PLH::x64Detour(oIsTowerUnlocked, (uint64_t)hkIsTowerUnlocked, &oIsTowerUnlocked);
		if (!plhIsTowerUnlocked->hook()) {
			::Logger::Print<::Logger::FAILURE>("Failed to hook Profile::IsTowerUnlocked");
			return false;
		}
	}

	if (config->UnlockUpgrades()) {
		plhIsUpgradeUnlocked = new PLH::x64Detour(oIsUpgradeUnlocked, (uint64_t)hkIsUpgradeUnlocked, &oIsUpgradeUnlocked);
		if (!plhIsUpgradeUnlocked->hook()) {
			::Logger::Print<::Logger::FAILURE>("Failed to hook Profile::IsUpgradeUnlocked");
			return false;
		}
	}

    return true;
}