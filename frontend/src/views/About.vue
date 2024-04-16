<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">About</div>
      </q-card-section>
      <q-separator />
      <q-card-section>
        <q-item tag="label" v-ripple>
          <q-item-section>
            <q-item-label>Live update</q-item-label>
          </q-item-section>
          <q-item-section avatar>
            <q-toggle v-model="systemStore.systemInfoLiveUpdate" />
          </q-item-section>
        </q-item>
      </q-card-section>
      <q-separator />
      <q-card-section class="q-pt-none">
        <q-list>
          <q-item>
            <q-item-section :side="true">
              <q-icon name="ion-film" />
            </q-item-section>
            <q-item-section>Memory</q-item-section>
            <q-item-section>
              <q-linear-progress
                size="25px"
                :value="heapMemoryUsageProgress"
                color="accent"
              >
                <div class="absolute-full flex flex-center">
                  <q-badge
                    color="white"
                    text-color="accent"
                    :label="heapMemoryUsageLabel"
                  />
                </div>
              </q-linear-progress>
            </q-item-section>
          </q-item>
          <q-item>
            <q-item-section :side="true">
              <q-icon name="ion-speedometer" />
            </q-item-section>
            <q-item-section>Max memory usage</q-item-section>
            <q-item-section>
              <q-linear-progress
                size="25px"
                :value="heapMaxMemoryUsageProgress"
                color="accent"
              >
                <div class="absolute-full flex flex-center">
                  <q-badge
                    color="white"
                    text-color="accent"
                    :label="heapMaxMemoryUsageLabel"
                  />
                </div>
              </q-linear-progress>
            </q-item-section>
          </q-item>
          <q-item>
            <q-item-section :side="true">
              <q-icon name="ion-save" />
            </q-item-section>
            <q-item-section>Storage</q-item-section>
            <q-item-section>
              <q-linear-progress
                size="25px"
                :value="storageUsageProgress"
                color="accent"
              >
                <div class="absolute-full flex flex-center">
                  <q-badge
                    color="white"
                    text-color="accent"
                    :label="storageUsageLabel"
                  />
                </div>
              </q-linear-progress>
            </q-item-section>
          </q-item>
          <q-item>
            <q-item-section :side="true">
              <q-icon name="ion-time" />
            </q-item-section>
            <q-item-section>Uptime</q-item-section>
            <q-item-section>
              {{ uptimeInSeconds }}
            </q-item-section>
          </q-item>
          <q-item>
            <q-item-section :side="true">
              <q-icon name="ion-bug" />
            </q-item-section>
            <q-item-section>Version</q-item-section>
            <q-item-section>
              {{ systemStore.info["version"] }}
            </q-item-section>
          </q-item>
        </q-list>
      </q-card-section>
    </q-card>
  </div>
</template>
<script setup lang="ts">
import { computed, onMounted, onUnmounted, watch } from "vue";
import { useSystemStore } from "../stores/system";

const systemStore = useSystemStore();

let liveFetchSystemInfo: number;

function enableLiveUpdate() {
  liveFetchSystemInfo = setInterval(systemStore.fetchSystemInfo, 1000);
}

function disableLiveUpdate() {
  clearInterval(liveFetchSystemInfo);
}

onMounted(() => {
  if (systemStore.systemInfoLiveUpdate) {
    enableLiveUpdate();
  } else {
    systemStore.fetchSystemInfo();
  }
});

onUnmounted(() => {
  disableLiveUpdate();
});

watch(
  () => systemStore.systemInfoLiveUpdate,
  (newValue, _oldValue) => {
    if (newValue === true) {
      enableLiveUpdate();
    } else {
      disableLiveUpdate();
    }
  }
);

const heapMemoryUsageProgress: any = computed(() => {
  return systemStore.heapUsagePercent / 100;
});

const heapMemoryUsageLabel = computed(() => {
  return (
    (systemStore.info["heap.total"] - systemStore.info["heap.free"]).toFixed(
      0
    ) +
    "/" +
    systemStore.info["heap.total"].toFixed(0) +
    " " +
    systemStore.heapUsagePercent.toFixed(2) +
    "%"
  );
});

const heapMaxMemoryUsageProgress: any = computed(() => {
  return systemStore.heapMaxUsagePercent / 100;
});

const heapMaxMemoryUsageLabel = computed(() => {
  return (
    (
      systemStore.info["heap.total"] - systemStore.info["heap.min_free"]
    ).toFixed(0) +
    "/" +
    systemStore.info["heap.total"].toFixed(0) +
    " " +
    systemStore.heapMaxUsagePercent.toFixed(2) +
    "%"
  );
});

const storageUsageProgress: any = computed(() => {
  return systemStore.storageUsagePercent / 100;
});

const storageUsageLabel = computed(() => {
  return (
    systemStore.info["storage.used"].toFixed(0) +
    "/" +
    systemStore.info["storage.total"].toFixed(0) +
    " " +
    systemStore.storageUsagePercent.toFixed(2) +
    "%"
  );
});

const uptimeInSeconds: any = computed(() => {
  return systemStore.info["uptime"] + "s";
});
</script>
