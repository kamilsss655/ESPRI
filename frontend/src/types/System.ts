export enum AudioState {
  OFF,
  LISTENING,
  RECEIVING,
  TRANSMITTING
}

export interface SystemInfo {
  "heap.total": number;
  "heap.free": number;
  "heap.min_free": number;
  "storage.used": number;
  "storage.total": number;
  "uptime": number;
  "version": string;
}

export interface System {
  rebootRequired: Boolean,
  audioState: AudioState,
  audioSending: Boolean,
  audioRecordingActive: Boolean,
  info: SystemInfo,
  systemInfoLiveUpdate: Boolean
}




