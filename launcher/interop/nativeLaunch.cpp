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
#include <ipc.h>

std::thread launchThread;

JSValueRef NativeLaunch(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception) {
    std::string userId = Steam::GetUserID();
    if (userId == "0") {
        Logger::Print<Logger::WARNING>("No steam account is logged in!");
        MessageBoxA(0, "Please login to Steam before launching!", "Launcher Error", MB_OK);
        return JSValueMakeBoolean(ctx, false);
    }
    
    //Launch procedure
    launchThread = std::thread([]() {
        //1. Move proxy dll into game dir
        if (!std::filesystem::exists("./proxies/wininet.dll")) {
            Logger::Print<Logger::FAILURE>("Couldn't find proxy dll, is SMF installed properly?");
            MessageBoxA(0, "Couldn't find proxy dll", "Launcher Error", MB_OK);
            return;
        }
        if (std::filesystem::exists("./wininet.dll")) {
            if (!std::filesystem::remove("./wininet.dll")) {
                Logger::Print<Logger::WARNING>("Couldn't remove pre-existing wininet.dll");
            }
        }
        try {
            std::filesystem::rename("./proxies/wininet.dll", "wininet.dll");
        }
        catch (std::filesystem::filesystem_error& err) {
            Logger::Print<Logger::FAILURE>("Error moving proxy: {}", err.what());
            MessageBoxA(0, err.what(), "Launcher Error", MB_OK);
            return;
        }

        //2. Launch exe
        STARTUPINFO info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;
        if (!CreateProcessA("btdb2_game.exe", 0, 0, 0, FALSE, 0, 0, 0, &info, &processInfo)) {
            Logger::Print<Logger::FAILURE>("Couldn't find game executable, is SMF installed properly?");
            MessageBoxA(0, "SoupedModFramework cannot find BTDB2, please ensure it is in the game's directory!", "SMF Error", MB_OK);
        }

        //3. Wait! The game will kill this process
        try {
            std::string sMsg(IPC::SpawnAndWaitForPipe(LAUNCH_STATUS_PIPE));
            if (sMsg == "exit") {
                exit(0);
            }
        }
        catch (std::exception& ex) {
            MessageBoxA(0, ex.what(), "Launcher error", MB_OK);
        }
    });
    launchThread.detach();

	return JSValueMakeBoolean(ctx, true);
}

JSValueRef AreWeNative(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    Logger::Print("Native check");
    return JSValueMakeBoolean(ctx, true);
}