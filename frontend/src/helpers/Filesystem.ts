import axios from "axios";
import { ApiPaths, ApiResponse } from "../types/Api";
import { Notify } from "quasar";

// Make API request to delete a file
export function deleteFile(filepath: string)
{
  axios
  .delete(ApiPaths.FileDelete + filepath, {
    headers: {
      "Content-Type": "application/json"
    }
  })
  .then((response: ApiResponse) => {
    console.log(response.data);
  
    Notify.create({
      message: response.data.response,
      color: "positive"
    });
  })
  .catch((error) => {
    console.error(error);
    if (error.response) {
      let response: ApiResponse = error.response;
  
      Notify.create({
        message: response.data.response,
        color: "negative"
      });
    }
  });
}