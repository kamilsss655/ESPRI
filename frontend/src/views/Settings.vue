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
          hint="0 for OFF, 1 for AP, 2 for STA"
          filled
        />
        <q-input v-model="settingsStore['wifi.ssid']" label="SSID" filled />

        <q-input
          v-model="settingsStore['wifi.password']"
          label="Password"
          filled
          :type="hideWifiPassword ? 'password' : 'text'"
        >
          <template v-slot:append>
            <q-icon
              :name="hideWifiPassword ? 'ion-eye' : 'ion-eye-off'"
              class="cursor-pointer"
              @click="hideWifiPassword = !hideWifiPassword"
            />
          </template>
        </q-input>

        <q-input
          v-model.number="settingsStore['wifi.channel']"
          type="number"
          label="Channel"
          filled
        />
        <q-input
          v-model.number="settingsStore['gpio.status_led']"
          type="number"
          label="Status LED"
          filled
        />
        <q-input
          v-model.number="settingsStore['gpio.audio_out']"
          type="number"
          label="Audio OUT"
          filled
        />
        <div class="text-right q-pa-md">
          <q-btn icon="ion-play" label="Submit" type="submit" color="primary" />
          <q-btn
            icon="ion-refresh"
            label="Reload"
            type="reset"
            class="q-ml-sm"
            outline
          />
        </div>
      </q-form>
    </q-card-section>
  </q-card>
</template>

<script setup lang="ts">
import { ref, onMounted } from "vue";
import { useSettingsStore } from "../stores/settings";
import { useQuasar } from "quasar";

const hideWifiPassword = ref(true);
const settingsStore = useSettingsStore();
const $q = useQuasar();

onMounted(() => {
  settingsStore.fetchSettings();
});

function submitForm() {
  $q.dialog({
    title: "Confirm",
    message: "This will restart the device.",
    cancel: true,
    persistent: true
  }).onOk(() => {
    settingsStore.updateSettings();
  });
}
</script>
