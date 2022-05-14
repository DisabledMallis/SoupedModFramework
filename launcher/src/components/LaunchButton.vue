<template>
	<VueSpinner v-if="loading" size="4em"/>
	<div v-else class="launchButton" v-on:click="launchClick()">
		<a>Launch</a>
	</div>
</template>

<script>
import { VueSpinner } from 'vue3-spinners'

const tauriApp = window.__TAURI__.app;
const fs = window.__TAURI__.fs;
const shell = window.__TAURI__.shell;

export default {
	setup() {
	},
	data() {
		return {
			loading: false
		}
	},
	components: {
		VueSpinner
	},
	methods: {
		async launchClick() {
			try {
				this.loading = true;
				await fs.renameFile("./proxies/wininet.dll",  "./wininet.dll");
				await shell.open("btdb2_game.exe");
			} catch {
				window.notify("Failed to launch, is the launcher inside of the game's folder?", "Error");
			}
		}
	},
	mounted() {
		this.tauriVersion = tauriApp.getTauriVersion();
	}
};
</script>


<style scoped>
.launchButton {
	border-style: solid;
	border-radius: 5px;
	padding: 15px 25px 15px 25px;
	transition: all 0.5s;
	cursor: pointer;
	user-select: none;
}
.launchButton:hover {
	color: var(--bg-color);
	background-color: var(--fg-color);
}
</style>