import { defineStore } from "pinia";
import { Notify } from "quasar";
import axios from "axios";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

export const useSettingsStore = defineStore({
  id: "settings",
  state: () => ({
    "wifi.mode": 1,
    "wifi.ssid": "undefined",
    "wifi.password": "undefined",
    "wifi.channel": 0,
    "gpio.status_led": 0,
    "gpio.audio_in": 0,
    "gpio.audio_out": 0,
    "gpio.ptt": 0,
    "morse_code_beacon.text": "undefined",
    "morse_code_beacon.period_seconds": 0,
    "morse_code_beacon.enabled": 0
  }),
  actions: {
    async fetchSettings() {
      try {
        const data = await axiosInstance.get("/api/settings");
        this.$state = data.data;
        Notify.create({
          message: "Fetched up-to date data.",
          color: "positive"
        });
      } catch (error) {
        Notify.create({
          message: "Failed to fetch up-to date data.",
          color: "negative"
        });
        console.log(error);
      }
    },
    async updateSettings() {
      const jsonData = JSON.stringify(this.$state);
      axiosInstance
        .post("/api/settings", jsonData, {
          headers: {
            "Content-Type": "application/json"
          }
        })
        .then((response) => {
          console.log(response.data);
          Notify.create({
            message: response.data.response,
            color: "positive"
          });
        })
        .catch((error) => {
          console.error(error);
          Notify.create({
            message: "Error.",
            color: "negative"
          });
        });
    }
  }
});
