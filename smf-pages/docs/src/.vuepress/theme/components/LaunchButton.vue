<script>
import LoadingSpinner from './LoadingSpinner.vue'

export default {
    data() {
        return {
            isLoading: false,
        }
    },
    methods: {
        launch() {
            console.log("Preparing for launch");
            // eslint-disable-next-line
            if(nativeLaunch()) {
                this.isLoading = true;
            }
        }
    },
    components: {
        LoadingSpinner
    }
}
</script>

<template>
    <LoadingSpinner v-if="isLoading"/>
    <button v-else @click="launch">Launch</button>
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