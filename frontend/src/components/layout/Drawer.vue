<template>
  <q-drawer v-model="store.leftDrawerIsOpen" side="left" flat bordered>
    <q-list>
        <q-item clickable to="/" rel="noopener">
          <q-item-section avatar>
            <q-icon name="ion-home" />
          </q-item-section>
          <q-item-section>
            <q-item-label>Home</q-item-label>
            <q-item-label caption>Main home page</q-item-label>
          </q-item-section>
        </q-item>
        <q-item clickable to="/morse_code" rel="noopener">
          <q-item-section avatar>
            <q-icon name="ion-flag" />
          </q-item-section>
          <q-item-section>
            <q-item-label>Morse code beacon</q-item-label>
            <q-item-label caption>Setup Morse code beacon</q-item-label>
          </q-item-section>
        </q-item>
        <q-item clickable to="/settings" rel="noopener">
          <q-item-section avatar>
            <q-icon name="ion-settings" />
          </q-item-section>
          <q-item-section>
            <q-item-label>Settings</q-item-label>
            <q-item-label caption>Manage settings</q-item-label>
          </q-item-section>
        </q-item>
        <q-item clickable to="/about" rel="noopener">
          <q-item-section avatar>
            <q-icon name="ion-information-circle" />
          </q-item-section>
          <q-item-section>
            <q-item-label>About</q-item-label>
            <q-item-label caption>More info about the project</q-item-label>
          </q-item-section>
        </q-item>
    </q-list>
  </q-drawer>
</template>

<script setup lang="ts">
import { useNavigationStore } from '../../stores/navigation';
const store = useNavigationStore();
</script>

