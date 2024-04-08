<script setup lang="ts">
import { useSystemStore } from "../../stores/system";
import { systemReboot } from "../../helpers/System";
const systemStore = useSystemStore();

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
        @click="systemReboot()"
      />
    </template>
  </q-banner>
</template>
