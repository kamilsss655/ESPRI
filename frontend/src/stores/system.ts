import { defineStore } from "pinia";
import { SystemInfo } from "../types/System";

export const useSystemStore = defineStore({
  id: "system",
  state: () => ({
    rebootRequired: false as Boolean,
    audio: "LISTENING",
    info: {
      "heap.total": 160000,
      "heap.free": 172740,
      "heap.min_free": 168540,
      "storage.free": 723633,
      "storage.total": 2462561,
      uptime: 168,
      version: "v0.5.6-4"
    } as SystemInfo
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
