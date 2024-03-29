<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">Notifications</div>
      </q-card-section>

      <q-list bordered separator>
        <q-item
          clickable
          v-ripple
          v-for="item in websocketStore.$state.messages"
        >
          <q-item-section>
            <q-item-label overline>{{ item.tag }}</q-item-label>
            <q-item-label>{{ item.message }}</q-item-label>
          </q-item-section>
        </q-item>
      </q-list>
    </q-card>
  </div>
</template>

<script setup lang="ts">
import { useWebsocketStore } from "../stores/websocket";

const websocketStore = useWebsocketStore();
</script>
