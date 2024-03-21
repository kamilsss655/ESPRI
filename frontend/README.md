# Vue 3 + TypeScript + Vite + Quasar.dev

This is a front-end application used for the ESPRI project.

## Commands

1. Install

```
npm install
```

2. Run dev server

```
npm run dev
```

3. Build front-end (automatically copied to storage on ESP)

```
npm run build
```

## ESP server proxy

You can make requests to ESP on your network in front-end dev mode by editing `vite.config.ts` file:

```
  // Setup proxy to allow making requests to ESP in dev mode
  server: {
    proxy: {
      "/api": {
        target: "http://192.168.4.1", // IP address for the ESP web server
        changeOrigin: true,
        secure: false,
        ws: true
      }
    }
  },
```

## Recommended IDE Setup

- [VS Code](https://code.visualstudio.com/) + [Volar](https://marketplace.visualstudio.com/items?itemName=Vue.volar) (and disable Vetur) + [TypeScript Vue Plugin (Volar)](https://marketplace.visualstudio.com/items?itemName=Vue.vscode-typescript-vue-plugin).

## Type Support For `.vue` Imports in TS

TypeScript cannot handle type information for `.vue` imports by default, so we replace the `tsc` CLI with `vue-tsc` for type checking. In editors, we need [TypeScript Vue Plugin (Volar)](https://marketplace.visualstudio.com/items?itemName=Vue.vscode-typescript-vue-plugin) to make the TypeScript language service aware of `.vue` types.

If the standalone TypeScript plugin doesn't feel fast enough to you, Volar has also implemented a [Take Over Mode](https://github.com/johnsoncodehk/volar/discussions/471#discussioncomment-1361669) that is more performant. You can enable it by the following steps:

1. Disable the built-in TypeScript Extension
   1. Run `Extensions: Show Built-in Extensions` from VSCode's command palette
   2. Find `TypeScript and JavaScript Language Features`, right click and select `Disable (Workspace)`
2. Reload the VSCode window by running `Developer: Reload Window` from the command palette.
