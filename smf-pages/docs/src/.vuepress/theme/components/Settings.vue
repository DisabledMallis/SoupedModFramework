<script>
import LoadingSpinner from './LoadingSpinner.vue'
import BoolSetting from './BoolSetting.vue'

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
        LoadingSpinner,
        BoolSetting
    }
}
</script>

<template>
    <div class="settings">
        <BoolSetting settingName="Debug Mode" settingId="debug"/>
    </div>
</template>

<style scoped>
.settings {
    justify-items: center;
    align-items: center;
}
</style>