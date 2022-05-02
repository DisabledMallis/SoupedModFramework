<script>
import LaunchButton from './components/LaunchButton.vue'

export default {
	components: {
		LaunchButton
	},
	data() {
		return {
			windowHeight: window.innerHeight,
			windowWidth: window.innerWidth,
		}
	},
	watch: {
        windowHeight(newHeight, oldHeight) {
            this.txt = `it changed to ${newHeight} from ${oldHeight}`;
        }
    },
	mounted() {
        this.$nextTick(() => {
            window.addEventListener('resize', this.onResize);
        })
    },
	beforeUnmount() { 
        window.removeEventListener('resize', this.onResize); 
    },
	methods: {  
        onResize() {
            this.windowHeight = window.innerHeight;
            this.windowWidth = window.innerWidth;
        }
    }
}
</script>

<template>
	<div v-if="windowWidth >= 500" class="content">
		<header>
			<h1>SoupedModFramework</h1>
		</header>
		<div class="body">
		</div>
		<footer>
			<LaunchButton />
		</footer>
	</div>
	<p v-else>Window too small</p>
</template>

<style>
@import url('https://fonts.googleapis.com/css2?family=Roboto:wght@100&display=swap');

html, body {
	margin:0px;
	height:100%;
	background-color: #131313;
	color: #FFFFFF;
	font-family: 'Roboto', sans-serif;
}

.content {
	position: absolute;
	width: 100vw;
	height: 100vh;
	background-color: #131313;
	color: #FFFFFF;
	display: grid;
	grid-template-rows: repeat(10, 1fr);
	grid-template-columns: repeat(3, 1fr);
}

header {
	grid-column: 2;
}

.body {
	grid-row: 2 / 8;
}

footer {
	grid-row: 9;
	grid-column: 2;
	display: grid;
}

h1 {
	text-align: center;
}
</style>
