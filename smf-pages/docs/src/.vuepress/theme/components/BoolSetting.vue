<script>
export default {
    methods: {
        get() {
            try {
                return nativeGetOption(settingId);
            } catch {
                return false;
            }
        },
        set(value) {
            nativeSetOption(settingId, event.target.checked);
        }
    },
    props: ["settingName", "settingId"]
}
</script>

<template>
    <div class="toggle">
        <input class="checkbox" type="checkbox" ::name="settingId" value="get()" v-on:click="set">
        <label ::for="settingId">{{ settingName }}</label><br>
    </div>
</template>

<style scoped>
.toggle {
    display: inline-flex;
    align-items: center;
}

.checkbox {
    -webkit-appearance: none;
	background-color: #131313;
	border: 1px solid #cacece;
	padding: 9px;
	border-radius: 3px;
    transition: all .25s;
}
.checkbox:checked {
	background-color: #fafafa;
}
</style>