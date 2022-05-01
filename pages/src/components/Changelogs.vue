<script>
    import Changelog from './Changelog.vue'

    export default {
        data() {
            return {
                versions: [
                    { ver: "1.0", desc: "Cool update" },
                    { ver: "test", desc: "Pretty epic"}
                ]
            }
        }
    }
</script>

<template>
    <div class="changelogContainer">
        <Changelog
            v-for="ver in versions"
            :num="ver.ver"
            :desc="ver.desc"
        />
    </div>
</template>

<style scoped>
    .changelogContainer {
        border: solid;
        border-color: black;
        border-width: 2px;
        border-radius: 5px;
        margin: 2em 2em 2em 2em;
        overflow-y: scroll;
        max-height: calc(100vh - 12em);
    }
</style>