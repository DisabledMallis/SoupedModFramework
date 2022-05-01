<script>
    export default {
        methods: {
            launch: () => {
                console.log("Preparing for launch");
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
        background-color: var(--vt-c-black);
        border: solid;
        border-width: 1px;
        border-radius: 5px;
        font-size: 2em;
        color: var(--vt-c-text-dark-1);
        cursor: pointer;
        transition: all .5s;
    }
    button:hover {
        background-color: var(--vt-c-text-dark-1);
        color: var(--vt-c-black);
    }
</style>