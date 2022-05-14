add_custom_target(Launcher ALL
	COMMAND npm install @vue/cli
	COMMAND npm run vue add tauri
)