<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">Files</div>
      </q-card-section>
      <q-separator />
      <q-card-section>
        <PathSelector v-model="storagePath"/>
      </q-card-section>
      <q-card-section>
        <Browser :prefix="storagePath.prefix" :path="storagePath.path"  />
      </q-card-section>
      <q-card-section>
        <Uploader :prefix="storagePath.prefix" :path="storagePath.path"/>
      </q-card-section>
      <q-card-section>
        <q-banner inline-actions rounded class="bg-info text-white">
          Files should have resolution of <b>16-bit signed</b>, and <b>32kHz</b> sample rate.
        </q-banner>
      </q-card-section>
    </q-card>
  </div>
</template>

<script setup lang="ts">
import Uploader from "../components/files/Uploader.vue";
import Browser from "../components/files/Browser.vue";
import PathSelector from "../components/files/PathSelector.vue";
import { FilesystemBasePath, StoragePath } from "../types/Filesystem";
import { ref } from "vue";

const storagePath = ref<StoragePath>({ prefix: FilesystemBasePath.SdCard, path: "/" });
</script>
