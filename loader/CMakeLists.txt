add_library(Loader SHARED)

target_sources(Loader PRIVATE
	"hookManager.cpp"
	"jsfuncs.cpp"
	"jswrapper.cpp"
	"main.cpp" "ui/gl/GPUDriverImpl.cpp" "ui/gl/GPUDriverImpl.h")

add_subdirectory(dumper)
add_subdirectory(patcher)
add_subdirectory(ui)

set(SDK_ROOT "${CMAKE_BINARY_DIR}/SDK/")
set(ULTRALIGHT_INCLUDE_DIR "${SDK_ROOT}/include")
set(ULTRALIGHT_BINARY_DIR "${SDK_ROOT}/bin")
set(ULTRALIGHT_RESOURCES_DIR "${SDK_ROOT}/resources")
set(ULTRALIGHT_INSPECTOR_DIR "${SDK_ROOT}/inspector")
set(ULTRALIGHT_LIBRARY_DIR "${SDK_ROOT}/lib")

target_include_directories(Loader PRIVATE "${ULTRALIGHT_INCLUDE_DIR}")
target_link_directories(Loader PRIVATE "${ULTRALIGHT_LIBRARY_DIR}")
target_link_libraries(Loader PRIVATE Common fmt PolyHook_2 SoupSDK UltralightCore AppCore Ultralight WebCore glfw)

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