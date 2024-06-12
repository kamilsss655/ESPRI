import axios from "axios";
import { Notify } from "quasar";
import { ApiPaths, ApiResponse } from "../types/Api";
import { TransmitParam } from "../types/Transmit";

// Make API request to transmit WAV file
export function transmitWAV(filepath: string)
{
  const axiosInstance = axios.create();
  axiosInstance.defaults.timeout = 600;

  const param: TransmitParam = 
  {
    filepath: filepath
  }

  const jsonData = JSON.stringify(param);

  axiosInstance
    .put(ApiPaths.TransmitWAV, jsonData, {
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