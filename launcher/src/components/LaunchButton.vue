<template>
	<div class="launchButton" v-on:click="launchClick()">
		<a>Launch</a>
	</div>
</template>

<script>
export default {
	setup() {},
	methods: {
		launchClick() {
			alert(window.__TAURI__);
		}
	},
	mounted() {}
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