#include <Memory.h>
#include <DbgHelp.h>
#include <logger.h>

uintptr_t Memory::FindMLvlPtr(uintptr_t baseAddr, std::vector<unsigned int> offsets) {
	uintptr_t addr = baseAddr;
	for (int I = 0; I < offsets.size(); I++) {
		addr = *(uintptr_t*)(addr);
		if ((uintptr_t*)(addr) == nullptr)
			return addr;
		addr += offsets[I];
	}
	return addr;
}

auto Memory::GetBaseModule() -> HMODULE {
	return GetModuleHandleA(nullptr);
}

void Memory::SetThisModule(HMODULE mod) {
	thisMod = mod;
}
auto Memory::GetThisModule() -> HMODULE {
	return thisMod;
}

auto Memory::GetModule(const char* moduleName) -> HMODULE
{
	return GetModuleHandleA(moduleName);
}

auto Memory::GetModuleSize(HMODULE hModule) -> size_t
{
	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), hModule, &info, sizeof(info));
	return info.SizeOfImage;
}

auto Memory::GetModuleEnd(HMODULE hModule) -> void*
{
	return (void*)((size_t)hModule + GetModuleSize(hModule));
}

#define INRANGE(x,a,b)	(x >= a && x <= b)
#define getBits( x )	(INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )	(getBits(x[0]) << 4 | getBits(x[1]))

auto Memory::FindSig(const char* pattern) -> uintptr_t {
	return FindSig((long long)GetBaseModule(), (long long)GetModuleEnd(GetBaseModule()), pattern);
}
struct SearchedSig {
	size_t rangeStart;
	size_t rangeEnd;
	std::string pattern;
	uintptr_t result;
	SearchedSig(size_t rangeStart, size_t rangeEnd, std::string pattern, uintptr_t result) {
		this->rangeStart = rangeStart;
		this->rangeEnd = rangeEnd;
		this->pattern = pattern;
		this->result = result;
	}
};

std::vector<SearchedSig> alreadySearched = std::vector<SearchedSig>();
auto Memory::FindSig(long long rangeStart, long long rangeEnd, const char* pattern) -> uintptr_t {
	for (int i = 0; i < alreadySearched.size(); i++) {
		SearchedSig searched = alreadySearched[i];
		if (searched.pattern == std::string(pattern) && searched.rangeStart == rangeStart && searched.rangeEnd == rangeEnd) {
			return searched.result;
		}
	}
	std::string sanitizedPat = pattern;
	int skips = 0;
	while (sanitizedPat[0] == '?' && sanitizedPat[1] == '?') {
		sanitizedPat = sanitizedPat.substr(3);
		skips++;
		Logger::Debug("Sig skips: {}", skips);
		Logger::Debug("Current sig: {}", sanitizedPat);
	}
	const char* pat = sanitizedPat.c_str();
	long long firstMatch = 0;
	for (long long pCur = rangeStart; pCur < rangeEnd; pCur++) {
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) {
				firstMatch += -skips;
				alreadySearched.push_back(SearchedSig(rangeStart, rangeEnd, std::string(pattern), firstMatch));
				return firstMatch;
			};
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') pat += 3;
			else pat += 2;
		}
		else {
			pat = sanitizedPat.c_str();
			firstMatch = 0;
		}
	}
	MessageBoxA(nullptr, pattern, "SCAN FAILURE", MB_OK);
	return 0;
}