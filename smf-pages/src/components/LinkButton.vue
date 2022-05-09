<script>
export default {
    data() {
        return {
            isLoading: false,
        }
    },
	methods: {
		openLink() {
			window.open(this.link, '_blank');
		}
	},
    components: {},
	props: ['link', 'text']
}
</script>

<template>
    <button @click="openLink()">{{ text }}</button>
</template>

<style scoped>
    button {
        background-color: #131313;
        border: solid;
        border-width: 1px;
        border-radius: 5px;
        font-size: 2em;
        color: #FFFFFF;
        cursor: pointer;
        transition: all .5s;
    }
    button:hover {
        background-color: #FFFFFF;
        color: #131313;
    }
</style>