add_library(Loader SHARED)

target_sources(Loader PRIVATE
	"hookManager.cpp"
	"main.cpp"
  "patcher/patchers/BattleMenuPatcher.h" "patcher/patchers/BattleMenuPatcher.cpp")

add_subdirectory(assets)
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
target_link_libraries(Loader PRIVATE Common fmt PolyHook_2 SoupSDK UltralightCore AppCore Ultralight WebCore glfw opengl32)

# Add target to build dir
add_custom_command(
	TARGET Loader POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/loader/Loader.dll
		${FINAL_DIR}/loaders/Loader.dll
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_LIST_DIR}/assets
		${FINAL_DIR}/assets
)