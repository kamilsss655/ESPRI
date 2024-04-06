<template>
  <q-card flat bordered>
    <q-card-section>
      <q-form
        @submit="submitForm"
        @reset="settingsStore.fetchSettings"
        class="q-gutter-md"
      >
        <div class="text-h4 text-center">Settings</div>
        <q-select
          filled
          v-model="settingsStore['wifi.mode']"
          :options="wifiModeOptions"
          label="Wifi mode"
          behavior="dialog"
          emit-value
          map-options
        >
          <template v-slot:option="scope">
            <q-item v-bind="scope.itemProps">
              <q-item-section avatar>
                <q-icon :name="scope.opt.icon" />
              </q-item-section>
              <q-item-section>
                <q-item-label>{{ scope.opt.label }}</q-item-label>
                <q-item-label caption>{{ scope.opt.description }}</q-item-label>
              </q-item-section>
            </q-item>
          </template>
        </q-select>

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

        <q-select
          v-model="settingsStore['wifi.channel']"
          :options="wifiChannelOptions"
          v-if="settingsStore['wifi.mode'] == 1"
          behavior="dialog"
          label="Wifi channel"
          filled
        />

        <q-select
          v-model.number="settingsStore['gpio.status_led']"
          :options="gpioOptions"
          behavior="dialog"
          label="Status LED"
          filled
        />

        <q-select
        v-model.number="settingsStore['gpio.audio_in']"
          :options="gpioOptions"
          behavior="dialog"
          label="Audio IN"
          filled
        />

        <q-select
        v-model.number="settingsStore['gpio.audio_out']"
          :options="gpioOptions"
          behavior="dialog"
          label="Audio OUT"
          filled
        />

        <q-select
        v-model.number="settingsStore['gpio.ptt']"
          :options="gpioOptions"
          behavior="dialog"
          label="PTT Pin"
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
import { useQuasar } from "quasar";
import { useSettingsStore } from "../stores/settings";
import { WifiMode } from "../types/Settings"

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

const wifiModeOptions = ref([
  {
    label: "Off",
    value: WifiMode.OFF,
    description: "Turns WiFi off.",
    icon: "ion-airplane"
  },
  {
    label: "Access point",
    value: WifiMode.AP,
    description: "Acts as a WiFi router.",
    icon: "ion-wifi"
  },
  {
    label: "Station mode",
    value: WifiMode.STA,
    description: "Connect to your network.",
    icon: "ion-wifi"
  }
]);

const wifiChannelOptions = ref([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13]);
const gpioOptions = ref(Array.from({ length: 36 }, (_v, i) => i));
</script>
