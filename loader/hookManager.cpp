#include "hookManager.h"
#include <logger.h>
#include <memory.h>

#include <Windows.h>

#include <polyhook2/Detour/x64Detour.hpp>
#include <steam.h>

static PLH::x64Detour* plhCreateFileW;
static uint64_t oCreateFileW;
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

    Logger::Print("CreateFileW at {}", nextFile.string());

    //Use a seperate session file
    if (nextFile.has_filename()) {
        //Check if the game is looking for the session file
        if (nextFile.filename().string() == "current.session") {
            //If so, redirect it to our custom session file
            std::filesystem::path moddedSession = std::filesystem::current_path().append("smf/modded.session").c_str();
            Logger::Print("Redirected account from {} to {}", nextFile.string(), moddedSession.string());
            nextFile = moddedSession;
        }
    }

    HANDLE hFile = PLH::FnCast(oCreateFileW, hkCreateFileW)(
        nextFile.c_str(),
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);

    if (hFile == INVALID_HANDLE_VALUE) {
        Logger::Print<Logger::FAILURE>("Game created an invalid handle?");
    }

    std::filesystem::path steamUserdata = Steam::GetUserdataDir();
    std::filesystem::path cacheDir = steamUserdata.append("local/Data/Cache");
    std::filesystem::path dumpDir = std::filesystem::current_path().append("dump");
    std::string sNextFile = nextFile.string();
    size_t cacheIdx = sNextFile.find("Cache");
    if (cacheIdx != std::string::npos) {
        //Get the path for the dump file
        std::string relGameAsset = sNextFile.substr(cacheIdx + strlen("Cache/"));
        std::filesystem::path dumpPath = dumpDir.append(relGameAsset);
        std::filesystem::create_directories(dumpPath.parent_path());
        Logger::Print("Dumping {} to {}", sNextFile, dumpPath.string());
        //Get the original file size
        int dumpSize = GetFileSize(hFile, NULL);
        char* fileBuffer = (char*)_malloca(dumpSize + 1);
        if (ReadFile(hFile, fileBuffer, dumpSize, NULL, NULL) == FALSE) {
            Logger::Print<Logger::WARNING>("Failed to dump file {} GLE: {}", dumpPath.string(), GetLastError());
        }
        //Get the dump file
        HANDLE hDumpFile = PLH::FnCast(oCreateFileW, hkCreateFileW)(
            dumpPath.wstring().c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        //Write the file contents
        WriteFile(hDumpFile, fileBuffer, dumpSize, NULL, NULL);
        //Close it
        //CloseHandle(hDumpFile);
        return hDumpFile;
        CloseHandle(hFile);
    }

    return hFile;
}



bool HookManager::ApplyHooks()
{
    plhCreateFileW = new PLH::x64Detour((uint64_t)CreateFileW, (uint64_t)hkCreateFileW, &oCreateFileW);
    if (!plhCreateFileW->hook()) {
        Logger::Print<Logger::FAILURE>("Failed to hook CreateFileW");
        return false;
    }

    return true;
}