#pragma once

#include "archive.h"

namespace modfs {
	archive parseArchive(void* data);
	void* packArchive(archive& data);
}