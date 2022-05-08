#include "hookManager.h"

#include <logger.h>
#include <memory.h>
#include <steam.h>
#include <ZipCpp.h>
#include <Bin2.h>
#include "dumper/dumper.h"
#include <patchers.h>

#include <Windows.h>
#include <stack>
#include <polyhook2/Detour/x64Detour.hpp>


std::stack<std::string> patchworkStack;

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
	Logger::Print("Decompressed {}", fileName);

	char file_header[] = "%BIN_2.0";
	if (bufferSize < 8 || memcmp(file_header, lpReadBuffer, sizeof(file_header) - 1) != 0)
	{
		//File isn't BIN2 encrypted
		Logger::Print("{} is not BIN2 encrypted", fileName);
	}
	else {
		Dumper::DumpToDisk(fileName, bundlePath, std::string(lpReadBuffer, bufferSize));
		patchworkStack.push(fileName);
	}

	return ret;
}

static PLH::x64Detour* plhDecryptBytes;
static uint64_t oDecryptBytes = Memory::FindSig(Soup::Signatures::SIG_BIN2_DECRYPTBYTES);
void hkDecryptBytes(uint8_t** bytes) {
	PLH::FnCast(oDecryptBytes, hkDecryptBytes)(bytes);
	//If theres nothing to patch we're done
	if (patchworkStack.empty()) {
		return;
	}
	//Get the first file in the stack
	std::string targetFile = patchworkStack.top();
	//Pop it off the stack when the name is stored
	patchworkStack.pop();

	//Print we are patching it
	Logger::Print("Patching {}", targetFile);

	/*Patch the file*/
	std::string fileContent = std::string(*(char**)bytes, (size_t)(bytes[1] - bytes[0]));
	Patchers::PatchData(targetFile, fileContent);

	//Print we finished patching
	Logger::Print("Patched {}", targetFile);
}

bool HookManager::ApplyHooks()
{
	plhDecompressFile = new PLH::x64Detour(oDecompressFile, (uint64_t)hkDecompressFile, &oDecompressFile);
	if (!plhDecompressFile->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook ZipCpp::DecompressFile");
		return false;
	}

	plhDecryptBytes = new PLH::x64Detour(oDecryptBytes, (uint64_t)hkDecryptBytes, &oDecryptBytes);
	if (!plhDecryptBytes->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook Bin2::DecryptBytes");
		return false;
	}

    return true;
}