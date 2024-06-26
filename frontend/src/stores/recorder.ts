import { defineStore } from "pinia";
import { ApiPaths, ApiResponse } from "../types/Api";
import { Notify } from "quasar";
import axios from "axios";
import { Recorder } from "../types/Recorder";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

export const useRecorderStore = defineStore({
  id: "recorder",
  state: (): Recorder => ({
    filepath: "sample.wav",
    duration_sec: 10
  }),
  actions: {
    async scheduleRecording() {
      const jsonData = JSON.stringify(this.$state);
      axiosInstance
        .put(ApiPaths.Record, jsonData, {
          headers: {
            "Content-Type": "application/json"
          }
        })
        .then((response: ApiResponse) => {
          console.log(response.data);

          Notify.create({
            message: response.data.response,
            color: "positive"
          });
        })
        .catch((error) => {
          console.error(error);
          if (error.response) {
            let response: ApiResponse = error.response;
      
            Notify.create({
              message: response.data.response,
              color: "negative"
            });
          }
        });
    }
  },
  getters: {}
});
