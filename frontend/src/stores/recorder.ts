import { defineStore } from "pinia";
import { ApiPaths, ApiResponse } from "../types/Api";
import { Notify } from "quasar";
import axios from "axios";
import { Recorder, RecordParam } from "../types/Recorder";
import { formatTimestamp } from "../helpers/Time";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

const formattedTimeStamp = formatTimestamp(new Date(Date.now()));

export const useRecorderStore = defineStore({
  id: "recorder",
  state: (): Recorder => ({
    filepath: "sample_" + formattedTimeStamp + "wav",
    duration_seconds: 10,
    duration_minutes: 0,
    duration_hours: 0
  }),
  actions: {
    async scheduleRecording() {
      const recordParam: RecordParam = {
        filepath: this.filepath,
        duration_sec: this.durationTotalInSeconds
      }
      const jsonData = JSON.stringify(recordParam);
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
  getters: {
    durationTotalInSeconds: (state): number =>
      state.duration_seconds + state.duration_minutes * 60 + state.duration_hours * 3600
  }
});