#pragma once

#include <stdint.h>
#include <WinFileIO.h>

namespace Soup {
	namespace Signatures {
		static constexpr const char* SIG_ZIPCPP_DECOMPRESSFILE = "40 ?? 55 41 ?? 48 83 ?? ?? 48 ?? ?? 45 33";
		static constexpr const char* SIG_ZIPCPP_FINDFILE = "40 ?? 48 83 ?? ?? 48 8B ?? 48 85 ?? 74 ?? 48 85 ?? 74 ?? 8B";
		static constexpr const char* SIG_ZIPCPP_GETFILEINFO = "48 83 ?? ?? 8B ?? ?? ?? ?? ?? ?? ?? 44 ?? ?? 48 ?? ?? ?? ?? ?? ?? ?? ?? 89";
		static constexpr const char* SIG_ZIPCPP_OPENFILE = "48 83 ?? ?? ?? 89 ?? ?? ?? 45 33 ?? 33";
	}
	namespace ZipCpp {
		/*
		void GetFileInfo(Soup::CFile* pCFile, void* param_2, void* param_3, int param_4, void* param_5, int param_6, void* param_7, int param_8);
		int DecompressFile(Soup::ZipIterator* pZipIterator, char* lpReadBuffer, uint32_t bufferSize);
		*/
	};
};