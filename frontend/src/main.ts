import { createApp } from "vue";
import { createRouter, createWebHistory } from "vue-router";
import { Quasar } from "quasar";

import Main from "./components/views/Main.vue";
import Main2 from "./components/views/Main2.vue";

// Import icon libraries
import "@quasar/extras/material-icons/material-icons.css";

// Import Quasar css
import "quasar/src/css/index.sass";

// Import custom css
import "./style.css";

// Assumes your root component is App.vue
// and placed in same folder as main.js
import App from "./App.vue";

const router = createRouter({
  history: createWebHistory(),
  routes: [
    { path: "/", name: "Home", component: Main },
    { path: "/2", name: "Home2", component: Main2 },
  ]
});

const myApp = createApp(App).use(router);

myApp.use(Quasar, {
  plugins: {}, // import Quasar plugins and add here
});

// Assumes you have a <div id="app"></div> in your index.html
myApp.mount("#app");
