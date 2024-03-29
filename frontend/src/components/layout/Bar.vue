<template>
  <q-bar>
    <!-- <div class="text-weight-bold">Status bar</div> -->
    <q-icon v-if="isWebsocketConnected" name="ion-cellular" />
    <q-btn dense v-if="!isWebsocketConnected" flat icon="ion-close-circle" color="negative"/>
    <div>{{ settingsStore["wifi.ssid"] }}</div>
    <q-space />
    <q-btn dense flat icon="ion-flag" v-if="isMorseCodeBeaconEnabled" />
    <q-btn dense flat icon="ion-notifications" color="white">
      <q-badge rounded color="white" text-color="black">{{
        websocketStore.messageCount
      }}</q-badge>
    </q-btn>
  </q-bar>
</template>

<script setup lang="ts">
import { onMounted, computed } from "vue";
import { useSettingsStore } from "../../stores/settings";
import { useWebsocketStore } from "../../stores/websocket";

const settingsStore = useSettingsStore();
const websocketStore = useWebsocketStore();

// Keep alive function that checks when we received ping from ESP
function keepAlive() {
  let secondsSinceLastSeen: number = (new Date().getTime() - websocketStore.lastSeenTimestamp) / 1000;
  secondsSinceLastSeen =  Math.floor(secondsSinceLastSeen);
  console.log(secondsSinceLastSeen);

  // If we didn't receive ping from ESP in 5 seconds it is likely offline
  if (secondsSinceLastSeen > 5) {
    websocketStore.$state.connected = false;
    websocketStore.disconnect();
    websocketStore.updateLastSeenTimestamp();
    console.log("reconnecting");
    websocketStore.connect();
  }
}

onMounted(() => {
  settingsStore.fetchSettings();
  setInterval(keepAlive, 1000);
});

const isMorseCodeBeaconEnabled = computed(() => {
  return settingsStore["morse_code_beacon.enabled"];
});

const isWebsocketConnected = computed(() => {
  return websocketStore["connected"];
});
</script>
