export interface Recorder {
  filepath: string,
  duration_seconds: number
  duration_minutes: number
  duration_hours: number
}

export interface RecordParam {
  filepath: string,
  duration_sec: number
}