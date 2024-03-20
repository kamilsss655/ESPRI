import { defineStore } from "pinia";
import axios from "axios";

export const useSettingsStore = defineStore({
  id: "settings",
  state: () => ({
    "wifi.mode": 0,
    "wifi.ssid": "undefined",
    "wifi.password": "undefined",
    "wifi.channel": 0,
    "gpio.status_led": 0,
    "gpio.audio_out": 0
  }),
  actions: {
    async fetchSettings() {
      try {
        const data = await axios.get("/api/settings");
        this.$state = data.data;
      } catch (error) {
        alert(error);
        console.log(error);
      }
    },
    async updateSettings() {
      const jsonData = JSON.stringify(this.$state);
      console.log(jsonData);
      axios
        .post("/api/settings", jsonData, {
          headers: {
            "Content-Type": "application/json"
          }
        })
        .then((response) => {
          console.log(response.data);
        })
        .catch((error) => {
          console.error(error);
        });
    }
  }
});
