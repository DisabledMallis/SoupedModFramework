add_library(ModFS SHARED)

target_sources(ModFS PRIVATE
	"modfs.cpp"
)

target_link_libraries(ModFS PRIVATE Common)

# Add target to build dir
add_custom_command(
	TARGET ModFS POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/modfs/${CMAKE_BUILD_TYPE}/ModFS.dll
		${FINAL_DIR}/ModFS.dll
)