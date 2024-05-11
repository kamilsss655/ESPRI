<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">Files</div>
      </q-card-section>
      <q-separator />
      <q-card-section>
        <FileUploader />
      </q-card-section>
      <q-card-section>
        <q-banner inline-actions rounded class="bg-info text-white">
          File should be named <b>sample.wav</b>. Have resolution of <b>16-bit signed</b>, and <b>32kHz</b> sample rate.
        </q-banner>
      </q-card-section>
    </q-card>
  </div>
</template>

<script setup lang="ts">
import FileUploader from "../components/system/FileUploader.vue";
</script>
