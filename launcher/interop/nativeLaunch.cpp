#include <logger.h>
#include "nativeLaunch.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/color.h>
#include <thread>
#include <fstream>
#include <steam.h>

std::thread launchThread;

bool procHasModule(const PROCESS_INFORMATION& processInfo, std::filesystem::path fullModule) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processInfo.dwProcessId);
    if (snapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 moduleInfo = { 0 };
        moduleInfo.dwSize = sizeof(MODULEENTRY32);

        BOOL ok = Module32First(snapshot, &moduleInfo);
        if (!ok)
        {
            // The read failed, handle the error here.
        }
        do
        {
            HANDLE hFile = CreateFileA(moduleInfo.szExePath,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (hFile)
            {
                char realPath[MAX_PATH];
                DWORD result = GetFinalPathNameByHandleA(hFile,
                    realPath,
                    MAX_PATH,
                    FILE_NAME_NORMALIZED);
                if (result > 0)
                {
                    std::filesystem::path proxyDll = fullModule;
                    if (proxyDll == realPath) {
                        return true;
                    }
                }

                CloseHandle(hFile);
            }
        } while (Module32Next(snapshot, &moduleInfo));

        CloseHandle(snapshot);
    }
    return false;
}

JSValueRef NativeLaunch(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception) {
    std::string userId = Steam::GetUserID();
    if (userId == "0") {
        Logger::Print<Logger::WARNING>("No steam account is logged in!");
        MessageBoxA(0, "Please login to Steam before launching!", "Launcher Error", MB_OK);
        return JSValueMakeBoolean(ctx, false);
    }
    
    //Launch procedure
    launchThread = std::thread([]() {

        //1. Copy proxy dll into game dir
        if (!std::filesystem::exists("./proxies/wininet.dll")) {
            fmt::print(fmt::fg(fmt::color::red), "Couldn't find proxy dll, is SMF installed properly?");
            MessageBoxA(0, "Couldn't find proxy dll", "Launcher Error", MB_OK);
        }
        if (!std::filesystem::exists("./wininet.dll")) {
            try {
                std::filesystem::copy_file("./proxies/wininet.dll", "./wininet.dll");
            }
            catch (std::filesystem::filesystem_error err) {
                fmt::print(fmt::fg(fmt::color::red), "Error copying proxy: {}", err.what());
                MessageBoxA(0, err.what(), "Launcher Error", MB_OK);
            }
        }

        //2. Create modded.lock file for proxy (this tells the proxy dll to load mods or not)
        std::fstream fileStream;
        fileStream.open("modded.lock", std::ios::out);
        fileStream << "Modded client";
        fileStream.close();

        //3. Launch exe
        STARTUPINFO info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;
        if (!CreateProcessA("btdb2_game.exe", 0, 0, 0, FALSE, 0, 0, 0, &info, &processInfo)) {
            Logger::Print("Couldn't find game executable, is SMF installed properly?");
            MessageBoxA(0, "SoupedModFramework cannot find BTDB2, please ensure it is in the game's directory!", "SMF Error", MB_OK);
        }

        //Steam takes some time to launch, so lets wait. Its possible we load when steam is loaded and not when the game is running.
        Sleep(5 * 1000);

        //4. Delete modded lock
        WaitForSingleObject(processInfo.hProcess, 5 * 1000);
        if (!std::filesystem::remove("modded.lock")) {
            Logger::Print("Failed to delete modded.lock, trying again in 1000ms");
            Sleep(1000);
        }

        //5. We're done!
        exit(0);
    });
    launchThread.detach();

	return JSValueMakeBoolean(ctx, true);
}