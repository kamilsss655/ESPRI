<template>
  <q-table
    title="Directories"
    :grid="$q.screen.xs"
    :rows="listing.directories"
    :columns="columnsDirectories"
    row-key="name"
  />
  <p></p>
  <q-table
    title="Files"
    :grid="$q.screen.xs"
    :rows="listing.files"
    :columns="columnsFiles"
    row-key="name"
  />
</template>

<script setup lang="ts">
import { onMounted, ref } from "vue";
import { Notify } from "quasar";
import axios from "axios";
import { Listing } from "../../types/Filesystem";

const axiosInstance = axios.create();
axiosInstance.defaults.timeout = 600;

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

onMounted(async () => {
  try {
    const response = await axios.get("/sd/");
    listing.value = response.data;
    Notify.create({
      message: "Fetched listing for the /sd/ directory.",
      color: "positive"
    });
  } catch (error) {
    Notify.create({
      message: "Failed to fetch listing for the /sd/ directory",
      color: "negative"
    });
    console.error(error);
  }
});
</script>
