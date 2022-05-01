<script>
    export default {
        props: ['num', 'desc']
    }
</script>

<template>
    <div class="change">
        <h1> {{ num }}</h1>
        <p>{{ desc }}</p>
    </div>
</template>
<style scoped>
    .change {
        margin: 12px 24px 12px 24px;
    }
</style>

