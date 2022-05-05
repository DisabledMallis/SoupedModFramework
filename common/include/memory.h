#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <string>

struct SigInfo {
    std::string* signature;
    int offset;

    SigInfo(std::string* signature, int offset) {
        this->signature = signature;
        this->offset = offset;
    };
    ~SigInfo() {
        delete signature;
    }
};

class Memory {
    static inline HMODULE thisMod;
public:
    static uintptr_t FindMLvlPtr(uintptr_t, std::vector<unsigned int>);
    static auto FindSig(const char* pattern)->uintptr_t;
    static auto FindSig(long long rangeStart, long long rangeEnd, const char* pattern)->uintptr_t;
    static auto GetBaseModule() -> HMODULE;
    static void SetThisModule(HMODULE);
    static auto GetThisModule()->HMODULE;
    static auto GetModule(const char* moduleName)->HMODULE;
    static auto GetModuleSize(HMODULE moduleName)->size_t;
    static auto GetModuleEnd(HMODULE moduleName)->void*;
    static auto ReadTypeName(void* objPtr)->std::string {
        if (!objPtr) {
            return "INVALID PTR";
        }
        long long vtable = *(long long*)objPtr;
        if (!vtable || IsBadReadPtr((const void*)vtable, 0)) {
            return "NO VTABLE";

        }
        long long* vtableMeta = (long long*)(vtable - 0x8);
        if (!vtableMeta || IsBadReadPtr(*(const void**)vtableMeta, 0) || *(int*)vtableMeta == 0xcccccccc) {
            return "NO VTABLE META";
        }
        int descRef = *(int*)((*vtableMeta) + 0xC);
        if (!descRef) {
            return "NO DESC REF";
        }
        long long modBase = (long long)GetBaseModule();
        long long refInBase = (modBase + descRef);
        if (!refInBase) {
            return "NO DESC REF IN BASE";
        }
        const char* typeStr = (const char*)(refInBase + 0x10);
        return typeStr;
    }
    //static auto DemangleSymbol(std::string mangled)->std::string;
};