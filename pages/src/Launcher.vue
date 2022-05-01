<script setup>
    import Launch from './components/Launch.vue'
    import Changelogs from './components/Changelogs.vue'
</script>

<template>
    <header>
        <h1>SMF Launcher</h1>
    </header>
    <news>
        <Changelogs/>
    </news>
    <footer>
        <Launch />
    </footer>
</template>

<style>
    @import './assets/base.css';

    h1 {
        color: var(--vt-c-text-dark-1)
    }
    header * {
        text-align: center;
    }
    news {
        height: 100%;
    }
    footer * {
        display: grid;
        margin: auto;
    }
</style>
