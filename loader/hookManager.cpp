#include "hookManager.h"
#include <logger.h>
#include <memory.h>

#include <Windows.h>

#include <polyhook2/Detour/x64Detour.hpp>
#include <steam.h>
#include <ZipCpp.h>

static PLH::x64Detour* plhDecompressFile;
static uint64_t oDecompressFile = Memory::FindSig(Soup::Signatures::SIG_ZIPCPP_DECOMPRESSFILE);
int hkDecompressFile(Soup::ZipIterator* pZipIterator, char* lpReadBuffer, uint32_t bufferSize) {
	//Get the bundle (.jet) file path
	std::string bundlePath = pZipIterator->psArchivePath->cpp_str();
	//Get the bundle information
	Soup::ZipEntry* bundleData = pZipIterator->pZipEntry;
	//Get the entry & path for the current file
	std::string fileName = bundleData->GetName().cpp_str();
	int ret = PLH::FnCast(oDecompressFile, hkDecompressFile)(pZipIterator, lpReadBuffer, bufferSize);

	std::filesystem::path cd = std::filesystem::current_path();
	std::filesystem::path dumpDir = cd / "dump";
	std::filesystem::path dumpFile = dumpDir / fileName;
	std::filesystem::create_directories(dumpFile.parent_path());
	std::ofstream dumpStream;
	dumpStream.open(dumpFile.string(), std::ios::trunc | std::ios::binary);
	dumpStream.write(lpReadBuffer, bufferSize);
	dumpStream.close();

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