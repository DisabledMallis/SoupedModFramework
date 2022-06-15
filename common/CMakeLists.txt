include(ExternalProject)

add_library(Common SHARED)

add_subdirectory(glad)
add_subdirectory(imgui)
add_subdirectory(src)
add_subdirectory(utils)

target_include_directories(Common PRIVATE "${ULTRALIGHT_INCLUDE_DIR}")
target_link_directories(Common PRIVATE "${ULTRALIGHT_LIBRARY_DIR}")
target_link_libraries(Common PUBLIC fmt glfw ChakraCore archive)

# Add target to build dir
add_custom_command(
	TARGET Common POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/common/${CMAKE_BUILD_TYPE}/Common.dll
		${FINAL_DIR}/Common.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${ch_SOURCE_DIR}/Build/VcBuild/bin/x64_${CMAKE_BUILD_TYPE}/ChakraCore.dll
		${FINAL_DIR}/ChakraCore.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${libarchive_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/archive.dll
		${FINAL_DIR}/archive.dll
)