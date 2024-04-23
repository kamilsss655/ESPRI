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
        <q-item clickable to="/beacon" rel="noopener">
          <q-item-section avatar>
            <q-icon name="ion-flag" />
          </q-item-section>
          <q-item-section>
            <q-item-label>Beacon</q-item-label>
            <q-item-label caption>Setup beacon</q-item-label>
          </q-item-section>
        </q-item>
        <q-item clickable to="/files" rel="noopener">
          <q-item-section avatar>
            <q-icon name="ion-folder" />
          </q-item-section>
          <q-item-section>
            <q-item-label>Files</q-item-label>
            <q-item-label caption>Manage files</q-item-label>
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
            <q-item-label caption>Get more details</q-item-label>
          </q-item-section>
        </q-item>
    </q-list>
  </q-drawer>
</template>

<script setup lang="ts">
import { useNavigationStore } from '../../stores/navigation';
const store = useNavigationStore();
</script>

