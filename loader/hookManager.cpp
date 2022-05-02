#include "hookManager.h"
#include <logger.h>

#include <Windows.h>

#include <polyhook2/Detour/x64Detour.hpp>

PLH::x64Detour* plhCreateFileW;
uint64_t oCreateFileW;
HANDLE WINAPI hkCreateFileW(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwShareMode,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    _In_ DWORD dwCreationDisposition,
    _In_ DWORD dwFlagsAndAttributes,
    _In_opt_ HANDLE hTemplateFile
) {

    std::filesystem::path nextFile(lpFileName);

    Logger::print("CreateFileW at {}", nextFile.string());

    //Use a seperate session file
    if (nextFile.has_filename()) {
        //Check if the game is looking for the session file
        if (nextFile.filename().string() == "current.session") {
            //If so, redirect it to our custom session file
            nextFile = std::filesystem::current_path().append("modded.session").c_str();
            Logger::print("Redirected account to {}", nextFile.string());
        }
    }

    return PLH::FnCast(oCreateFileW, hkCreateFileW)(
        nextFile.c_str(),
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

bool HookManager::ApplyHooks()
{
    plhCreateFileW = new PLH::x64Detour((uint64_t)CreateFileW, (uint64_t)hkCreateFileW, &oCreateFileW);
    if (!plhCreateFileW->hook()) {
        Logger::print<Logger::FAILURE>("Failed to hook CreateFileW");
        return false;
    }

    return true;
}