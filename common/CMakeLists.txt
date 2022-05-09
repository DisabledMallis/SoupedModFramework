include(ExternalProject)

add_library(Common SHARED)

add_subdirectory(utils)

set(SDK_ROOT "${CMAKE_BINARY_DIR}/SDK/")
set(ULTRALIGHT_INCLUDE_DIR "${SDK_ROOT}/include")
set(ULTRALIGHT_BINARY_DIR "${SDK_ROOT}/bin")
set(ULTRALIGHT_RESOURCES_DIR "${SDK_ROOT}/resources")
set(ULTRALIGHT_INSPECTOR_DIR "${SDK_ROOT}/inspector")
set(ULTRALIGHT_LIBRARY_DIR "${SDK_ROOT}/lib")
set(S3_DOMAIN ".sfo2.cdn.digitaloceanspaces.com")
set(PLATFORM "win")
set(ARCHITECTURE "x64")
ExternalProject_Add(UltralightSDK
  URL https://ultralight-sdk${S3_DOMAIN}/ultralight-sdk-latest-${PLATFORM}-${ARCHITECTURE}.7z
  SOURCE_DIR "${SDK_ROOT}"
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

target_include_directories(Common PRIVATE "${ULTRALIGHT_INCLUDE_DIR}")
target_link_directories(Common PRIVATE "${ULTRALIGHT_LIBRARY_DIR}")
target_link_libraries(Common PRIVATE fmt UltralightCore AppCore Ultralight WebCore)

# Add target to build dir
add_custom_command(
	TARGET Common POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/common/Common.dll
		${FINAL_DIR}/Common.dll
)