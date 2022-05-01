<script>
    export default {
        methods: {
            launch: () => {
                console.log("Preparing for launch");
				// eslint-disable-next-line
                nativeLaunch();
            }
        }
    }
</script>

<template>
    <button @click="launch">Launch</button>
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