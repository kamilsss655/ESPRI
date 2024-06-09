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
