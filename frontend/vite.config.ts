import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import { quasar, transformAssetUrls } from "@quasar/vite-plugin";

// TODO: Rewrite to .env
let espIpAddress = "10.0.5.6";

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    vue({
      template: { transformAssetUrls }
    }),

    // @quasar/plugin-vite options list:
    // https://github.com/quasarframework/quasar/blob/dev/vite-plugin/index.d.ts
    quasar({
      sassVariables: "src/quasar-variables.sass"
    })
  ],
  // Setup proxy to allow making requests to ESP in dev mode
  server: {
    hmr: {
      host: "localhost",
      port: 4300,
      protocol: "ws"
    },
    proxy: {
      "/websocket": {
        target: "ws://" + espIpAddress,
        changeOrigin: true,
        secure: false,
        ws: true
      },
      "/api": {
        target: "http://" + espIpAddress, // IP address for the ESP web server
        changeOrigin: true,
        secure: false,
      }
    }
  },
  build: {
    rollupOptions: {
      output: {
        assetFileNames: "[name]-[hash:5][extname]" // limit filename length for SPIFFS support
      }
    },
    outDir: "../main/storage/",
    emptyOutDir: true
  }
});
