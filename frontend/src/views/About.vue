<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">About</div>
      </q-card-section>

      <q-card-section class="q-pt-none">
        "heap.total": {{ systemStore.info["heap.total"] }}
        "heap.free": {{ systemStore.info['heap.free'] }}
        "heap.min_free": {{ systemStore.info['heap.min_free'] }}
        "storage.free": {{ systemStore.info['storage.free'] }}
        "storage.total": {{ systemStore.info['heap.total'] }}
        "uptime": {{ systemStore.info['uptime'] }}
        "version": {{ systemStore.info['version'] }}
      </q-card-section>
    </q-card>
  </div>
</template>
<script setup lang="ts">
import { useSystemStore } from "../stores/system";

const systemStore = useSystemStore();
</script>
