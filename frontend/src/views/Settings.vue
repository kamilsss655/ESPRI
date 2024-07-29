<template>
  <q-card flat bordered>
    <q-card-section>
      <div class="text-h4 text-center">Settings</div>
    </q-card-section>
    <q-separator />
    <q-card-section>
      <q-form
        @submit="submitForm"
        @reset="settingsStore.fetchSettings"
        class="q-gutter-md"
      >
        <div>
          <q-splitter v-model="splitterModel" style="height: 50vh">
            <template v-slot:before>
              <q-tabs v-model="currentTab" vertical>
                <q-tab name="general" icon="ion-settings" label="General" />
                <q-tab name="wifi" icon="ion-wifi" label="Wifi" />
                <q-tab name="gpio" icon="ion-swap" label="GPIO" />
                <q-tab name="advanced" icon="ion-build" label="Advanced" />
              </q-tabs>
            </template>

            <template v-slot:after>
              <q-tab-panels
                v-model="currentTab"
                animated
                swipeable
                vertical
                transition-prev="jump-up"
                transition-next="jump-up"
              >
                <q-tab-panel name="general">
                  <div class="q-pa-md">
                    <q-list>
                      <q-item>
                        <q-item-section :side="true">
                          <q-icon name="ion-volume-high" />
                        </q-item-section>
                        <q-item-section :side="true">Out</q-item-section>
                        <q-item-section>
                          <q-slider
                            v-model="settingsStore['audio.out.volume']"
                            :label-value="
                              settingsStore['audio.out.volume'] + '%'
                            "
                            :min="0"
                            :max="100"
                            label
                          />
                        </q-item-section>
                      </q-item>
                      <q-item>
                        <q-item-section :side="true">
                          <q-icon name="ion-volume-high" />
                        </q-item-section>
                        <q-item-section :side="true">In</q-item-section>
                        <q-item-section>
                          <q-slider
                            v-model="settingsStore['audio.in.squelch']"
                            :label-value="
                              settingsStore['audio.in.squelch'] + '%'
                            "
                            :min="0"
                            :max="100"
                            label
                          />
                        </q-item-section>
                      </q-item>
                      <q-item>
                        <q-item-section :side="true">
                          <q-icon name="ion-sunny" />
                        </q-item-section>
                        <q-item-section :side="true">LED</q-item-section>
                        <q-item-section>
                          <q-slider
                            v-model="settingsStore['led.max_brightness']"
                            :label-value="
                              settingsStore['led.max_brightness'] + '%'
                            "
                            :min="0"
                            :max="100"
                            label
                          />
                        </q-item-section>
                      </q-item>
                    </q-list>
                  </div>
                </q-tab-panel>
                <q-tab-panel name="wifi">
                  <div class="q-pa-md">
                    <q-select
                      filled
                      v-model="settingsStore['wifi.mode']"
                      :options="wifiModeOptions"
                      label="Wifi mode"
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
                            <q-item-label caption>{{
                              scope.opt.description
                            }}</q-item-label>
                          </q-item-section>
                        </q-item>
                      </template>
                    </q-select>
                  </div>
                  <div class="q-pa-md">
                    <q-input
                      v-model="settingsStore['wifi.ssid']"
                      label="SSID"
                      filled
                    />
                  </div>
                  <div class="q-pa-md">
                    <q-input
                      v-model="settingsStore['wifi.password']"
                      label="Password"
                      filled
                      :type="hideWifiPassword ? 'password' : 'text'"
                    >
                      <template v-slot:append>
                        <q-icon
                          :name="hideWifiPassword ? 'ion-eye' : 'ion-eye-off'"
                          class="cursor-pointer"
                          @click="hideWifiPassword = !hideWifiPassword"
                        />
                      </template>
                    </q-input>
                  </div>
                  <div class="q-pa-md">
                    <q-select
                      v-model="settingsStore['wifi.channel']"
                      :options="wifiChannelOptions"
                      v-if="settingsStore['wifi.mode'] == 1"
                      behavior="dialog"
                      label="Wifi channel"
                      filled
                    />
                  </div>
                </q-tab-panel>

                <q-tab-panel name="gpio">
                  <div class="q-pa-md">
                    <q-select
                      v-model.number="settingsStore['gpio.status_led']"
                      :options="gpioOptions"
                      behavior="dialog"
                      label="Status LED"
                      filled
                    />
                  </div>
                  <div class="q-pa-md">
                    <q-select
                      v-model.number="settingsStore['gpio.audio_in']"
                      :options="gpioOptions"
                      behavior="dialog"
                      label="Audio IN"
                      filled
                    />
                  </div>
                  <div class="q-pa-md">
                    <q-select
                      v-model.number="settingsStore['gpio.audio_out']"
                      :options="gpioOptions"
                      behavior="dialog"
                      label="Audio OUT"
                      filled
                    />
                  </div>
                  <div class="q-pa-md">
                    <q-select
                      v-model.number="settingsStore['gpio.ptt']"
                      :options="gpioOptions"
                      behavior="dialog"
                      label="PTT Pin"
                      filled
                    />
                  </div>
                </q-tab-panel>
                <q-tab-panel name="advanced">
                  <div class="q-pa-md text-center">
                    <q-btn
                      icon="ion-build"
                      label="Factory reset"
                      @click="factoryReset()"
                      type="reset"
                      class="q-ml-sm"
                      outline
                    />
                  </div>
                </q-tab-panel>
              </q-tab-panels>
            </template>
          </q-splitter>
        </div>

        <div class="text-right q-pa-md">
          <q-btn icon="ion-play" label="Submit" type="submit" color="primary" />
          <q-btn
            icon="ion-refresh"
            label="Reload"
            type="reset"
            class="q-ml-sm"
            outline
          />
        </div>
      </q-form>
    </q-card-section>
  </q-card>
</template>

<script setup lang="ts">
import { ref, onMounted } from "vue";
import { useQuasar } from "quasar";
import { useSettingsStore } from "../stores/settings";
import { WifiMode } from "../types/Settings";
import { systemFactoryReset } from "../helpers/System";

const hideWifiPassword = ref(true);
const settingsStore = useSettingsStore();
const $q = useQuasar();
const currentTab = ref("general");
const splitterModel = ref(20);

onMounted(() => {
  settingsStore.fetchSettings();
});

function submitForm() {
  $q.dialog({
    title: "Confirm",
    message: "Are you sure?",
    cancel: true,
    persistent: true
  }).onOk(() => {
    settingsStore.updateSettings();
  });
}

function factoryReset() {
  $q.dialog({
    title: "Confirm",
    message:
      "Are you sure? This will completely reset device settings to the factory defaults.",
    cancel: true,
    persistent: true
  }).onOk(() => {
    systemFactoryReset();
  });
}

const wifiModeOptions = ref([
  {
    label: "Off",
    value: WifiMode.OFF,
    description: "Turns WiFi off.",
    icon: "ion-airplane"
  },
  {
    label: "Access point",
    value: WifiMode.AP,
    description: "Acts as a WiFi router.",
    icon: "ion-wifi"
  },
  {
    label: "Station mode",
    value: WifiMode.STA,
    description: "Connect to your network.",
    icon: "ion-wifi"
  }
]);

const wifiChannelOptions = ref([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13]);
const gpioOptions = ref(Array.from({ length: 36 }, (_v, i) => i));
</script>
