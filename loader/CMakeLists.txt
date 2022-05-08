add_library(Loader SHARED)

target_sources(Loader PRIVATE
	"hookManager.cpp"
	"jsfuncs.cpp"
	"jspatcher.cpp"
	"jswrapper.cpp"
	"main.cpp"
)

add_subdirectory(dumper)
add_subdirectory(patcher)

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
endif()

target_include_directories(Loader PRIVATE "${CMAKE_BINARY_DIR}/_deps/ch-src/lib/jsrt/")
target_link_directories(Loader PRIVATE "${ch_SOURCE_DIR}/Build/VcBuild/bin/x64_${CMAKE_BUILD_TYPE}")

target_link_libraries(Loader PRIVATE Common fmt PolyHook_2 SoupSDK ChakraCore)

# Add target to build dir
add_custom_command(
	TARGET Loader POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/loader/Loader.dll
		${FINAL_DIR}/loaders/Loader.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${ch_SOURCE_DIR}/Build/VcBuild/bin/x64_${CMAKE_BUILD_TYPE}/ChakraCore.dll
		${FINAL_DIR}/ChakraCore.dll
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_LIST_DIR}/js
		${FINAL_DIR}/js
)