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
import { ApiPaths, ApiResponse, GetApiResponseFromJson } from "../../types/Api";

const props = defineProps({
  prefix: {
    type: String,
    required: true
  },
  path: {
    type: String,
    required: true
  }
});

function onUploaded(info: {files: readonly any[], xhr: XMLHttpRequest}) {
  const response: ApiResponse = GetApiResponseFromJson(info.xhr.response);

  Notify.create({
    type: "positive",
    message: response.data.response
  });
}

function onRejected(rejectedEntries: any) {
  console.log(rejectedEntries);
  Notify.create({
    type: "negative",
    message: `${rejectedEntries.length} file(s) did not pass validation constraints. File too big?`
  });
}

function onFailed(info: {files: readonly any[], xhr: XMLHttpRequest}) {
  console.log(info.files);
  console.log(info.xhr);

  const response: ApiResponse = GetApiResponseFromJson(info.xhr.response);

  Notify.create({
    type: "negative",
    message: "Error! " + response.data.response
  });
}

// Resolve upload URL
const factoryFn: QUploaderFactoryFn = (files: readonly File[]) => {
  return {
    url: ApiPaths.FileUpload + props.prefix + props.path + files[0].name,
    method: "POST"
  };
};
</script>
