cmake_minimum_required(VERSION 3.17)
project(SoupedModLoader VERSION 1.0 LANGUAGES CXX)

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

# Final output dir for compile files
set(FINAL_DIR "${CMAKE_CURRENT_LIST_DIR}/build/${CMAKE_BUILD_TYPE}")

# Include SMF sources globally
include_directories("${CMAKE_CURRENT_LIST_DIR}/common/include/")
include_directories("${CMAKE_CURRENT_LIST_DIR}/soupsdk/include/")

# Add SMF sources
add_subdirectory(b2smf-js)
add_subdirectory(common)
add_subdirectory(launcher)
add_subdirectory(loader)
add_subdirectory(modfs)
add_subdirectory(proxy)

# Create release ready folders
execute_process(COMMAND mkdir ${FINAL_DIR})
execute_process(COMMAND mkdir ${FINAL_DIR}/loaders)
execute_process(COMMAND mkdir ${FINAL_DIR}/proxies)