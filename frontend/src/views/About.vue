<template>
  <div>
    <q-card flat bordered>
      <q-card-section>
        <div class="text-h4 text-center">About</div>
      </q-card-section>

      <q-card-section class="q-pt-none">
        As such following features would be enabled:

    time domain scrambling (based on the HEKTOR 128 project)
    custom digital modes (modulated and demodulated by the ESP chip)
        more advanced messaging
            could be served as webpage by the esp chip itself
            or have a mobile client as Meshtastic
    wireless UART connection to the radio
    turning any radio into IoT device via JSON API served from the ESP

The physicial inteface layer between the ESPRI and the host radio will be through the Kenwood connector.

The audio input and output (for the means of time domain scrambling) would be through the Kenwood connector.
      </q-card-section>
    </q-card>
  </div>
</template>