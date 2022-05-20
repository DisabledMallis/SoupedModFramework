#include <ziputils.h>
#include <archive.h>
#include <archive_entry.h>
#include <logger.h>

ZipUtils::ArchiveWrapper::ArchiveWrapper(std::filesystem::path pathOnDisk) {
	this->pathOnDisk = pathOnDisk;
}
ZipUtils::ArchiveWrapper::~ArchiveWrapper() {}
ZipUtils::ArchiveWrapper* ZipUtils::ArchiveWrapper::OpenArchive(std::filesystem::path pathOnDisk)
{
	ZipUtils::ArchiveWrapper* archiveWrapper = new ZipUtils::ArchiveWrapper(pathOnDisk);
	return archiveWrapper;
}
std::string ZipUtils::ArchiveWrapper::ReadEntry(std::string path)
{
	archive* tempArchive = archive_read_new();
	archive_read_support_format_all(tempArchive);
	archive_read_support_compression_all(tempArchive);
	archive_read_open_filename(tempArchive, pathOnDisk.string().c_str(), 10240);
	archive_entry* pEntry;
	while (true) {
		auto r = archive_read_next_header(tempArchive, &pEntry);
		if (r == ARCHIVE_EOF) {
			break;
		}
		if (r == ARCHIVE_WARN) {
			int errNum = archive_errno(tempArchive);
			const char* errStr = archive_error_string(tempArchive);
			Logger::Print<Logger::WARNING>("Warning (libarchive #{}): {}", errNum, std::string(errStr));
		}
		if (r == ARCHIVE_FAILED) {
			Logger::Print<Logger::FAILURE>("Failed to open archive {} to read path {}", this->pathOnDisk.filename().string(), path);
			break;
		}
		if (r == ARCHIVE_FATAL) {
			Logger::Print<Logger::FAILURE>("libarchive reported a FATAL error. Failed to open archive {} to read path {}", this->pathOnDisk.filename().string(), path);
			break;
		}
		std::string entryPathName = archive_entry_pathname(pEntry);
		Logger::Debug("Entry Name: {}", entryPathName);
		if (entryPathName == path) {
			const char* buff;
			size_t size;
			la_int64_t offset;
			r = archive_read_data_block(tempArchive, (const void**)&buff, &size, &offset);
			if (r == ARCHIVE_EOF || r == ARCHIVE_OK) {
				//Everythings okay
			}
			if (r == ARCHIVE_WARN) {
				int errNum = archive_errno(tempArchive);
				const char* errStr = archive_error_string(tempArchive);
				Logger::Print<Logger::WARNING>("Warning (libarchive #{}): {}", errNum, std::string(errStr));
			}
			if (r == ARCHIVE_FAILED) {
				int errNum = archive_errno(tempArchive);
				const char* errStr = archive_error_string(tempArchive);
				Logger::Print<Logger::FAILURE>("Failed to open archive entry '{}': {} (Make sure you are using 'store' for compression in 7zip!)", path, std::string(errStr));
				break;
			}
			if (r == ARCHIVE_FATAL) {
				int errNum = archive_errno(tempArchive);
				const char* errStr = archive_error_string(tempArchive);
				Logger::Print<Logger::FAILURE>("libarchive reported a FATAL error. Failed to open archive entry '{}': ", path, std::string(errStr));
				break;
			}
			std::string result = std::string(buff, size);
			archive_read_close(tempArchive);
			archive_read_free(tempArchive);
			return result;
		}
	}
	Logger::Print<Logger::WARNING>("Couldn't find entry {}", path);
	archive_read_close(tempArchive);
	archive_read_free(tempArchive);
	return "";
}