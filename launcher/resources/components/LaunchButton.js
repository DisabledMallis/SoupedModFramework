import LoadingSpinner from './LoadingSpinner.js'

export default {
    data() {
        return {
            isLoading: false,
        }
    },
    methods: {
        launch() {
            console.log("Preparing for launch");
			this.isLoading = true;
            // eslint-disable-next-line
            if(!window.nativeLaunch()) {
                this.isLoading = false;
            }
        }
    },
    components: {
        LoadingSpinner
    },
	template: `
		<LoadingSpinner v-if="isLoading"/>
		<button v-else @click="launch">Launch</button>
	`
}