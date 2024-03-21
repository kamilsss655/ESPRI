import { createApp } from "vue";
import { createPinia } from "pinia";
import { Quasar, Dialog, Notify, AddressbarColor } from "quasar";

import iconSet from "quasar/icon-set/ionicons-v4";
import "@quasar/extras/ionicons-v4/ionicons-v4.css";

import router from "./router";

// Import Quasar css
import "quasar/src/css/index.sass";

// Import custom css
import "./style.css";

// Assumes your root component is App.vue
// and placed in same folder as main.js
import App from "./App.vue";

const pinia = createPinia();

const myApp = createApp(App).use(router).use(pinia);

myApp.use(Quasar, {
  iconSet: iconSet,
  plugins: { Dialog, Notify, AddressbarColor }, // import Quasar plugins and add here
  config: {
    dark: true // force dark mode
  }
});

// Assumes you have a <div id="app"></div> in your index.html
myApp.mount("#app");
