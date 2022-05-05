#include <ZipCpp.h>

#include <polyhook2/Detour/x64Detour.hpp>
#include <memory.h>
#include <logger.h>
#include <string>

static PLH::x64Detour* plhDecompressFile;
static uint64_t oDecompressFile = Memory::FindSig(Soup::Signatures::SIG_ZIPCPP_DECOMPRESSFILE);
int hkDecompressFile(Soup::CCompressedFile* pCCompressedFile, char* lpReadBuffer, uint32_t bufferSize) {
	std::string fileName = pCCompressedFile->pbFilePath->cpp_str();
	int ret = PLH::FnCast(oDecompressFile, hkDecompressFile)(pCCompressedFile, lpReadBuffer, bufferSize);
	Logger::Print("fileName: {}", fileName);
	return ret;
}

bool Soup::ZipCpp::CreateHooks() {
	plhDecompressFile = new PLH::x64Detour(oDecompressFile, (uint64_t)hkDecompressFile, &oDecompressFile);
	if (!plhDecompressFile->hook()) {
		Logger::Print<Logger::FAILURE>("Failed to hook ZipCpp::DecompressFile");
		return false;
	}
	return true;
}

void Soup::ZipCpp::GetFileInfo(CCompressedFile* pCCompressedFile, void* param_2, void* param_3, int param_4, void* param_5, int param_6, void* param_7, int param_8) {

}

int Soup::ZipCpp::DecompressFile(CCompressedFile* pCCompressedFile, char* lpReadBuffer, uint32_t bufferSize) {
	return PLH::FnCast(oDecompressFile, DecompressFile)(pCCompressedFile, lpReadBuffer, bufferSize);
}