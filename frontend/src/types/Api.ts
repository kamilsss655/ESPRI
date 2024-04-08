export enum ApiPaths {
  Settings = "/api/settings",
  Event = "/api/event",
  Reboot = "/api/system/reboot",
  DeepSleep = "/api/system/deep_sleep",
  FactoryReset = "/api/system/factory_reset"
}

export interface ApiResponse {
  data: {
    response: string;
  };
}
