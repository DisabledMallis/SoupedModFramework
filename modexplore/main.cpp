#include <modfs.h>
#include <logger.h>

int main(const char* argv, int argc) {
	std::filesystem::path cd = std::filesystem::current_path();
	std::vector<ModFS::Mod> allMods = ModFS::LoadAllMods(cd);

	for (auto& mod : allMods) {
		for (auto& author : mod.meta.authors) {
			Logger::Print("Author: {}", author);
		}
		Logger::Print("Description: {}", mod.meta.description);
		Logger::Print("Name: {}", mod.meta.name);
		for (auto& script : mod.meta.scripts) {
			Logger::Print("Script: {}", script);
		}
		Logger::Print("version: {}", mod.meta.version);
	}

	std::cin.get();
	return 0;
}