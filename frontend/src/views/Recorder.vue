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

          <!-- Duration Label and Sliders -->
          <q-card-section>
            <div class="text-subtitle1">Duration</div>
            <!-- Seconds Slider -->
            <div class="slider-group q-mt-md">
              <q-slider
                v-model="durationStore.seconds"
                :min="0"
                :max="59"
                :step="1"
                markers
                snap
                label-always
                color="blue"
                @input="updateDuration"
                :hint="'Define record length in seconds'"
              ></q-slider>
              <q-badge class="q-ml-sm" color="secondary">Seconds</q-badge>
            </div>
            <!-- Minutes Slider -->
            <div class="slider-group q-mt-md">
              <q-slider
                v-model="durationStore.minutes"
                :min="0"
                :max="59"
                :step="1"
                markers
                snap
                label-always
                color="blue"
                @input="updateDuration"
                :hint="'Define record length in minutes'"
              ></q-slider>
              <q-badge class="q-ml-sm" color="secondary">Minutes</q-badge>
            </div>
            <!-- Hours Slider -->
            <div class="slider-group q-mt-md">
              <q-slider
                v-model="durationStore.hours"
                :min="0"
                :max="9"
                :step="1"
                markers
                snap
                label-always
                color="blue"
                @input="updateDuration"
                :hint="'Define record length in hours'"
              ></q-slider>
              <q-badge class="q-ml-sm" color="secondary">Hours</q-badge>
            </div>
          </q-card-section>

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
import { useDurationStore } from '../stores/duration';
import { FilesystemBasePath, StoragePath } from "../types/Filesystem";
import PathSelector from "../components/files/PathSelector.vue";
import { formatTimestamp } from "../helpers/Time";

// Store setup
const recorderStore = useRecorderStore();
const durationStore = useDurationStore();

// Timestamp formatting
const formattedTimeStamp = formatTimestamp(new Date(Date.now()));

// Storage path
const storagePath = ref<StoragePath>({
  prefix: FilesystemBasePath.SdCard,
  path: "/sample_" + formattedTimeStamp + ".wav",
});

// Function to update the total duration in seconds
const updateDuration = () => {
  const total = durationStore.seconds + durationStore.minutes * 60 + durationStore.hours * 3600;
  recorderStore.duration_sec = Math.min(total, 32767); // Clamp to max 32767 seconds
};

// Sync changes to store
function updateRecorderStore() {
  recorderStore.filepath = storagePath.value.prefix + storagePath.value.path;
}

// Watch for changes in slider values and update the total duration
watch(
  () => [durationStore.seconds, durationStore.minutes, durationStore.hours],
  () => {
    updateDuration();
  }
);

// On component mount, update store path
onMounted(() => {
  updateRecorderStore();
  durationStore.loadState(); // Load stored duration values from localStorage
  updateDuration(); // Ensure that the recorderStore is updated with the loaded values
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
