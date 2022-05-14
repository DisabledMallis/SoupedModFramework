add_custom_target(Launcher ALL
	COMMAND npm install @vue/cli
	COMMAND npm install @vue/cli-service
	COMMAND npm install vue-template-compiler
	COMMAND npm run vue add tauri
	COMMAND npm run tauri:build
)