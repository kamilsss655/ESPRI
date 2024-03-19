import { defineStore } from "pinia";

export const useNavigationStore = defineStore({
  id: "navigation",
  state: () => ({ leftDrawerIsOpen: false }),
  actions: {
    toggleLeftDrawer() {
      this.leftDrawerIsOpen = !this.leftDrawerIsOpen;
    }
  }
});
