import { defineStore } from "pinia";

export const useSystemStore = defineStore({
  id: "system",
  state: () => ({
    audio: "LISTENING"
  }),
  actions: {
    handle(event: any) {
      if (event.tag == "gAudioState") {
        this.$state.audio = event.message;
      }
    }
  },
  getters: {
    audioListening: (state) => state.audio == "LISTENING",
    audioTransmitting: (state) => state.audio == "TRANSMITTING"
  }
});
