import { createRouter, createWebHistory } from "vue-router";

import Home from "./views/Home.vue";
import Beacon from "./views/Beacon.vue";
import About from "./views/About.vue";
import Notifications from "./views/Notifications.vue";
import Settings from "./views/Settings.vue";
import Files from "./views/Files.vue";
import Recorder from "./views/Recorder.vue";

const router = createRouter({
    history: createWebHistory(),
    routes: [
      { path: '/index.html', redirect: '/' },
      { path: "/", name: "Home", component: Home },
      { path: "/beacon", name: "Beacon", component: Beacon },
      { path: "/about", name: "About", component: About },
      { path: "/notifications", name: "Notifications", component: Notifications },
      { path: "/settings", name: "Settings", component: Settings },
      { path: "/files", name: "Files", component: Files },
      { path: "/recorder", name: "Recorder", component: Recorder }
    ]
  });

  export default router
  