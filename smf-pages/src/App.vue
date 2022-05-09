<script>
import LinkButton from './components/LinkButton.vue'

export default {
	components: {
		LinkButton
	},
}
</script>

<template>
	<div class="content">
		<header>
			<h1>SoupedModFramework</h1>
			<p>A Bloons TD Battles 2 modding framework with flexibility in mind</p>
		</header>
		<div class="body">
			<LinkButton link="https://discord.gg/nPcBPyHP4c" text="Discord" />
			<LinkButton link="https://github.com/DisabledMallis/SoupedModFramework" text="GitHub" />
		</div>
	</div>
</template>

<style>
@import url('https://fonts.googleapis.com/css2?family=Roboto:wght@100&display=swap');

html, body {
	margin:0px;
	height:100%;
}

.content {
	position: absolute;
	width: 100vw;
	height: 100vh;
	background-color: #131313;
	color: #FFFFFF;
	font-family: 'Roboto', sans-serif;
}

header {
	text-align: center;
}

.body {
	text-align: center;
}

.body * {
	margin-right: 5px;
	margin-left: 5px;
}

footer {
	display: grid;
}

h1 {
	font-size: 4rem;
}
p {
	font-size: 1.5rem;
}
</style>
