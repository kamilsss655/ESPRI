import { defineStore } from "pinia";
import { useSystemStore } from "./system";
import WebsocketMessage from "../types/WebsocketMessage";
import { useAudioStore } from "../stores/audio";

let connection: WebSocket;
const maxMessagesStored = 100;
let audioStore: ReturnType<typeof useAudioStore> | null = null;

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
      audioStore = useAudioStore();
      connection.binaryType = 'arraybuffer';

      connection.onmessage = (event) => {
        if (typeof event.data === 'string' || event.data[0]=='{') {
          if (this.messageCountMaxLimitReached) {
            this.shiftMessages();
          }

          let jsonMessage = JSON.parse(event.data);
          const systemStore = useSystemStore();
          systemStore.handle(jsonMessage);
          this.storeMessage(jsonMessage);
          this.updateLastSeenTimestamp();
        } else {
          // TODO: process binary (audio) data
          // If we have a processor, send the data to it
          if (window.pcmProcessor==null) return;
          const dataView = new DataView(event.data);
          const newBuffer = new Float32Array(event.data.byteLength);

          let sumOfSquares=0;
          // Transform to -1/1 float audio
          // Assuming 8-bit PCM, little-endian
          for (let i = 0; i < newBuffer.length; i++) {
            newBuffer[i] = dataView.getInt8(i) / 128;
            sumOfSquares+=newBuffer[i] ** 2;
          }

          // This disabled for now... its vanilla JS
          // Calculate the avg power to show on the meter
          if (audioStore != null) {
            const avgPowerDecibels = 10 * Math.log10(sumOfSquares / newBuffer.length);
            if (isFinite(avgPowerDecibels)) audioStore.setAudioLevel((100+avgPowerDecibels)/100);
          }
          
          /*
          if (recordActive) {
            let tempBuffer = new Int16Array(completeAudioBuffer.length + newBuffer.length);
            tempBuffer.set(completeAudioBuffer, 0);
            tempBuffer.set(newBuffer, completeAudioBuffer.length);
            // Used for local audio recording TODO
            //completeAudioBuffer = tempBuffer;
          }
          */

          window.pcmBuffer.push(newBuffer);
          if (window.pcmProcessor!=null) {
            window.pcmProcessor.port.postMessage('dataAvailable');
          } else {
            console.error('pcmProcessor is null');
          }
        }


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
