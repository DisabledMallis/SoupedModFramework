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
		${CMAKE_BINARY_DIR}/launcher/Launcher.exe
		${FINAL_DIR}/Launcher.exe
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/AppCore.dll
		${FINAL_DIR}/AppCore.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/Ultralight.dll
		${FINAL_DIR}/Ultralight.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/UltralightCore.dll
		${FINAL_DIR}/UltralightCore.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/WebCore.dll
		${FINAL_DIR}/WebCore.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/glib-2.0-0.dll
		${FINAL_DIR}/glib-2.0-0.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/gstreamer-full-1.0.dll
		${FINAL_DIR}/gstreamer-full-1.0.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/gobject-2.0-0.dll
		${FINAL_DIR}/gobject-2.0-0.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/gio-2.0-0.dll
		${FINAL_DIR}/gio-2.0-0.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/gmodule-2.0-0.dll
		${FINAL_DIR}/gmodule-2.0-0.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/launcher/gthread-2.0-0.dll
		${FINAL_DIR}/gthread-2.0-0.dll
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_BINARY_DIR}/launcher/assets
		${FINAL_DIR}/assets
)