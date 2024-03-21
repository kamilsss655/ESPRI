import { createRouter, createWebHistory } from "vue-router";

import Home from "./views/Home.vue";
import MorseCode from "./views/MorseCode.vue";
import About from "./views/About.vue";
import Settings from "./views/Settings.vue";

const router = createRouter({
    history: createWebHistory(),
    routes: [
      { path: '/index.html', redirect: '/' },
      { path: "/", name: "Home", component: Home },
      { path: "/morse_code", name: "Morse code", component: MorseCode },
      { path: "/about", name: "About", component: About },
      { path: "/settings", name: "Settings", component: Settings }
    ]
  });

  export default router
  