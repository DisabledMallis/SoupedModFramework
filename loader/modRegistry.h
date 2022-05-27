#pragma once

#include <modfs.h>
#include <jsutils.h>

namespace ModRegistry {
	namespace JS {
		jsfunction(readFile);
	}
	struct Mod {
		ModFS::Mod modFile;
	public:
		Mod(ModFS::Mod modFile);
		void Run();
		void Kill();
	};

	void LoadAllMods(std::filesystem::path);
	Mod* LoadMod(ModFS::Mod&);
	void UnloadMod(Mod*);
	std::vector<Mod*> GetMods();
	Mod* FindMod(std::string);
	void SetImmediateMod(Mod*);
	Mod* GetImmediateMod();
};