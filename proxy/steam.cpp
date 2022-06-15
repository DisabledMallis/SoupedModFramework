#include <stdint.h>

/*
* Stubs for steam api functions that need to be replaced
*/

#define S_API extern "C"
#define S_CALLTYPE __fastcall
typedef int32_t HSteamUser;

S_API HSteamUser S_CALLTYPE SteamAPI_GetHSteamUser() {
	return 0;
}

S_API bool S_CALLTYPE SteamAPI_Init() {
	return true;
}

S_API bool S_CALLTYPE SteamAPI_IsSteamRunning() {
	return true;
}

S_API void S_CALLTYPE SteamAPI_RegisterCallback(int64_t param_1, uint32_t param_2) {
	return;
}

S_API void S_CALLTYPE SteamAPI_RegisterCallResult(uint64_t param_1, int64_t* param_2) {
	return;
}

S_API uint64_t S_CALLTYPE SteamAPI_RestartAppIfNecessary(uint32_t param_1, uint64_t param_2, uint64_t param_3, uint64_t param_4) {
	return 0;
}

S_API void S_CALLTYPE SteamAPI_RunCallbacks() {
	return;
}

S_API void S_CALLTYPE SteamAPI_Shutdown() {
	return;
}

S_API void S_CALLTYPE SteamAPI_UnregisterCallback(int64_t* param_1) {
	return;
}

S_API void S_CALLTYPE SteamAPI_UnregisterCallResult(int64_t* param_1, int64_t* param_2) {
	return;
}

S_API uint64_t** S_CALLTYPE SteamInternal_ContextInit(uint64_t** param_1) {
	*(param_1 + 0x10) = nullptr;
	return param_1 + 0x10;
}

S_API uint64_t S_CALLTYPE SteamInternal_FindOrCreateUserInterface(uint32_t param_1, int64_t param_2, uint64_t param_3, uint64_t param_4) {
	return 0;
}