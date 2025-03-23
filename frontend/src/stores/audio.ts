
import { defineStore } from "pinia";
import { Audio } from "../types/Audio";

export const useAudioStore = defineStore({
    id: "audio",
    state: (): Audio => ({
      audioLevel: 0
    }),
    actions: {
      setAudioLevel(value: number) {
        this.$state.audioLevel = value;
      },
    },
    getters: {}
  });
  