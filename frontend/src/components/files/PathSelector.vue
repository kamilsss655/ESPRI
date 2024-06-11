<template>
  <div class="row">
    <div class="col-12 col-md-2">
      <q-select
        v-model="storagePath.prefix"
        :options="storageOptions"
        label="Prefix"
        behavior="dialog"
        emit-value
        map-options
      >
        <template v-slot:option="scope">
          <q-item v-bind="scope.itemProps">
            <q-item-section avatar>
              <q-icon :name="scope.opt.icon" />
            </q-item-section>
            <q-item-section>
              <q-item-label>{{ scope.opt.label }}</q-item-label>
              <q-item-label caption>{{ scope.opt.description }}</q-item-label>
            </q-item-section>
          </q-item>
        </template>
      </q-select>
    </div>
    <div class="col-12 col-md-10">
      <q-input v-model="storagePath.path" label="Path" />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from "vue";
import { FilesystemBasePath, StoragePath } from "../../types/Filesystem";

const storagePath: StoragePath = defineModel();

const storageOptions = ref([
  {
    label: FilesystemBasePath.Flash,
    value: FilesystemBasePath.Flash,
    description: "Device FLASH memory. Limited in size and can be unreliable.",
    icon: "ion-bug"
  },
  {
    label: FilesystemBasePath.SdCard,
    value: FilesystemBasePath.SdCard,
    description: "Removable SD card memory. Larger size and reliable.",
    icon: "ion-save"
  }
]);
</script>
