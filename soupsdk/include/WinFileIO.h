#pragma once

#include <stdint.h>

namespace Soup {
	class IFileHelper {};
	class WinFileIO : public IFileHelper {};

	class IFile {};
	class CFile : public IFile {};
	class CCompressedFile : public CFile {};
	class CDecompressedFile : public CFile {};
};