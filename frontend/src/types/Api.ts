export enum ApiPaths {
  Settings = "/api/settings",
  SystemInfo = "/api/system/info",
  Event = "/api/event",
  Reboot = "/api/system/reboot",
  Record = "/api/audio/record",
  AudioSend = "/api/audio/send",
  TransmitWAV = "/api/audio/transmit_wav",
  DeepSleep = "/api/system/deep_sleep",
  FactoryReset = "/api/system/factory_reset",
  FileUpload = "/upload",
  FileDelete = "/delete"
}

export interface ApiResponse {
  data: {
    response: string;
  };
}
