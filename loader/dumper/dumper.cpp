#include "dumper.h"

#include <fstream>

#include <logger.h>
#include <Bin2.h>
#include "../jpng/jpng.h"

void Dumper::DumpToDisk(std::string fileName, std::filesystem::path bundlePath, std::string content)
{
	Config* config = Config::GetConfig();
	if(!config->DumpAssets()) {
		return;
	}
	/*
	uint8_t* decryptBuffer = (uint8_t*)_malloca(content.size());
	if (!decryptBuffer) {
		Logger::Print<Logger::WARNING>("Failed to dump file! Couldn't allocate decryptBuffer");
		return;
	}
	memcpy(decryptBuffer, content.c_str(), content.size());

	std::filesystem::path cd = std::filesystem::current_path();
	std::filesystem::path dumpDir = cd / "dump";
	std::filesystem::path dumpFile = dumpDir / bundlePath.filename() / fileName;
	std::filesystem::create_directories(dumpFile.parent_path());
	std::ofstream dumpStream;

	static char bin2_header[] = "%BIN_2.0";
	static char jpng_header[] = "%JPNG001";
	if (memcmp(bin2_header, decryptBuffer, sizeof(bin2_header) - 1) == 0)
	{
		Soup::Bin2::DecryptBytes<Soup::Bin2::INTERNAL>(decryptBuffer, content.size());
		std::string sDumpData = std::string((char*)decryptBuffer, content.size() - 8); //8 bytes are removed because of checksums or smth
		Logger::Debug("Decrypted {}", fileName);

		dumpStream.open(dumpFile.string(), std::ios::trunc | std::ios::binary);
		dumpStream << sDumpData;
		dumpStream.close();
		Logger::Debug("Dumped {}", fileName);
		return;
	}
	if (memcmp(jpng_header, decryptBuffer, sizeof(jpng_header) - 1) == 0)
	{
		std::vector<uint8_t> jpngData(decryptBuffer, decryptBuffer + content.size());
		JPNG jpngImage(jpngData);
		std::vector<uint8_t> dumpData = jpngImage.ToPNG();

		dumpStream.open(dumpFile.string(), std::ios::trunc | std::ios::binary);
		dumpStream.write((const char*)dumpData.data(), dumpData.size());
		dumpStream.close();
		Logger::Debug("Dumped {}", fileName);
		return;
	}
	*/
}
