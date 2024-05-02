<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">Recorder</div>
      </q-card-section>
      <q-separator />
      <q-card-section>
        <q-form class="q-gutter-md" @submit="recorderStore.scheduleRecording">
          <q-input
            v-model="recorderStore.filepath"
            label="Filepath"
            hint="Define filepath for the file"
            filled
          />

          <q-field
            filled
            label="Max duration in ms"
            :hint="'Define max duration of the recording in ms'"
            clearable
          >
            <template v-slot:control>
              <q-slider
                v-model="recorderStore.max_duration_ms"
                :min="1000"
                :max="10000"
                :step="500"
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
import { useRecorderStore } from "../stores/recorder";

const recorderStore = useRecorderStore();
</script>
