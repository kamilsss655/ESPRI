<template>
  <div class="row">
    <div class="col-12 col-md-6">
      <q-table
        title="Directories"
        :dense="$q.screen.lt.lg"
        :rows="listing.directories"
        :columns="columnsDirectories"
        row-key="name"
        :loading="loading"
        flat
        bordered
      >
      <template v-slot:top>
          <p class="text-h6">Directories</p>
          <q-space />
          <q-btn
            round
            dense
            flat
            icon="ion-refresh"
            @click="debouncedFetchData()"
          >
            <q-tooltip>Refresh</q-tooltip>
          </q-btn>
        </template>
      </q-table>
    </div>
    <div class="col-12 col-md-6">
      <q-table
        title="Files"
        :dense="$q.screen.lt.lg"
        :rows="listing.files"
        :columns="columnsFiles"
        row-key="name"
        :loading="loading"
        flat
        bordered
      >
        <template v-slot:top>
          <p class="text-h6">Files</p>
          <q-space />
          <q-btn
            round
            dense
            flat
            icon="ion-refresh"
            @click="debouncedFetchData()"
          >
            <q-tooltip>Refresh</q-tooltip>
          </q-btn>
        </template>

        <template v-slot:body="tableProps">
          <q-tr :props="tableProps">
            <q-td key="name" :props="tableProps">
              {{ tableProps.row.name }}
            </q-td>
            <q-td key="size" :props="tableProps">
              {{ tableProps.row.size }}
            </q-td>
            <q-td key="actions" :props="tableProps">
              <!-- <q-btn
            dense
            flat
            icon="ion-play"
            color="white"
            @click="notImplemented()"
          >
            <q-tooltip> Play {{ tableProps.row.name }} </q-tooltip>
          </q-btn> -->
              <q-btn
                dense
                flat
                icon="ion-flag"
                color="white"
                @click="
                  transmitWAV(props.prefix + props.path + tableProps.row.name)
                "
              >
                <q-tooltip> Transmit {{ tableProps.row.name }} </q-tooltip>
              </q-btn>
              <q-btn
                dense
                flat
                icon="ion-cloud-download"
                color="white"
                @click="
                  downloadFile(props.prefix + props.path, tableProps.row.name)
                "
              >
                <q-tooltip> Download {{ tableProps.row.name }} </q-tooltip>
              </q-btn>
              <q-btn
                dense
                flat
                icon="ion-trash"
                color="white"
                @click="
                  loading = true;
                  deleteFile(props.prefix + props.path + tableProps.row.name);
                  debouncedFetchData();
                "
              >
                <q-tooltip> Delete {{ tableProps.row.name }} </q-tooltip>
              </q-btn>
            </q-td>
          </q-tr>
        </template>
      </q-table>
    </div>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref, watch } from "vue";
import { Notify } from "quasar";
import axios from "axios";
import { debounce } from "lodash";
import { Listing } from "../../types/Filesystem";
import { transmitWAV } from "../../helpers/Transmit";
import { deleteFile } from "../../helpers/Filesystem";

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

const axiosInstance = axios.create();

const loading = ref(false);

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
  },
  { name: "actions", label: "Actions", field: "actions" }
];

const columnsDirectories = [
  { name: "name", label: "Name", field: "name", sortable: true }
];

const fetchData = async () => {
  try {
    const response = await axiosInstance.get(props.prefix + props.path);
    listing.value = response.data;
    Notify.create({
      message: `Fetched listing for the ${
        props.prefix + props.path
      } directory.`,
      color: "positive"
    });
  } catch (error) {
    Notify.create({
      message: `Failed to fetch listing for the ${
        props.prefix + props.path
      } directory`,
      color: "negative"
    });
    console.error(error);
  } finally {
    loading.value = false;
  }
};

const downloadFile = async (relativePath: string, filename: string) => {
  Notify.create({
    message: `Downloading file. Please wait.`,
    color: "positive"
  });
  axiosInstance({
    url: `${relativePath}/${filename}`,
    method: "GET",
    responseType: "blob"
  })
    .then((response) => {
      const url = window.URL.createObjectURL(new Blob([response.data]));
      const link = document.createElement("a");
      link.href = url;
      link.setAttribute("download", filename);
      document.body.appendChild(link);
      link.click();
      if (link.parentNode === null) {
        return;
      }
      link.parentNode.removeChild(link);
    })
    .catch((error) => {
      console.error("Error downloading file:", error);
    });
};

// Debounced version of the fetchData() function - can only be called once per 500ms
const debouncedFetchData = debounce(async () => {
  loading.value = true;
  fetchData();
}, 500);

// On component render we fetch data
onMounted(() => {
  debouncedFetchData();
});

// If current path changes we fetch data
watch(
  () => [props.prefix, props.path],
  ([currentPath, storage]) => {
    if (currentPath || storage) {
      loading.value = true;
      debouncedFetchData();
    }
  }
);
</script>
