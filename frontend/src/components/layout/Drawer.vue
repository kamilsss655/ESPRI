<template>
  <q-drawer v-model="store.leftDrawerIsOpen" side="left" overlay elevated>
    <!-- drawer content -->
  </q-drawer>
</template>

<script setup lang="ts">
import { useNavigationStore } from '../../stores/navigation';
const store = useNavigationStore();
</script>

