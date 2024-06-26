import { defineStore } from "pinia";
import { useSystemStore } from "./system";
import WebsocketMessage from "../types/WebsocketMessage";

let connection: WebSocket;
const maxMessagesStored = 100;

export const useWebsocketStore = defineStore({
  id: "websocket",
  state: () => ({
    connected: false,
    lastSeenTimestamp: new Date().getTime(),
    messages: [] as WebsocketMessage[]
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
        if (this.messageCountMaxLimitReached) {
          this.shiftMessages();
        }
        let jsonMessage = JSON.parse(event.data);
        const systemStore = useSystemStore();
        systemStore.handle(jsonMessage);
        this.storeMessage(jsonMessage);
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
    storeMessage(message: { [index: string]: any }) {
      const websocketMessage = {} as WebsocketMessage;

      websocketMessage.tag = message["tag"];
      websocketMessage.message = message["message"];
      websocketMessage.timestamp = Date.now();

      this.$state.messages.push(websocketMessage);
    },
    shiftMessages() {
      this.$state.messages.shift();
    },
    updateLastSeenTimestamp() {
      this.$state.lastSeenTimestamp = new Date().getTime();
    }
  },
  getters: {
    messageCount: (state) => state.messages.length,
    messageCountMaxLimitReached: (state) =>
      state.messages.length >= maxMessagesStored
  }
});
