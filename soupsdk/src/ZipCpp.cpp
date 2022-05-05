#include <ZipCpp.h>

#include <polyhook2/Detour/x64Detour.hpp>
#include <memory.h>
#include <logger.h>
#include <string>

void Soup::ZipCpp::GetFileInfo(CFile* pCFile, void* param_2, void* param_3, int param_4, void* param_5, int param_6, void* param_7, int param_8) {

}

static uint64_t oDecompressFile = Memory::FindSig(Soup::Signatures::SIG_ZIPCPP_DECOMPRESSFILE);
int Soup::ZipCpp::DecompressFile(Soup::ZipIterator* pZipIterator, char* lpReadBuffer, uint32_t bufferSize) {
	return PLH::FnCast(oDecompressFile, DecompressFile)(pZipIterator, lpReadBuffer, bufferSize);
}