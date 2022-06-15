add_executable(ModExplore)

target_sources(ModExplore PRIVATE
	"main.cpp"
)

target_link_libraries(ModExplore PRIVATE ModFS Common)

# Add target to build dir
add_custom_command(
	TARGET ModExplore POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/modexplore/${CMAKE_BUILD_TYPE}/ModExplore.exe
		${FINAL_DIR}/ModExplore.exe
)