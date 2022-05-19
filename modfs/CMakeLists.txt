add_library(ModFS SHARED)

target_include_directories(ModFS PUBLIC "include")

#Add ZipLib
FetchContent_Declare(
    ziplib
    GIT_REPOSITORY https://bitbucket.org/wbenny/ziplib.git
    GIT_TAG master
)
FetchContent_GetProperties(ziplib)
if(NOT ziplib_POPULATED)
    FetchContent_Populate(ziplib)
    add_library(ZipLib SHARED
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zAlloc.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zBuf.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zBuf2.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zCrc.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zCrcOpt.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zDec.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zFile.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zIn.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/7zStream.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/Alloc.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/Bcj2.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/Bra.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/Bra86.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/extlibs/lzma/BraIA64.c"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/ZipArchive.cpp"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/ZipArchiveEntry.cpp"
		"${ziplib_SOURCE_DIR}/Source/ZipLib/ZipFile.cpp"
	)
endif()
target_include_directories(ModFS PRIVATE "${ziplib_SOURCE_DIR}/Source/ZipLib/")

target_sources(ModFS PRIVATE
	"modfs.cpp"
)

target_link_libraries(ModFS PRIVATE Common)

# Add target to build dir
add_custom_command(
	TARGET ModFS POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/modfs/ModFS.dll
		${FINAL_DIR}/ModFS.dll
)