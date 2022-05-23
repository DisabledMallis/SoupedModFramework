#pragma once

#include <stdint.h>
#include <SoupSTL.h>

namespace Soup {
	class IFileHelper {};
	class WinFileIO : public IFileHelper {};

	class IFile {};
	class CFile : public IFile {};
	class CDecompressedFile : public CFile {};

	class ZipEntry {
	public:
		char* entryPath; //0x0000
		struct {
			char type[4]; //0x0000
			int16_t madeBy; //0x0004
			int16_t ver; //0x0006
			int16_t flags; //0x0008
			int16_t method; //0x000A
			int32_t date; //0x000C
			int32_t crc; //0x0010
			int32_t rawSize; //0x0014
			int32_t size; //0x0018
			int16_t filenameLen; //0x001C
			int16_t extraLen; //0x001E
			int16_t commentLen; //0x0020
			int16_t disk; //0x0022
			int16_t intAttrib; //0x0024
			int32_t extAttrib; //0x0026
			uint32_t fileOffset; //0x002A
			char filename[0]; //0x002E
		}* pEntryData; //0x0008

	public:
		Soup::String GetName() {
			return Soup::String(entryPath, pEntryData->filenameLen);
		}
	};
	class ZipIterator
	{
	public:
		char pad_0000[56]; //0x0000
		Soup::String* psArchivePath; //0x0038
		ZipEntry* pZipEntry; //0x0040
	};
	class ZipReader {
	public:
		ZipIterator* pZipIterator;
		uint8_t* pReadBuffer;
		uint32_t bufferSize;
	};
};