import axios from "axios";
import { ApiPaths } from "../types/Api";
import { useSystemStore } from "../stores/system";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;
export function activateAudioSend() {
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
            const systemStore = useSystemStore();
            systemStore.setAudioSending(true);
        })
        .catch((error) => {
            console.error(error);
        });      
    } catch (error) {
      console.error('Error sending audio command:', error);
    }
}

export async function startAudio() {
    console.log("Starting audio");
    const sampleRate = 8000; // Adjust the sample rate as needed
    const audioContext = new window.AudioContext({sampleRate:sampleRate});
    
    try {
      await audioContext.audioWorklet.addModule('/pcm-processor.js');
    } catch (error) {
      console.error('Error adding audio worklet module:', error);
      return;
    }

    window.pcmProcessor = new AudioWorkletNode(audioContext, 'pcm-processor');
    window.pcmProcessor.port.onmessage = (event: { data: string; }) => {
      if (event.data === 'request') {
        if (window.pcmBuffer.length > 0) {
          window.pcmProcessor.port.postMessage(window.pcmBuffer.shift());
        }
      }
    };

    window.pcmProcessor.connect(audioContext.destination);
    console.log("Started audio");
    activateAudioSend();
}