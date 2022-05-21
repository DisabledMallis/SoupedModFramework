export default {
    props: ["settingName", "settingId"],
	data() {
		return {
			innerValue: false
		}
	},
    methods: {
        async get() {
            try {
                this.innerValue = await window.nativeGetOption(this.settingId);
				console.log("VAL: "+ this.innerValue);
            } catch(ex) {
				console.log(ex.message);
                return;
            }
        },
        async set(value) {
            await window.nativeSetOption(this.settingId, event.target.checked);
			this.innerValue = event.target.checked;
        }
    },
	beforeMount() {
		this.get();
	},
	template: `
		<div class="toggle">
			<input class="checkbox" type="checkbox" ::name="settingId" v-bind:checked="innerValue" v-on:click="set">
			<label ::for="settingId">{{ settingName }}</label><br>
		</div>
	`
}