import { createRouter, createWebHistory } from "vue-router";

import Home from "./views/Home.vue";
import About from "./views/About.vue";
import Settings from "./views/Settings.vue";

const router = createRouter({
    history: createWebHistory(),
    routes: [
      { path: "/", name: "Home", component: Home },
      { path: "/about", name: "About", component: About },
      { path: "/settings", name: "Settings", component: Settings }
    ]
  });

  export default router
  