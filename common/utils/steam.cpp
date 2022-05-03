#include "steam.h"
#include "registry.h"

std::filesystem::path Steam::GetSteamDir()
{
	return Registry::ReadString(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath");
}

std::filesystem::path Steam::GetUserdataDir()
{
	return GetSteamDir().append("userdata");
}

std::string Steam::GetUserID()
{
	int user_id = Registry::ReadDWORD(HKEY_CURRENT_USER, "SOFTWARE\\Valve\\Steam\\ActiveProcess", "ActiveUser");
	std::string s_user_id = std::to_string(user_id);
	return s_user_id;
}