<script setup lang="ts">
import Banner from "./components/layout/Banner.vue";
import Header from "./components/layout/Header.vue";
import Drawer from "./components/layout/Drawer.vue";
import Footer from "./components/layout/Footer.vue";
import { useWebsocketStore } from "./stores/websocket";

import { onMounted } from "vue";
import { useQuasar } from "quasar";
const $q = useQuasar();
// set addressbarColor color to $primary color
$q.addressbarColor.set("");

const websocketStore = useWebsocketStore();

onMounted(() => {
  websocketStore.connect();
});
</script>

<template>
  <q-layout view="hHh LpR fFf">
    <Header />
    <Drawer />
    <q-page-container>
      <Banner />
      <div class="row">
        <div class="col-0 col-sm-3"></div>
        <div class="col-12 col-sm-6">
          <div class="q-pa-md">
            <RouterView />
          </div>
        </div>
      </div>
      <div class="col-0 col-sm-3"></div>
    </q-page-container>
    <Footer />
  </q-layout>
</template>
