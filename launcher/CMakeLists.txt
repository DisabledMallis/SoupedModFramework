add_custom_target(Launcher ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	COMMAND npm install -g @neutralinojs/neu
	COMMAND npm install .
        COMMAND neu update
	COMMAND neu build --release
)
add_custom_command(
	TARGET Launcher POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_LIST_DIR}/dist/launcher/launcher-win_x64.exe
		${FINAL_DIR}/Launcher.exe
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_LIST_DIR}/dist/launcher/WebView2Loader.dll
		${FINAL_DIR}/WebView2Loader.dll
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_LIST_DIR}/dist/launcher/resources.neu
		${FINAL_DIR}/resources.neu
)
