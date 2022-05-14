<template>
	<div class="boolSetting">
		<input type="checkbox" :name="settingId" />
		<label :for="settingId">{{ settingName }}</label>
	</div>
</template>

<script>
export default {
	setup() {},
	props: {
		settingName: String,
		settingId: String
	},
	mounted() {}
};
</script>


<style lang="sass" scoped>
</style>