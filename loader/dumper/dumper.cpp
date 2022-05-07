#include "dumper.h"

#include <fstream>

#include <logger.h>
#include <Bin2.h>

void Dumper::DumpToDisk(std::string fileName, std::filesystem::path bundlePath, std::string content)
{
	uint8_t* decryptBuffer = (uint8_t*)_malloca(content.size());
	if (!decryptBuffer) {
		Logger::Print<Logger::WARNING>("Failed to dump file! Couldn't allocate decryptBuffer");
		return;
	}
	memcpy(decryptBuffer, content.c_str(), content.size());
	std::string sDumpData;
	char file_header[] = "%BIN_2.0";
	if (content.size() < 8 || memcmp(file_header, decryptBuffer, sizeof(file_header) - 1) != 0)
	{
		//File isn't encrypted
	}
	else {
		Soup::Bin2::DecryptBytes<Soup::Bin2::INTERNAL>(decryptBuffer, content.size());
		sDumpData = std::string((char*)decryptBuffer, content.size() - 8); //8 bytes are removed because of checksums or smth
		Logger::Print("Decrypted {}", fileName);
	}

	std::filesystem::path cd = std::filesystem::current_path();
	std::filesystem::path dumpDir = cd / "dump";
	std::filesystem::path dumpFile = dumpDir / bundlePath.filename() / fileName;
	std::filesystem::create_directories(dumpFile.parent_path());
	std::ofstream dumpStream;
	dumpStream.open(dumpFile.string(), std::ios::trunc | std::ios::binary);
	dumpStream << sDumpData;
	dumpStream.close();
	Logger::Print("Dumped {}", fileName);
}
