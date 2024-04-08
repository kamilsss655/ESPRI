import { defineStore } from "pinia";

export const useSystemStore = defineStore({
  id: "system",
  state: () => ({
    rebootRequired: false as Boolean,
    audio: "LISTENING"
  }),
  actions: {
    handle(event: any) {
      if (event.tag == "gAudioState") {
        this.$state.audio = event.message;
      }
    },
    resetRebootRequiredFlag() {
      this.$state.rebootRequired = false;
    },
    setRebootRequiredFlag() {
      this.$state.rebootRequired = true;
    }
  },
  getters: {
    audioListening: (state) => state.audio == "LISTENING",
    audioTransmitting: (state) => state.audio == "TRANSMITTING"
  }
});
