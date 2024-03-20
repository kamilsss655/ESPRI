<template>
  <q-drawer v-model="store.leftDrawerIsOpen" side="left" flat>
    <!-- drawer content -->
  </q-drawer>
</template>

<script setup lang="ts">
import { useNavigationStore } from '../../stores/navigation';
const store = useNavigationStore();
</script>

