import { ApiResponse } from "../types/Api";

// Convert JSON API response into ApiResponse
export function getApiResponseFromJson(jsonText: string): ApiResponse {
  if (jsonText === "") {
    return { data: { response: "" } };
  } else {
    return { data: { response: JSON.parse(jsonText).response } };
  }
}
