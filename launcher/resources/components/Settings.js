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
			<BoolSetting settingName="Unlock all towers (and heroes)" settingId="unlockTowers"/>
			<BoolSetting settingName="Unlock all upgrades" settingId="unlockUpgrades"/>
			<BoolSetting settingName="Dump Assets" settingId="dump"/>
		</div>
	`
}