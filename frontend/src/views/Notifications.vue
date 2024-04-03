<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">Notifications</div>
      </q-card-section>
        <q-virtual-scroll
          style="max-height: 68vh"
          :items="websocketStore.$state.messages"
          separator
          v-slot="{ item, index }"
        >
          <q-item clickable v-ripple :key="index">
            <q-item-section>
              <q-item-label overline>{{ item.tag }}</q-item-label>
              <q-item-label>{{ item.message }}</q-item-label>
            </q-item-section>
          </q-item>
        </q-virtual-scroll>
    </q-card>
  </div>
</template>

<script setup lang="ts">
import { useWebsocketStore } from "../stores/websocket";

const websocketStore = useWebsocketStore();
</script>
