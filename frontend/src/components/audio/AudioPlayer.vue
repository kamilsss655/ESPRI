<script setup lang="ts">
import { activateAudioSend } from "../../helpers/Audio";

</script>
<template>
    <q-banner>
      For some of the settings updates to take effect, a system reboot might be
      required.
      <template v-slot:action>
        <q-btn
          icon="ion-refresh"
          label="Reboot"
          type="submit"
          color="primary"
          @click="activateAudioSend()"
        />
      </template>
    </q-banner>
  </template>