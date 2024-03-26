# include source files
set(srcs
    "board.c"
    "main.c"
    "settings.c"
    "app/button.c"
    "app/morse_code.c"
    "app/uvk5.c"
    "external/printf/printf.c"
    "external/ws2812b/esp_ws28xx.c"
    "hardware/button.c"
    "hardware/led.c"
    "hardware/http_server.c"
    "hardware/uart.c"
    "hardware/audio.c"
    "hardware/wifi.c"
    "hardware/ws2812b.c"
    "hardware/spiffs.c"
    "hardware/ptt.c"
    "helper/api.c"
    "web/router.c"
    "web/handlers/root.c"
    "web/handlers/static_files.c"
    "web/handlers/api/event.c"
    "web/handlers/api/uvk5_message.c"
    "web/handlers/api/settings.c"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")

# mount /storage folder at /storage partition
spiffs_create_partition_image(storage storage FLASH_IN_PROJECT)