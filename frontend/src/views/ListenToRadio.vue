<script setup lang="ts">
import { startAudio } from "../helpers/Audio";
import { useSystemStore } from "../stores/system";
import { useAudioStore } from "../stores/audio";
const systemStore = useSystemStore();
const audioStore = useAudioStore();


</script>
<template>
  <div>
    <q-card flat bordered>
      <q-card-section>    
        <div class="text-h4 text-center">Listen to the radio</div>
      </q-card-section>
      <q-separator />
      <q-card-section>
        <div class="text-right q-pa-md">
            <q-btn
              icon="ion-play"
              label="Start receiving audio"
              type="button"
              color="primary"
              @click="startAudio()"
            />
          </div>
        <!-- :value="progress2" -->
        <q-linear-progress label="Audio volume" stripe rounded size="20px" :value="audioStore.audioLevel" color="primary" class="q-mt-sm" />
        
        <q-toggle v-model="systemStore.audioRecordingActive" label="Activate recording" />
        <!-- <q-toggle v-model="systemStore.audioSending" label="Audio send active" />
         -->
      </q-card-section>
      <q-card-section>
        <q-banner inline-actions rounded class="bg-info text-white">
          When activated, the audio will be sent here if the squelch is open.
        </q-banner>
      </q-card-section>
    </q-card>
  </div>
</template>