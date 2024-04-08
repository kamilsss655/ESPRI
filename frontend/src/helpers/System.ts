import axios from "axios";
import { ApiPaths, ApiResponse } from "../types/Api";
import { useSystemStore } from "../stores/system";
import { Notify } from "quasar";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

// Perform system reboot
export function systemReboot() {
  axiosInstance
    .put(ApiPaths.Reboot, null, {
      headers: {
        "Content-Type": "application/json"
      }
    })
    .then((response: ApiResponse) => {
      const systemStore = useSystemStore();
      systemStore.resetRebootRequiredFlag();

      Notify.create({
        message: response.data.response,
        color: "positive"
      });
    })
    .catch((_error) => {
      Notify.create({
        message: "Error.",
        color: "negative"
      });
    });
}

// Go into deep sleep
export function systemDeepSleep() {
  axiosInstance
    .put(ApiPaths.DeepSleep, null, {
      headers: {
        "Content-Type": "application/json"
      }
    })
    .then((response: ApiResponse) => {
      Notify.create({
        message: response.data.response,
        color: "positive"
      });
    })
    .catch((_error) => {
      Notify.create({
        message: "Error.",
        color: "negative"
      });
    });
}