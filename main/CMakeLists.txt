# include source files
set(srcs
    "board.c"
    "main.c"
    "settings.c"
    "system.c"
    "app/button.c"
    "app/morse.c"
    "app/beacon.c"
    "app/transmit.c"
    "app/uvk5.c"
    "dsp/filter.c"
    "dsp/agc.c"
    "external/printf/printf.c"
    "hardware/button.c"
    "hardware/led.c"
    "hardware/http_server.c"
    "hardware/uart.c"
    "hardware/audio.c"
    "hardware/wifi.c"
    "hardware/sd.c"
    "hardware/spiffs.c"
    "hardware/ptt.c"
    "helper/api.c"
    "helper/filesystem.c"
    "web/router.c"
    "web/handlers/root.c"
    "web/handlers/websocket.c"
    "web/handlers/static_files.c"
    "web/handlers/api/audio.c"
    "web/handlers/api/event.c"
    "web/handlers/api/uvk5_message.c"
    "web/handlers/api/settings.c"
    "web/handlers/api/system.c"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")

# mount /storage folder at /storage partition
spiffs_create_partition_image(storage storage FLASH_IN_PROJECT)