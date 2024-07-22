import { defineStore } from "pinia";
import { Notify } from "quasar";
import { useSystemStore } from "./system";
import { Settings, WifiMode, BeaconMode } from "../types/Settings";
import { ApiPaths, ApiResponse } from "../types/Api";
import axios from "axios";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

export const useSettingsStore = defineStore({
  id: "settings",
  state: (): Settings => ({
    "wifi.mode": WifiMode.AP,
    "wifi.ssid": "undefined",
    "wifi.password": "undefined",
    "wifi.channel": 0,
    "wifi.max_connections": 0,
    "gpio.status_led": 22,
    "gpio.audio_in": 35,
    "gpio.audio_out": 18,
    "gpio.ptt": 13,
    "audio.out.volume": 100,
    "audio.in.squelch": 3,
    "led.max_brightness": 5,
    "beacon.mode": BeaconMode.MORSE_CODE,
    "beacon.text": "Default text",
    "beacon.delay_seconds": 12,
    "beacon.morse_code.text": "-.. . ..-. .- ..- .-.. - / - . -..- -",
    "beacon.morse_code.baud": 5,
    "beacon.morse_code.tone_freq": 800,
    "beacon.afsk.baud": 1200,
    "beacon.afsk.zero_freq": 2200,
    "beacon.afsk.one_freq": 1200,
    "beacon.wav.filepath": "/storage/sample.wav"
  }),
  actions: {
    async fetchSettings() {
      try {
        const data = await axiosInstance.get(ApiPaths.Settings);
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
    async updateSettings(askForReboot?: Boolean) {
      const jsonData = JSON.stringify(this.$state);
      axiosInstance
        .post(ApiPaths.Settings, jsonData, {
          headers: {
            "Content-Type": "application/json"
          }
        })
        .then((response: ApiResponse) => {
          console.log(response.data);

          if (askForReboot) {
            const systemStore = useSystemStore();
            systemStore.setRebootRequiredFlag();
          }

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
