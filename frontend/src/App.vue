<script setup lang="ts">
import HelloWorld from "./components/HelloWorld.vue";
import Header from "./components/layout/Header.vue";
import Drawer from "./components/layout/Drawer.vue";
import Footer from "./components/layout/Footer.vue";
</script>

<template>
  <q-layout view="hHh LpR fFf">
    <Header />
    <Drawer />
    <q-page-container>
      <router-view />
    </q-page-container>
    <Footer />
  </q-layout>
</template>
