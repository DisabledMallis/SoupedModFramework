add_library(SoupSDK SHARED main.cpp)

add_subdirectory(src)

target_link_libraries(SoupSDK PRIVATE Common)

# Add target to build dir
add_custom_command(
	TARGET SoupSDK POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/soupsdk/SoupSDK.dll
		${FINAL_DIR}/SoupSDK.dll
)