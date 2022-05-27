cmake_minimum_required(VERSION 3.17)
project(SoupedModLoader VERSION 1.0 LANGUAGES C CXX)

# Use C++ 20
set(CMAKE_CXX_STANDARD 20)

# Include CMake's FetchContent module
include(FetchContent)

# Set runtime libs
if(MSVC)
    add_compile_options(
	    $<$<CONFIG:>:/MD> #---------|
	    $<$<CONFIG:Debug>:/MDd> #---|-- Dynamically link the runtime libraries
	    $<$<CONFIG:Release>:/MD> #--|
    )
endif()

#Add fmt
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
)
FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
endif()

#Fetch & Install PolyHook2
FetchContent_Declare(
    plh
    GIT_REPOSITORY https://github.com/stevemk14ebr/PolyHook_2_0.git
    GIT_TAG master
)
FetchContent_GetProperties(plh)
if(NOT plh_POPULATED)
    FetchContent_Populate(plh)
    set(POLYHOOK_BUILD_DLL ON)
    #set(POLYHOOK_BUILD_SHARED_LIB OFF)
    add_subdirectory(${plh_SOURCE_DIR} ${plh_BINARY_DIR})
endif()

#Add glfw
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3-stable
)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

#Fetch & Install ChakraCore
FetchContent_Declare(
    ch
    GIT_REPOSITORY https://github.com/chakra-core/ChakraCore.git
    GIT_TAG master
)
FetchContent_GetProperties(ch)
if(NOT ch_POPULATED)
    FetchContent_Populate(ch)
    execute_process(COMMAND msbuild /m /p:Platform=x64 /p:Configuration=${CMAKE_BUILD_TYPE} ${ch_SOURCE_DIR}/Build/Chakra.Core.sln)
    add_library(ChakraCore SHARED IMPORTED)
    set_target_properties(ChakraCore PROPERTIES
        IMPORTED_LOCATION "${ch_SOURCE_DIR}/Build/VcBuild/bin/x64_${CMAKE_BUILD_TYPE}/ChakraCore.dll"
        IMPORTED_IMPLIB "${ch_SOURCE_DIR}/Build/VcBuild/bin/x64_${CMAKE_BUILD_TYPE}/ChakraCore.lib"
    )
endif()

#Add zlib
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG master
)
FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
    FetchContent_Populate(zlib)
    add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
endif()

#Add libarchive
FetchContent_Declare(
    libarchive
    GIT_REPOSITORY https://github.com/libarchive/libarchive.git
    GIT_TAG master
)
FetchContent_GetProperties(libarchive)
if(NOT libarchive_POPULATED)
    FetchContent_Populate(libarchive)
    add_subdirectory(${libarchive_SOURCE_DIR} ${libarchive_BINARY_DIR})
endif()

# Include library include dir
include_directories("${glfw_SOURCE_DIR}/include/")
include_directories("${libarchive_SOURCE_DIR}/libarchive")
include_directories("${ch_SOURCE_DIR}/lib/jsrt/")

# Final output dir for compile files
set(FINAL_DIR "${CMAKE_CURRENT_LIST_DIR}/build/${CMAKE_BUILD_TYPE}")

# Include SMF headers globally
include_directories("${CMAKE_CURRENT_LIST_DIR}/common/include/")
include_directories("${CMAKE_CURRENT_LIST_DIR}/common/glad/include/")
include_directories("${CMAKE_CURRENT_LIST_DIR}/common/imgui/")
include_directories("${CMAKE_CURRENT_LIST_DIR}/loader/include/")
include_directories("${CMAKE_CURRENT_LIST_DIR}/modfs/include/")
include_directories("${CMAKE_CURRENT_LIST_DIR}/soupsdk/include/")

# Export all symbols (ensures .lib files are created)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Add SMF sources
add_subdirectory(common)
add_subdirectory(launcher)
add_subdirectory(loader)
add_subdirectory(modexplore)
add_subdirectory(modfs)
add_subdirectory(proxy)
add_subdirectory(soupsdk)

# Create release ready folders
execute_process(COMMAND mkdir "${CMAKE_CURRENT_LIST_DIR}/build")
execute_process(COMMAND mkdir ${FINAL_DIR})
execute_process(COMMAND mkdir ${FINAL_DIR}/loaders)
execute_process(COMMAND mkdir ${FINAL_DIR}/proxies)