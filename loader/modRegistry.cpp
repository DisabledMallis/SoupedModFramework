#include "modRegistry.h"

#include <map>
#include <jsutils.h>

using namespace ModRegistry;
std::map<std::string, Mod*> modMap;
static Mod* currentMod = nullptr;

jsfunction(JS::readFile) {
	if (jsargc <= 1) {
		return JS_INVALID_REFERENCE;
	}
	JSUtils::JsValue jsRequestedFile = jsargv[1];
	Mod* targetPackage = nullptr;
	if (jsargc == 3) {
		JSUtils::JsValue jsModID = jsargv[2];
		targetPackage = ModRegistry::FindMod(jsModID);
	}
	else if (jsargc == 2) {
		targetPackage = GetImmediateMod();
	}
	JSUtils::JsValue entryContent = targetPackage->modFile.ReadEntry(jsRequestedFile);
	return entryContent;
}

ModRegistry::Mod::Mod(ModFS::Mod modFile)
{
	this->modFile = modFile;
}

void ModRegistry::Mod::Run()
{
	SetImmediateMod(this);
	JSUtils::JsValue modMeta = this->modFile.ReadEntry("meta.json");

	JSUtils::JsValue& global = JSUtils::GetGlobalObject();
	global.SetProperty("mod", modMeta);
	JSUtils::RunCode("Loader", "mod = JSON.parse(mod);");

	for (auto script : this->modFile.meta.scripts) {
		std::string scriptCode = this->modFile.ReadEntry(script);
		JSUtils::RunCode(this->modFile.meta.name + "/" + script, scriptCode);
	}
}

void ModRegistry::Mod::Kill()
{
}

void ModRegistry::LoadAllMods(std::filesystem::path cd)
{
	std::vector<ModFS::Mod> modFiles = ModFS::LoadAllMods(cd);
	for (auto modFile : modFiles) {
		LoadMod(modFile);
	}
}

Mod* ModRegistry::LoadMod(ModFS::Mod& modFile) {
	Mod* mod = new Mod(modFile);
	modMap.emplace(modFile.meta.modid, mod);
	return mod;
}

void ModRegistry::UnloadMod(Mod* mod) {
	modMap.erase(mod->modFile.meta.modid);
}

std::vector<Mod*> ModRegistry::GetMods()
{
	std::vector<Mod*> modsVec;
	for (const auto& [modid, mod] : modMap) {
		modsVec.push_back(mod);
	}
	return modsVec;
}

Mod* ModRegistry::FindMod(std::string modid) {
	return modMap[modid];
}
void ModRegistry::SetImmediateMod(Mod* currMod) {
	currentMod = currMod;
}
Mod* ModRegistry::GetImmediateMod() {
	return currentMod;
}