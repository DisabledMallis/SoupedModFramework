#include <modfs.h>
#include <nlohmann/json.hpp>
#include <logger.h>

ModFS::Mod::Mod(std::filesystem::path pathOnDisk) {
	this->pArchive = ZipFile::Open(pathOnDisk.string());
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
	ZipArchiveEntry::Ptr pEntry = this->pArchive->GetEntry(entry);
	std::istream* decompressStream = pEntry->GetDecompressionStream();
	std::string fileContent((std::istreambuf_iterator<char>(*decompressStream)), std::istreambuf_iterator<char>());
	pEntry->CloseDecompressionStream();
	return fileContent;
}

ModFS::Mod ModFS::OpenArchive(std::string pathOnDisk) {
	return ModFS::Mod(pathOnDisk);
}