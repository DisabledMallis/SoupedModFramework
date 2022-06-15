add_library(steam_api64 SHARED)

target_sources(steam_api64 PRIVATE
	"main.cpp"
	"steam.cpp"
)

target_link_libraries(steam_api64 PRIVATE fmt)

# Add target to build dir
add_custom_command(
	TARGET steam_api64 POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/proxy/steam_api64.dll
		${FINAL_DIR}/steam_api64.dll
)