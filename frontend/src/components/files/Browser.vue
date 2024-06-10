<template>
  <q-input v-model="currentPath" label="Current path" />
  <br />
  <q-table
    title="Directories"
    :grid="$q.screen.xs"
    :rows="listing.directories"
    :columns="columnsDirectories"
    row-key="name"
  />
  <br />
  <q-table
    title="Files"
    :grid="$q.screen.xs"
    :rows="listing.files"
    :columns="columnsFiles"
    row-key="name"
  />
</template>

<script setup lang="ts">
import { onMounted, ref, watch } from "vue";
import { Notify } from "quasar";
import axios from "axios";
import { debounce } from "lodash";
import { FilesystemBasePath, Listing } from "../../types/Filesystem";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

const currentPath = ref(FilesystemBasePath.SdCard);

const listing = ref<Listing>({
  files: [],
  directories: []
});

const columnsFiles = [
  { name: "name", label: "Name", field: "name", sortable: true },
  {
    name: "size",
    label: "Size (bytes)",
    field: "size",
    sortable: true,
    sort: (a: string, b: string, _rowA: any, _rowB: any) =>
      parseInt(a, 10) - parseInt(b, 10)
  }
];

const columnsDirectories = [
  { name: "name", label: "Name", field: "name", sortable: true }
];

const fetchData = async () => {
  try {
    const response = await axios.get(currentPath.value + "/");
    listing.value = response.data;
    Notify.create({
      message: `Fetched listing for the ${currentPath.value} directory.`,
      color: "positive"
    });
  } catch (error) {
    Notify.create({
      message: `Failed to fetch listing for the ${currentPath.value} directory`,
      color: "negative"
    });
    console.error(error);
  }
};

// Debounced version of the fetchData() function - can only be called once per 500ms
const debouncedFetchData = debounce(async () => {
  fetchData();
}, 500);

// On component render we fetch data
onMounted(() => {
  debouncedFetchData();
});

// If current path changes we fetch data
watch(
  () => currentPath.value,
  () => {
    debouncedFetchData();
  }
);
</script>
