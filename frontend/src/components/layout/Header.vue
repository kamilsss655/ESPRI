<template>
  <q-header flat class="bg-primary text-white">
    <q-toolbar>
      <q-btn dense flat round @click="store.toggleLeftDrawer" icon="ion-menu" />

      <q-btn flat to="/" no-wrap class="q-ml-xs" v-if="$q.screen.gt.xs">
        <q-toolbar-title shrink class="text-weight-bold">
          ESPRI
        </q-toolbar-title>
      </q-btn>

      <q-btn round dense flat to="/" icon="ion-home" v-if="$q.screen.lt.sm">
        <q-tooltip>Home</q-tooltip>
      </q-btn>

      <q-space />

      <div class="q-gutter-sm row items-center no-wrap">
        <q-btn round dense flat to="/settings" icon="ion-settings">
          <q-tooltip>Settings</q-tooltip>
        </q-btn>
        <q-btn round dense flat to="/about" icon="ion-information-circle">
          <q-tooltip>About</q-tooltip>
        </q-btn>
      </div>
    </q-toolbar>
  </q-header>
</template>

<script setup lang="ts">
import { useNavigationStore } from "../../stores/navigation";
const store = useNavigationStore();
</script>
