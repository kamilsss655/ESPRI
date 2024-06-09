export enum WifiMode {
  OFF,
  AP,
  STA
}

export enum BeaconMode {
  OFF,
  MORSE_CODE,
  AFSK,
  WAV
}

export interface Settings {
  "wifi.mode": WifiMode;
  "wifi.ssid": string;
  "wifi.password": string;
  "wifi.channel": number;
  "wifi.max_connections": number;
  "gpio.status_led": number;
  "gpio.audio_in": number;
  "gpio.audio_out": number;
  "gpio.ptt": number;
  "audio.out.volume": number;
  "audio.in.squelch": number;
  "led.max_brightness": number;
  "beacon.mode": BeaconMode;
  "beacon.text": string;
  "beacon.delay_seconds": number;
  "beacon.morse_code.baud": number;
  "beacon.morse_code.tone_freq": number;
  "beacon.afsk.baud": number;
  "beacon.afsk.zero_freq": number;
  "beacon.afsk.one_freq": number;
  "beacon.wav.filepath": string;
}
