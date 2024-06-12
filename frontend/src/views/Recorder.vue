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
            label="Max duration in ms"
            :hint="'Define max duration of the recording in ms'"
            clearable
          >
            <template v-slot:control>
              <q-slider
                v-model="recorderStore.max_duration_ms"
                :min="1000"
                :max="30000"
                :step="1000"
                label
                label-always
                class="q-mt-lg"
              />
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
    </q-card>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref, watch } from "vue";
import { useRecorderStore } from "../stores/recorder";
import { FilesystemBasePath, StoragePath } from "../types/Filesystem";
import PathSelector from "../components/files/PathSelector.vue";

const recorderStore = useRecorderStore();
const storagePath = ref<StoragePath>({
  prefix: FilesystemBasePath.SdCard,
  path: "/sample.wav"
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
