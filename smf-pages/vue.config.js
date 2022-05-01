const { defineConfig } = require('@vue/cli-service')
module.exports = defineConfig({
	transpileDependencies: true,
	pages: {
		index: {
			entry: 'src/main.js',
			template: 'public/index.html',
			filename: 'index.html',
			title: 'SMF - Index'
		},
		launcher: {
			entry: 'src/launcher.js',
			template: 'public/index.html',
			filename: 'launcher.html',
			title: 'SoupedModFramework Launcher'
		}
	}
})
