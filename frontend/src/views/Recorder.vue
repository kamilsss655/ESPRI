<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">Recorder</div>
      </q-card-section>
      <q-separator />
      <q-card-section>
        <q-form class="q-gutter-md" @submit="recorderStore.scheduleRecording">
          <PathSelector v-model="storagePath" filepath />

          <q-list bordered>
            <q-item>
              <div class="text-subtitle1">Duration</div>
            </q-item>

            <q-item>
              <q-item-section>
                <q-slider
                  v-model="recorderStore.duration_seconds"
                  :min="0"
                  :max="59"
                  :step="1"
                  markers
                  snap
                  label-always
                  color="primary"
                  :hint="'Define record length in seconds'"
                ></q-slider>
              </q-item-section>
              <q-item-section avatar> Seconds </q-item-section>
            </q-item>

            <q-item>
              <q-item-section>
                <q-slider
                  v-model="recorderStore.duration_minutes"
                  :min="0"
                  :max="59"
                  :step="1"
                  markers
                  snap
                  label-always
                  color="primary"
                  :hint="'Define record length in minutes'"
                ></q-slider>
              </q-item-section>
              <q-item-section avatar> Minutes </q-item-section>
            </q-item>

            <q-item>
              <q-item-section>
                <q-slider
                  v-model="recorderStore.duration_hours"
                  :min="0"
                  :max="9"
                  :step="1"
                  markers
                  snap
                  label-always
                  color="primary"
                  :hint="'Define record length in hours'"
                ></q-slider>
              </q-item-section>
              <q-item-section avatar> Hours </q-item-section>
            </q-item>
          </q-list>

          <div class="text-right q-pa-md">
            <q-btn
              icon="ion-play"
              label="Submit"
              type="submit"
              color="primary"
            />
          </div>
        </q-form>
      </q-card-section>
      <q-card-section>
        <q-banner inline-actions rounded class="bg-info text-white">
          When activated, the audio recorder will be on standby and will only
          record when the squelch is open. Please note that the audio record
          task will not persist device reboot or deep-sleep.
        </q-banner>
      </q-card-section>
    </q-card>
  </div>
</template>

<script setup lang="ts">
import { ref, watch, onMounted } from "vue";
import { useRecorderStore } from "../stores/recorder";
import { FilesystemBasePath, StoragePath } from "../types/Filesystem";
import PathSelector from "../components/files/PathSelector.vue";
import { formatTimestamp } from "../helpers/Time";

// Store setup
const recorderStore = useRecorderStore();

// Timestamp formatting
const formattedTimeStamp = formatTimestamp(new Date(Date.now()));

// Storage path
const storagePath = ref<StoragePath>({
  prefix: FilesystemBasePath.SdCard,
  path: "/sample_" + formattedTimeStamp + ".wav"
});

// Sync changes to store
function updateRecorderStore() {
  recorderStore.filepath = storagePath.value.prefix + storagePath.value.path;
}

// On component mount, update store path
onMounted(() => {
  updateRecorderStore();
});

// Watch for changes in the storage path and update the store
watch(
  () => [storagePath.value.prefix, storagePath.value.path],
  () => {
    updateRecorderStore();
  }
);
</script>

<style scoped>
.slider-group {
  display: flex;
  align-items: center;
  margin-bottom: 8px;
}
</style>
