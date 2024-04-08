<script setup lang="ts">
import { systemReboot, systemDeepSleep } from "../../helpers/System";
</script>

<template>
  <q-btn-dropdown round dense flat icon="ion-power">
    <q-list class="bg-primary">
      <q-item clickable v-close-popup @click="systemReboot()">
        <q-item-section side>
          <q-icon name="ion-refresh" />
        </q-item-section>
        <q-item-section>
          <q-item-label>Restart</q-item-label>
        </q-item-section>
      </q-item>

      <q-item clickable v-close-popup @click="systemDeepSleep()">
        <q-item-section side>
          <q-icon name="ion-cloudy-night" />
        </q-item-section>
        <q-item-section>
          <q-item-label>Deep sleep</q-item-label>
        </q-item-section>
      </q-item>
    </q-list>
  </q-btn-dropdown>

  <!-- <q-btn round dense flat to="/about" icon="ion-information-circle">
          <q-tooltip>About</q-tooltip>
        </q-btn> -->
</template>
