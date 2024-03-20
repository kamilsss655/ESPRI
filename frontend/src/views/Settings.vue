<template>
  <q-card flat bordered>
    <q-card-section>
      <q-form
        @submit="submitForm"
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
        <div class="text-right q-pa-md">
          <q-btn icon="ion-play" label="Submit" type="submit" color="primary"/>
          <q-btn icon="ion-refresh" label="Reload" class="q-ml-sm" outline/>
        </div>
      </q-form>
    </q-card-section>
  </q-card>
</template>

<script setup lang="ts">
import { onMounted } from "vue";
import { useSettingsStore } from "../stores/settings";
import { useQuasar } from 'quasar'

const settingsStore = useSettingsStore();
const $q = useQuasar()

onMounted(() => {
  settingsStore.fetchSettings();
});

function submitForm () {
      $q.dialog({
        title: 'Confirm',
        message: 'This will restart the device.',
        cancel: true,
        persistent: true
      }).onOk(() => {
        settingsStore.updateSettings()
      })
    }
</script>
