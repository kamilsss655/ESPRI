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
    </q-card>
  </div>
</template>

<script setup lang="ts">
import FileUploader from "../components/system/FileUploader.vue";
</script>
