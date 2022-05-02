project(Launcher C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

set(APP_NAME Launcher)
include(cmake/App.cmake)

set(SOURCES "main.cpp"
			"launcher.cpp"
			"interop/nativeLaunch.cpp"
			)

add_app("${SOURCES}")

target_link_libraries(Launcher PRIVATE Common fmt)

# Add target to build dir
add_custom_command(
	TARGET Launcher POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/${CMAKE_CONFIGURATION_TYPES}/Launcher.exe
		${FINAL_DIR}/Launcher.exe
)