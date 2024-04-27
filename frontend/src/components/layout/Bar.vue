<template>
  <q-bar>
    <!-- <div class="text-weight-bold">Status bar</div> -->
    <q-icon v-if="isWebsocketConnected" name="ion-cellular" />
    <q-btn
      dense
      v-if="!isWebsocketConnected"
      flat
      icon="ion-close-circle"
      color="negative"
    />
    <div>{{ settingsStore["wifi.ssid"] }}</div>
    <q-space />

    <q-btn
      dense
      flat
      icon="ion-flag"
      v-if="isBeaconEnabled"
      to="/beacon"
    />
    <q-btn dense flat icon="ion-arrow-dropup-circle" v-if="systemStore.audioState == AudioState.TRANSMITTING" color="red">
      <q-tooltip>
        TRANSMITTING
      </q-tooltip>
    </q-btn>
    <q-btn dense flat icon="ion-arrow-dropdown-circle" v-if="systemStore.audioState == AudioState.RECEIVING" color="warning">
      <q-tooltip>
        RECEIVING
      </q-tooltip>
    </q-btn>
    <q-btn dense flat icon="ion-headset" v-if="systemStore.audioState == AudioState.LISTENING" color="white">
      <q-tooltip>
        LISTENING
      </q-tooltip>
    </q-btn>
    <q-btn
      dense
      flat
      icon="ion-notifications"
      color="white"
      to="/notifications"
    >
      <q-badge rounded color="white" text-color="black">
        {{ websocketStore.messageCount }}
        {{ websocketStore.messageCountMaxLimitReached ? "+" : "" }}
      </q-badge>
    </q-btn>
    
  </q-bar>
</template>

<script setup lang="ts">
import { AudioState } from "../../types/System";
import { onMounted, computed } from "vue";
import { useSettingsStore } from "../../stores/settings";
import { useWebsocketStore } from "../../stores/websocket";
import { useSystemStore } from "../../stores/system";
import { BeaconMode } from "../../types/Settings";

const settingsStore = useSettingsStore();
const websocketStore = useWebsocketStore();
const systemStore = useSystemStore();

// Keep alive function that checks when we received ping from ESP
function keepAlive() {
  let secondsSinceLastSeen: number =
    (new Date().getTime() - websocketStore.lastSeenTimestamp) / 1000;
  secondsSinceLastSeen = Math.floor(secondsSinceLastSeen);

  // If we didn't receive ping from ESP in 5 seconds it is likely offline
  if (secondsSinceLastSeen > 5) {
    websocketStore.$state.connected = false;
    // discard old connections to avoid multiple websocket connections
    websocketStore.disconnect();
    // zero the delta between times
    websocketStore.updateLastSeenTimestamp();
    // try to connect
    websocketStore.connect();
  }
}

onMounted(() => {
  settingsStore.fetchSettings();
  setInterval(keepAlive, 1000);
});

const isBeaconEnabled = computed(() => {
  return settingsStore["beacon.mode"] != BeaconMode.OFF;
});

const isWebsocketConnected = computed(() => {
  return websocketStore["connected"];
});
</script>
