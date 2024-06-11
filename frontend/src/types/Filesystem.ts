export enum FilesystemBasePath {
  Flash = "/storage",
  SdCard = "/sd"
}

export interface File {
  name: string;
  size: string;
}

export interface Directory {
  name: string;
}

export interface Listing {
  files: File[];
  directories: Directory[];
}

export interface StoragePath {
  prefix :string;
  path :string;
}