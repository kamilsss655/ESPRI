export enum ApiPaths {
  Settings = "/api/settings",
  SystemInfo = "/api/system/info",
  Event = "/api/event",
  Reboot = "/api/system/reboot",
  Record = "/api/audio/record",
  TransmitWAV = "/api/audio/transmit_wav",
  DeepSleep = "/api/system/deep_sleep",
  FactoryReset = "/api/system/factory_reset",
  FileUpload = "/upload"
}

export interface ApiResponse {
  data: {
    response: string;
  };
}
