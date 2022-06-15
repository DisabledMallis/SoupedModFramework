add_library(SoupSDK SHARED)

add_subdirectory(src)

target_link_libraries(SoupSDK PRIVATE Common PolyHook_2 fmt)

# Add target to build dir
add_custom_command(
	TARGET SoupSDK POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/soupsdk/${CMAKE_BUILD_TYPE}/SoupSDK.dll
		${FINAL_DIR}/SoupSDK.dll
)