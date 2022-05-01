// vue.config.js
const { defineConfig } = require('@vue/cli-service')

module.exports = defineConfig({
	pages: {
		index: {
			entry: 'src/main.js',
			template: 'index.html',
			filename: 'index.html'
		},
		launcher: {
			entry: 'src/main.js',
			template: 'launcher.html',
			filename: 'launcher.html'
		}
	}
})