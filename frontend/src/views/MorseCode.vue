<template>
  <q-card flat bordered>
    <q-card-section>
      <q-form
        @submit="submitForm"
        @reset="settingsStore.fetchSettings"
        class="q-gutter-md"
      >
        <div class="text-h4 text-center">Morse code beacon</div>

        <q-item tag="label" v-ripple>
          <q-item-section>
            <q-item-label>Enabled</q-item-label>
          </q-item-section>
          <q-item-section avatar>
            <q-toggle
              color="blue"
              v-model="settingsStore['morse_code_beacon.enabled']"
              size="xl"
              :false-value="0"
              :true-value="1"
            />
          </q-item-section>
        </q-item>

        <q-input
          v-model="settingsStore['morse_code_beacon.text']"
          label="Text"
          hint="Define what Morse code message will be transmitted"
          v-if="isEnabled"
          filled
        />

        <q-field
          filled
          label="Period in seconds"
          :hint="'Define delay in seconds between messages'"
          v-if="isEnabled"
          clearable
        >
          <template v-slot:control>
            <q-slider
              v-model="settingsStore['morse_code_beacon.period_seconds']"
              :min="5"
              :max="7200"
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
import { computed, onMounted } from "vue";
import { useSettingsStore } from "../stores/settings";
import { useQuasar } from "quasar";

const settingsStore = useSettingsStore();
const $q = useQuasar();

onMounted(() => {
  settingsStore.fetchSettings();
});

const isEnabled = computed(() => {
  return settingsStore['morse_code_beacon.enabled'];
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
