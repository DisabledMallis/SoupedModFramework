#pragma once

#define MFS_MAX_PATH 128

namespace modfs {
	class dataBlock {
		size_t blockSize;
		char* data;
	};
	class entry {
		char path[MFS_MAX_PATH];
		dataBlock block;
	};
	class archive {
		int archiveVer = 1;
		int entryCount;
		entry* entries;
	};
}