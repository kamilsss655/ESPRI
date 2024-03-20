<template>
  <div class="q-pa-md">
    <q-form
      @submit="settingsStore.updateSettings"
      @reset="settingsStore.fetchSettings"
      class="q-gutter-md"
    >
      <div class="text-h4 text-center">Settings</div>

      <q-input
        v-model.number="settingsStore['wifi.mode']"
        type="number"
        label="Mode"
      />
      <q-input v-model="settingsStore['wifi.ssid']" label="SSID" />
      <q-input v-model="settingsStore['wifi.password']" label="Password" />
      <q-input
        v-model.number="settingsStore['wifi.channel']"
        type="number"
        label="Channel"
      />
      <q-input
        v-model.number="settingsStore['gpio.status_led']"
        type="number"
        label="Status LED"
      />
      <q-input
        v-model.number="settingsStore['gpio.audio_out']"
        type="number"
        label="Audio OUT"
      />

      <div>
        <q-btn label="Submit" type="submit" color="primary" />
        <q-btn
          label="Reset"
          type="reset"
          color="primary"
          flat
          class="q-ml-sm"
        />
      </div>
    </q-form>
  </div>
</template>

<script setup lang="ts">
import { onMounted } from "vue";
import { useSettingsStore } from "../stores/settings";
const settingsStore = useSettingsStore();

onMounted(() => {
  settingsStore.fetchSettings();
});
</script>
