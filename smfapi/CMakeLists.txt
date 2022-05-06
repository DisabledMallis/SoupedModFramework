add_library(SMFApi SHARED)

target_sources(SMFApi PRIVATE
    "main.cpp"
    "jswrapper.cpp")

#Fetch & Install ChakraCore
FetchContent_Declare(
    ch
    GIT_REPOSITORY https://github.com/chakra-core/ChakraCore.git
    GIT_TAG master
)
FetchContent_GetProperties(ch)
if(NOT ch_POPULATED)
    FetchContent_Populate(ch)
    execute_process(COMMAND msbuild /m /p:Platform=x64 /p:Configuration=${CMAKE_BUILD_TYPE} ${CMAKE_BINARY_DIR}/_deps/ch-src/Build/Chakra.Core.sln)
endif()

target_include_directories(SMFApi PRIVATE "${CMAKE_BINARY_DIR}/_deps/ch-src/lib/jsrt/")
target_link_directories(SMFApi PRIVATE "${CMAKE_BINARY_DIR}/_deps/ch-src/Build/VcBuild/bin/x64_${CMAKE_BUILD_TYPE}")

target_link_libraries(SMFApi PUBLIC Common PRIVATE ChakraCore)