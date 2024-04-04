<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">Notifications</div>
      </q-card-section>

      <q-separator />

      <q-item tag="label" v-ripple>
        <q-item-section>
          <q-item-label>Auto scroll</q-item-label>
        </q-item-section>
        <q-item-section avatar>
          <q-toggle v-model="isAutoScrollEnabled" />
        </q-item-section>
      </q-item>

      <q-separator />

      <q-virtual-scroll
        ref="virtualScrollRef"
        style="max-height: 60vh"
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
import { watch, ref } from "vue";
import { useWebsocketStore } from "../stores/websocket";
import { QVirtualScroll } from "quasar";

const websocketStore = useWebsocketStore();
const virtualScrollRef = ref<QVirtualScroll>();
let isAutoScrollEnabled = ref(true);

// watch for new messages and auto-scroll down when new message detected
watch(
  () => websocketStore.messageCount,
  (count) => {
    if (!isAutoScrollEnabled.value) {
      return;
    }

    setTimeout(() => {
      virtualScrollRef.value?.scrollTo(count, "center");
    }, 10);
  }
);
</script>
