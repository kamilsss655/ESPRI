<script setup lang="ts">
import { Notify } from "quasar";
import { useSystemStore } from "../../stores/system";
import { ApiPaths, ApiResponse } from "../../types/Api";
import axios from "axios";
const systemStore = useSystemStore();

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

function reboot() {
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
</script>

<template>
  <q-banner v-if="systemStore.rebootRequired">
    For some of the settings updates to take effect, a system reboot might be
    required.
    <template v-slot:action>
      <q-btn
        outline
        icon="ion-close"
        color="white"
        label="Dismiss"
        @click="systemStore.resetRebootRequiredFlag()"
      />
      <q-btn
        icon="ion-refresh"
        label="Reboot"
        type="submit"
        color="primary"
        @click="reboot()"
      />
    </template>
  </q-banner>
</template>
