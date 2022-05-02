add_library(Common SHARED
	"main.cpp"
)

target_include_directories(Common PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(Common PRIVATE fmt)

# Add target to build dir
add_custom_command(
	TARGET Common POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/common/${CMAKE_CONFIGURATION_TYPES}/Common.dll
		${FINAL_DIR}/Common.dll
)