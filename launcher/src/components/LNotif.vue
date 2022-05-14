<template>
	<div class="notif" :id="notifType">
		<p>{{ notifMessage }}</p>
	</div>
</template>

<style scoped>
#Error {
	border-color: red;
}
#Warn {
	border-color: yellow;
}
#Ok {
	border-color: green;
}

.notif {
	border-color: black;
	border-style: solid;
	border-width: 1px;
	border-radius: 5px;
	background-color: #0000007F;
	padding: 5px 5px 5px 5px;
	margin: 5px 5px 5px 5px;
	max-width: 25vw;
}
</style>

<script>
export default {
	setup() {},
	props: {
		notifType: String,
		notifMessage: String
	}
}
</script>