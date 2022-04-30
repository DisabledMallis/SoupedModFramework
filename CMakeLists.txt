cmake_minimum_required(VERSION 3.17)
project(Nuvola VERSION 1.0 LANGUAGES CSharp CXX)

# Use C++ 20
set(CMAKE_CXX_STANDARD 20)
# Use C# 11
set_property(GLOBAL PROPERTY VS_GLOBAL_LangVersion "10")

add_subdirectory(b2smf-js)
add_subdirectory(launcher)
add_subdirectory(loader)
add_subdirectory(modfs)