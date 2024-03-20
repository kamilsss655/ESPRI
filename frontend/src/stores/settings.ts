import { defineStore } from "pinia";

export const useSettingsStore = defineStore({
  id: "settings",
  state: () => ({ 
    ssid: "test",
    password: "testpass" 
  }),
  actions: {
    getSettings() {
      this.ssid = "fetched"
    }
  }
});
