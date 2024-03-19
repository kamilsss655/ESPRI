import { createRouter, createWebHistory } from "vue-router";

import Home from "./views/Home.vue";
import About from "./views/About.vue";

const router = createRouter({
    history: createWebHistory(),
    routes: [
      { path: "/", name: "Home", component: Home },
      { path: "/about", name: "Home2", component: About }
    ]
  });

  export default router
  