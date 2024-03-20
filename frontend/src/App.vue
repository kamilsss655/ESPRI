<script setup lang="ts">
import Header from "./components/layout/Header.vue";
import Drawer from "./components/layout/Drawer.vue";
import Footer from "./components/layout/Footer.vue";
</script>

<template>
  <q-layout view="hHh LpR fFf">
    <Header />
    <Drawer />
    <q-page-container>
      <div class="q-pa-md">
        <RouterView />
      </div>
    </q-page-container>
    <Footer />
  </q-layout>
</template>
