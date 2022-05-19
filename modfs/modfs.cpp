#include <modfs.h>
#include <nlohmann/json.hpp>
#include <logger.h>

ModFS::Mod::Mod(std::filesystem::path pathOnDisk) {
	this->innerArchive = ZipUtils::ArchiveWrapper::OpenArchive(pathOnDisk);
	this->meta = ModFS::ModMeta();
	std::string modMeta = this->ReadEntry("meta.json");
	nlohmann::json metaJson = nlohmann::json::parse(modMeta);
	if (metaJson.contains("authors")) {
		this->meta.authors = metaJson["authors"];
	}
	if (metaJson.contains("description")) {
		this->meta.description = metaJson["description"];
	}
	if (metaJson.contains("name")) {
		this->meta.name = metaJson["name"];
	}
	else {
		Logger::Print<Logger::FAILURE>("Mod {} is missing the field '{}' in the meta.json", pathOnDisk.filename().string(), "name");
	}
	if (metaJson.contains("scripts")) {
		this->meta.scripts = metaJson["scripts"];
	}
	if (metaJson.contains("version")) {
		this->meta.version = metaJson["version"];
	}
	else {
		Logger::Print<Logger::FAILURE>("Mod {} is missing the field '{}' in the meta.json", pathOnDisk.filename().string(), "version");
	}
}

std::string ModFS::Mod::ReadEntry(std::string entry) {
	return this->innerArchive->ReadEntry(entry);
}

ModFS::Mod ModFS::OpenArchive(std::filesystem::path pathOnDisk) {
	return ModFS::Mod(pathOnDisk);
}