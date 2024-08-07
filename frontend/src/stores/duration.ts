// stores/duration.ts
import { defineStore } from 'pinia';

export const useDurationStore = defineStore('duration', {
  state: () => ({
    seconds: 0,
    minutes: 0,
    hours: 0,
  }),
  actions: {
    setSeconds(value: number) {
      this.seconds = value;
      this.saveState();
    },
    setMinutes(value: number) {
      this.minutes = value;
      this.saveState();
    },
    setHours(value: number) {
      this.hours = value;
      this.saveState();
    },
    saveState() {
      localStorage.setItem('durationState', JSON.stringify(this.$state));
    },
    loadState() {
      const storedState = localStorage.getItem('durationState');
      if (storedState) {
        const parsedState = JSON.parse(storedState);
        this.$state = { ...this.$state, ...parsedState };
      }
    },
  },
});
