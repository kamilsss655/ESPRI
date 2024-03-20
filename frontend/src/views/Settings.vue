<template>
  <div class="q-pa-md">
    <q-form @submit="onSubmit" @reset="settingsStore.getSettings" class="q-gutter-md">
      <div class="text-h4 text-center">Settings</div>
      <q-input v-model="settingsStore.ssid" label="SSID" />
      <q-input v-model="settingsStore.password" label="Password" />

      <div>
        <q-btn label="Submit" type="submit" color="primary"/>
        <q-btn label="Reset" type="reset" color="primary" flat class="q-ml-sm" />
      </div>
    </q-form>
  </div>
</template>
<script setup lang="ts">
import { useSettingsStore } from "../stores/settings";
const settingsStore = useSettingsStore();
</script>
