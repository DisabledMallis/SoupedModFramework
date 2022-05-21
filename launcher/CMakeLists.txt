add_custom_target(Launcher ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	COMMAND npm install .
	COMMAND npm run tauri:build
)
add_custom_command(
	TARGET Launcher POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_LIST_DIR}/src-tauri/target/release/launcher.exe
		${FINAL_DIR}/launcher.exe
)