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

          <q-field
            borderless
            clearable
            label="Duration in seconds"
            >
            <template v-slot:control>
            
              <q-input dark 
                  v-model.number="recorderStore.duration_sec"
                  inputmode="numeric"
                  input-style="text-align: right"           
                  mask="#####"
                  step=10
                  type="number"
                  style="max-width: 100px"
                  suffix="sec"
                  :rules="[ val => val <= 32767 || 'Maximum size is 32767 seconds']">
                </q-input>

                <q-slider
                  v-model="recorderStore.duration_sec"
                  :min="10"
                  :max="600"
                  :step="10"
                  snap
                  label
                  color="blue"
                  :hint="'Define record length in seconds'"  
              ></q-slider>
            </template>


          </q-field>

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
          When activated, the audio recorder will be on standby and will only record when the squelch is open. Please note that the audio record task will not persist device reboot or deep-sleep.
        </q-banner>
      </q-card-section>
    </q-card>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref, watch } from "vue";
import { useRecorderStore } from "../stores/recorder";
import { FilesystemBasePath, StoragePath } from "../types/Filesystem";
import PathSelector from "../components/files/PathSelector.vue";
import { date } from 'quasar'

const timeStamp = Date.now()
const formattedTimeStamp = date.formatDate(timeStamp, 'YYYYMMDD_HHmmss')

const recorderStore = useRecorderStore();
const storagePath = ref<StoragePath>({
  prefix: FilesystemBasePath.SdCard,
  path: "/sample_" + formattedTimeStamp + ".wav"
});

// sync changes to store
function updateRecorderStore() {
  recorderStore.filepath = storagePath.value.prefix + storagePath.value.path;
}

onMounted(() => {
  updateRecorderStore();
});

watch(
  () => [storagePath.value.prefix, storagePath.value.path],
  ([_prefix, _path]) => {
    updateRecorderStore();
  }
);
</script>
