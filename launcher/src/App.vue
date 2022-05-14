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
	</div>
</template>

<script>
import LSettings from "./components/LSettings.vue";
import LaunchButton from "./components/LaunchButton.vue";

export default {
	name: "App",
	components: {
		LSettings,
		LaunchButton
	}
};
</script>

<style>
@import url("https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@500&display=swap");

html {
	background-color: #282828;
	font-family: "JetBrains Mono", monospace;
}
.appContainer {
	border-style: solid;
	border-color: black;
	border-width: 1px;
	width: calc(100vw - 20px);
	height: calc(100vh - 20px);
	box-shadow: inset 0 0 10px #000000;
	background-color: #121212;
	display: grid;
	grid-auto-rows: 5;
	grid-auto-columns: 5;
	color: #dfdfdf;
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
</style>
