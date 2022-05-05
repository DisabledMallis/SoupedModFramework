#pragma once

#include <stdint.h>
#include <SoupSTL.h>

namespace Soup {
	class IFileHelper {};
	class WinFileIO : public IFileHelper {};

	class IFile {
	public:
		char pad_0000[56]; //0x0000
		Soup::String* psBundlePath; //0x0038
		Soup::String* pbFilePath; //0x0040
	};
	class CFile : public IFile {};
	class CCompressedFile : public CFile {};
	class CDecompressedFile : public CFile {};
};