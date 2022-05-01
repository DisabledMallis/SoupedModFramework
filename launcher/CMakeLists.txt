project(Launcher C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

set(APP_NAME Launcher)
include(cmake/App.cmake)

set(SOURCES "main.cpp"
			"launcher.cpp")

add_app("${SOURCES}")