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
              @click="startAudio"
            />
          </div>
        <q-linear-progress label="Audio volume" stripe rounded size="20px" :value="progress2" color="primary" class="q-mt-sm" />
        <q-toggle v-model="recordingActive" label="Activate recording" />
      </q-card-section>
      <q-card-section>
        <q-banner inline-actions rounded class="bg-info text-white">
          When activated, the audio will be sent here if the squelch is open.
        </q-banner>
      </q-card-section>
    </q-card>
  </div>
</template>

<script>
import axios from "axios";
import { ApiPaths } from "../types/Api";
export default {
  data() {
    return {
      recordingActive: false
    };
  },
  methods: {
    async startAudio() {
      console.log("Starting audio");
      const sampleRate = 32000; // Adjust the sample rate as needed
      const audioContext = new (window.AudioContext || window.webkitAudioContext)({sampleRate:sampleRate});
      
      try {
        await audioContext.audioWorklet.addModule('/pcm-processor.js');
      } catch (error) {
        console.error('Error adding audio worklet module:', error);
        return;
      }

      window.pcmProcessor = new AudioWorkletNode(audioContext, 'pcm-processor');
      window.pcmProcessor.port.onmessage = (event) => {
        if (event.data === 'request') {
          if (window.pcmBuffer.length > 0) {
            window.pcmProcessor.port.postMessage(window.pcmBuffer.shift());
          }
        }
      };

      window.pcmProcessor.connect(audioContext.destination);
      console.log("Started audio");

      console.log("Sending command");
      try {
        const jsonData = "{}";
        const axiosInstance = axios.create();
        axiosInstance.defaults.timeout = 600;
        axiosInstance
          .put(ApiPaths.AudioSend, jsonData, {
            headers: {
              "Content-Type": "application/json"
            }
          })
          .then((response) => {
            console.log(response);
          })
          .catch((error) => {
            console.error(error);
          });
          
        } catch (error) {
          console.error('Error sending audio command:', error);
        }

    }
  }

};
</script>