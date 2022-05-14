<template>
	<div class="appContainer">
		<div class="header">
			<h1>SoupedModFramework</h1>
		</div>
		<div class="content">
			<LSettings />
		</div>
		<div class="footer">
			<LaunchButton />
		</div>
		<div class="infoOverlay">
			<a>Launcher: {{ launcherVersion }}</a>
			<a>Tauri: {{ tauriVersion }}</a>
		</div>
		<div class="notifOverlay">
			<TransitionGroup name="list">
			<LNotif v-for="notif in notifications"
				:key="notif.key"
				:notifMessage="notif.message"
				:notifType="notif.type"/>
			</TransitionGroup>
		</div>
	</div>
</template>

<script>
import LSettings from "./components/LSettings.vue";
import LaunchButton from "./components/LaunchButton.vue";
import LNotif from "./components/LNotif.vue";
const tauriApp = window.__TAURI__.app;

export default {
	name: "App",
	data() {
		return {
			tauriVersion: "",
			launcherVersion: "pre-0.1",
			notifications: [],
			nextNotifKey: 0
		}
	},
	components: {
		LSettings,
		LaunchButton,
		LNotif
	},
	methods: {
		notify(text, ntype) {
			this.notifications.unshift({
				message: text,
				type: ntype,
				key: this.nextNotifKey
			});
			this.nextNotifKey++;
			setTimeout(()=>{
				this.notifications.pop();
			}, 5*1000);
		}
	},
	async mounted() {
		this.tauriVersion = await tauriApp.getTauriVersion();
		window.notify = this.notify;
	}
};
</script>

<style>
@import url("https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@500&display=swap");

:root {
	--border-color: #282828;
	--bg-color: #121212;
	--fg-color: #dfdfdf;
}

html {
	background-color: var(--border-color);
	font-family: "JetBrains Mono", monospace;
	overflow: hidden;
}
.appContainer {
	border-style: solid;
	border-color: black;
	border-width: 1px;
	width: calc(100vw - 20px);
	height: calc(100vh - 20px);
	box-shadow: inset 0 0 10px #000000;
	background-color: var(--bg-color);
	display: grid;
	grid-auto-rows: 5;
	grid-auto-columns: 5;
	color: var(--fg-color);
}
.header {
	display: grid;
	grid-row: 1;
	grid-column-start: 2;
	grid-column-end: 4;
	align-content: center;
	justify-content: center;
}
.content {
	display: grid;
	grid-row: 3;
	grid-column-start: 1;
	grid-column-end: 5;
	align-content: center;
	justify-content: center;
}
.footer {
	display: grid;
	grid-row: 5;
	grid-column-start: 2;
	grid-column-end: 4;
	align-content: center;
	justify-content: center;
}
.infoOverlay {
	position: absolute;
	display: grid;
	bottom: 20px;
	left: 20px;
}
.notifOverlay {
	position: absolute;
	display: grid;
	bottom: 20px;
	right: 20px;
	max-height: 90vh;
}

.list-move, /* apply transition to moving elements */
.list-enter-active,
.list-leave-active {
	transition: all 0.5s ease;
}

.list-enter-from,
.list-leave-to {
	opacity: 0;
	transform: translateX(20px);
}

/* ensure leaving items are taken out of layout flow so that moving
   animations can be calculated correctly. */
.list-leave-active {
	position: absolute;
}
</style>
