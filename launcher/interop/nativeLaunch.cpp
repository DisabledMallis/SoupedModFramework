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

        //1. Copy proxy dll into game dir
        if (!std::filesystem::exists("./proxies/wininet.dll")) {
            Logger::Print<Logger::FAILURE>("Couldn't find proxy dll, is SMF installed properly?");
            MessageBoxA(0, "Couldn't find proxy dll", "Launcher Error", MB_OK);
        }
        if (std::filesystem::exists("./wininet.dll")) {
            std::filesystem::remove("./wininet.dll");
        }
        try {
            std::filesystem::copy_file("./proxies/wininet.dll", "./wininet.dll");
        }
        catch (std::filesystem::filesystem_error err) {
            Logger::Print<Logger::FAILURE>("Error copying proxy: {}", err.what());
            MessageBoxA(0, err.what(), "Launcher Error", MB_OK);
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
            Logger::Print<Logger::FAILURE>("Couldn't find game executable, is SMF installed properly?");
            MessageBoxA(0, "SoupedModFramework cannot find BTDB2, please ensure it is in the game's directory!", "SMF Error", MB_OK);
        }

        //4. Wait! The game will kill this process
        try {
            Logger::Print("FUCK");
            std::string sMsg(IPC::SpawnAndWaitForPipe(LAUNCH_STATUS_PIPE));
            Logger::Print("YOU");
            if (sMsg == "exit") {
                Logger::Print("okbye");
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