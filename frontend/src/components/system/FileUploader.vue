<template>
  <q-uploader
    :factory=factoryFn
    label="Max file size (1.2MB)"
    multiple
    send-raw
    accept=".wav"
    max-file-size="1228800"
    max-total-size="1228800"
    @rejected="onRejected"
    @failed="onFailed"
    @uploaded="onUploaded"
  />
</template>

<script setup lang="ts">
import { Notify, QUploaderFactoryFn } from "quasar";
import { ApiPaths } from "../../types/Api";

function onUploaded() {
  Notify.create({
    type: "positive",
    message: `Upload complete.`
  });
}

function onRejected(rejectedEntries: any) {
  console.log(rejectedEntries);
  Notify.create({
    type: "negative",
    message: `${rejectedEntries.length} file(s) did not pass validation constraints`
  });
}

function onFailed(info: {files: readonly any[], xhr: any}) {
  console.log(info.files);
  console.log(info.xhr);
  Notify.create({
    type: "negative",
    message: `Upload failed`
  });
}

// Resolve upload URL
const factoryFn: QUploaderFactoryFn = (files: readonly File[]) => {
  return {
    url: ApiPaths.FileUpload + "/" + files[0].name,
    method: "POST"
  };
};
</script>
