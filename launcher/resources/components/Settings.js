import BoolSetting from './BoolSetting.js'

export default {
    data() {
        return {
            isLoading: false,
        }
    },
    methods: {
    },
    components: {
        BoolSetting
    },
	template: `
		<div class="settings">
			<BoolSetting settingName="Debug Mode" settingId="debug"/>
		</div>
	`
}