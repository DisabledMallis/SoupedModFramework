#include "hookManager.h"
#include <logger.h>
#include <memory.h>

#include <Windows.h>

#include <polyhook2/Detour/x64Detour.hpp>
#include <steam.h>
#include <ZipCpp.h>

bool HookManager::ApplyHooks()
{
    if (!Soup::ZipCpp::CreateHooks()) {
        return false;
    }
    return true;
}