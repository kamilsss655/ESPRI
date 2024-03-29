import { defineStore } from "pinia";

let connection: WebSocket;
const maxMessagesStored = 100;

export const useWebsocketStore = defineStore({
  id: "websocket",
  state: () => ({
    connected: false,
    lastSeenTimestamp: new Date().getTime(),
    messages: Array()
  }),
  actions: {
    disconnect() {
      connection.close();
    },
    connect() {
      connection = new WebSocket(
        (window.location.protocol === "https:" ? "wss://" : "ws://") +
          window.location.host +
          "/websocket"
      );

      connection.onmessage = (event) => {
        if (this.messageCount >= maxMessagesStored) {
          this.clearMessages();
        } else {
          this.$state.messages.push(JSON.parse(event.data));
        }
        this.updateLastSeenTimestamp();
      };

      connection.onerror = (event) => {
        this.$state.connected = false;
        console.log(event);
        connection.close();
      };

      connection.onopen = (_event) => {
        this.$state.connected = true;
      };

      connection.onclose = (event) => {
        this.$state.connected = false;
        console.log(
          "Socket is closed. Reconnect will be attempted in 1 second.",
          event.reason
        );
        connection.close();
      };
    },
    clearMessages() {
      this.$state.messages = [];
    },
    updateLastSeenTimestamp() {
      this.$state.lastSeenTimestamp = new Date().getTime();
    }
  },
  getters: {
    messageCount: (state) => state.messages.length
  }
});