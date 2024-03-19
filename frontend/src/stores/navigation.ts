import { defineStore } from "pinia";

export const useNavigationStore = defineStore({
  id: "navigation",
  state: () => ({ leftDrawerIsOpen: false }),
  getters: {
    leftDrawerOpen: (state) => state.leftDrawerIsOpen,
  },
  actions: {
    toggleLeftDrawer() {
      this.leftDrawerIsOpen = !this.leftDrawerIsOpen;
    }
  }
});
