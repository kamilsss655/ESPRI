import { defineStore } from "pinia";
import { ApiPaths } from "../types/Api";
import { Notify } from "quasar";
import { SystemInfo } from "../types/System";
import axios from "axios";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

export const useSystemStore = defineStore({
  id: "system",
  state: () => ({
    rebootRequired: false as Boolean,
    audio: "LISTENING",
    info: {
      "heap.total": 160000,
      "heap.free": 80241,
      "heap.min_free": 10000,
      "storage.used": 723633,
      "storage.total": 2462561,
      uptime: 168,
      version: "v0.5.6-4"
    } as SystemInfo,
    systemInfoLiveUpdate: false as Boolean
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
    },
    async fetchSystemInfo() {
      try {
        const data = await axiosInstance.get(ApiPaths.SystemInfo);
        this.$state.info = data.data;
      } catch (error) {
        Notify.create({
          message: "Failed to fetch up-to date data.",
          color: "negative"
        });
        console.log(error);
      }
    }
  },
  getters: {
    audioListening: (state) => state.audio == "LISTENING",
    audioTransmitting: (state) => state.audio == "TRANSMITTING",
    heapUsagePercent: (state): number =>
      ((state.info["heap.total"] - state.info["heap.free"]) /
        state.info["heap.total"]) *
      100,
    heapMaxUsagePercent: (state): number =>
      ((state.info["heap.total"] - state.info["heap.min_free"]) /
        state.info["heap.total"]) *
      100,
    storageUsagePercent: (state): number =>
      (state.info["storage.used"] / state.info["storage.total"]) * 100
  }
});
