#include "hookManager.h"
#include <logger.h>
#include <memory.h>

#include <Windows.h>

#include <polyhook2/Detour/x64Detour.hpp>
#include <steam.h>
#include <ZipCpp.h>
#include <Bin2.h>

static PLH::x64Detour* plhDecompressFile;
static uint64_t oDecompressFile = Memory::FindSig(Soup::Signatures::SIG_ZIPCPP_DECOMPRESSFILE);
int hkDecompressFile(Soup::ZipIterator* pZipIterator, char* lpReadBuffer, uint32_t bufferSize) {
	//Get the bundle (.jet) file path
	std::filesystem::path bundlePath = pZipIterator->psArchivePath->cpp_str();
	//Get the bundle information
	Soup::ZipEntry* bundleData = pZipIterator->pZipEntry;
	//Get the entry & path for the current file
	std::string fileName = bundleData->GetName().cpp_str();
	int ret = PLH::FnCast(oDecompressFile, hkDecompressFile)(pZipIterator, lpReadBuffer, bufferSize);

	std::string sDumpData = std::string(lpReadBuffer, bufferSize);
	uint8_t* decryptBuffer = (uint8_t*)_malloca(bufferSize);
	if (!decryptBuffer) {
		Logger::Print<Logger::WARNING>("Failed to dump file! Couldn't allocate decryptBuffer");
		return ret;
	}
	memcpy(decryptBuffer, lpReadBuffer, bufferSize);
	char file_header[] = "%BIN_2.0";
	if (bufferSize < 8 || memcmp(file_header, decryptBuffer, sizeof(file_header) - 1) != 0) {
		//File isn't encrypted
	} 
	else {
		Soup::Bin2::DecryptBytes<Soup::Bin2::INTERNAL>(decryptBuffer, bufferSize);
		sDumpData = std::string((char*)decryptBuffer, bufferSize - 8); //8 bytes are removed because of checksums or smth
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

	return ret;
}

bool HookManager::ApplyHooks()
{
	plhDecompressFile = new PLH::x64Detour(oDecompressFile, (uint64_t)hkDecompressFile, &oDecompressFile);
	if (!plhDecompressFile->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook ZipCpp::DecompressFile");
		return false;
	}
    return true;
}