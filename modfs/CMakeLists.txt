add_library(ModFS SHARED)

target_include_directories(ModFS PUBLIC "include")

target_sources(ModFS PRIVATE
	"modfs.cpp"
)

# Add target to build dir
add_custom_command(
	TARGET ModFS POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/modfs/ModFS.dll
		${FINAL_DIR}/ModFS.dll
)