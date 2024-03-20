<template>
  <q-header elevated class="bg-primary text-white">
    <q-toolbar>
      <q-btn dense flat round @click="store.toggleLeftDrawer" icon="menu" />

      <q-btn flat to="/" no-wrap class="q-ml-xs" v-if="$q.screen.gt.xs">
        <q-toolbar-title shrink class="text-weight-bold">
          ESPRI
        </q-toolbar-title>
      </q-btn>

      <q-btn round dense flat to="/" icon="home" v-if="$q.screen.lt.sm">
        <q-tooltip>Home</q-tooltip>
      </q-btn>

      <q-space />

      <div class="q-gutter-sm row items-center no-wrap">
        <q-btn round dense flat to="/settings" icon="settings">
          <q-tooltip>Settings</q-tooltip>
        </q-btn>
        <q-btn round dense flat to="/about" icon="info">
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
