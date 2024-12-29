class PCMProcessor extends AudioWorkletProcessor {
    constructor() {
      super();
      this.buffer = new Float32Array(0);
      this.port.onmessage = (event) => {
        if (event.data instanceof Float32Array) {
          const newBuffer = event.data;
          const tempBuffer = new Float32Array(this.buffer.length + newBuffer.length);
          tempBuffer.set(this.buffer, 0);
          tempBuffer.set(newBuffer, this.buffer.length);
          this.buffer = tempBuffer;
        } else if (event.data === 'dataAvailable') {
          this.port.postMessage('request');
        }
      };
    }
  
    process(inputs, outputs, parameters) {
      const output = outputs[0];
      const outputChannel = output[0];
  
      if (this.buffer.length >= outputChannel.length) {
        outputChannel.set(this.buffer.subarray(0, outputChannel.length));
        this.buffer = this.buffer.subarray(outputChannel.length);
      } else {
        outputChannel.set(this.buffer);
        this.buffer = new Float32Array(0);
      }
      this.port.postMessage('request');
  
      return true;
    }
  }
  
  registerProcessor('pcm-processor', PCMProcessor);
  