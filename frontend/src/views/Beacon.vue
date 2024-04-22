<template>
  <q-card flat bordered>
    <q-card-section>
      <div class="text-h4 text-center">Beacon</div>
    </q-card-section>
    <q-separator />
    <q-card-section>
      <q-form
        @submit="submitForm"
        @reset="settingsStore.fetchSettings"
        class="q-gutter-md"
      >
        <q-select
          filled
          v-model="settingsStore['beacon.mode']"
          :options="beaconModeOptions"
          label="Beacon mode"
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

        <q-input
          v-model="settingsStore['beacon.text']"
          label="Text"
          hint="Define what message will be transmitted"
          v-if="beaconMode != BeaconMode.OFF"
          filled
        />

        <q-field
          filled
          label="Delay in seconds"
          :hint="'Define delay in seconds between messages'"
          v-if="beaconMode != BeaconMode.OFF"
          clearable
        >
          <template v-slot:control>
            <q-slider
              v-model="settingsStore['beacon.delay_seconds']"
              :min="5"
              :max="7200"
              :step="5"
              label
              label-always
              class="q-mt-lg"
            />
          </template>
        </q-field>

        <q-field
          filled
          label="Baud"
          :hint="'Define baud for Morse code'"
          v-if="beaconMode == BeaconMode.MORSE_CODE"
          clearable
        >
          <template v-slot:control>
            <q-slider
              v-model="settingsStore['beacon.morse_code.baud']"
              :min="1"
              :max="200"
              label
              label-always
              class="q-mt-lg"
            />
          </template>
        </q-field>

        <q-field
          filled
          label="Baud"
          :hint="'Define baud for AFSK'"
          v-if="beaconMode == BeaconMode.AFSK"
          clearable
        >
          <template v-slot:control>
            <q-slider
              v-model="settingsStore['beacon.afsk.baud']"
              :min="50"
              :max="1200"
              :step="50"
              label
              label-always
              class="q-mt-lg"
            />
          </template>
        </q-field>

        <q-field
          filled
          label="Zero tone frequency"
          :hint="'Define tone frequency in Hz representing digital zero (space frequency)'"
          v-if="beaconMode == BeaconMode.AFSK"
          clearable
        >
          <template v-slot:control>
            <q-slider
              v-model="settingsStore['beacon.afsk.zero_freq']"
              :min="300"
              :max="4000"
              :step="50"
              label
              label-always
              class="q-mt-lg"
            />
          </template>
        </q-field>

        <q-field
          filled
          label="One tone frequency"
          :hint="'Define tone frequency in Hz representing digital one (mark frequency)'"
          v-if="beaconMode == BeaconMode.AFSK"
          clearable
        >
          <template v-slot:control>
            <q-slider
              v-model="settingsStore['beacon.afsk.one_freq']"
              :min="300"
              :max="4000"
              :step="50"
              label
              label-always
              class="q-mt-lg"
            />
          </template>
        </q-field>

        <q-field
          filled
          label="Tone frequency"
          :hint="'Define tone frequency in Hz'"
          v-if="beaconMode == BeaconMode.MORSE_CODE"
          clearable
        >
          <template v-slot:control>
            <q-slider
              v-model="settingsStore['beacon.morse_code.tone_freq']"
              :min="500"
              :max="4000"
              :step="100"
              label
              label-always
              class="q-mt-lg"
            />
          </template>
        </q-field>

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
import { computed, onMounted, ref } from "vue";
import { useSettingsStore } from "../stores/settings";
import { BeaconMode } from "../types/Settings";

const settingsStore = useSettingsStore();

onMounted(() => {
  settingsStore.fetchSettings();
});

const beaconMode = computed(() => {
  return settingsStore["beacon.mode"];
});

function submitForm() {
  settingsStore.updateSettings();
}

const beaconModeOptions = ref([
  {
    label: "Off",
    value: BeaconMode.OFF,
    description: "Turns beacon off.",
    icon: "ion-airplane"
  },
  {
    label: "Morse code",
    value: BeaconMode.MORSE_CODE,
    description: "Use Morse code.",
    icon: "ion-wifi"
  },
  {
    label: "AFSK",
    value: BeaconMode.AFSK,
    description: "Use AFSK modulation.",
    icon: "ion-wifi"
  },
  {
    label: "WAV",
    value: BeaconMode.WAV,
    description: "Play .wav audio file",
    icon: "ion-musical-notes"
  }
]);
</script>
