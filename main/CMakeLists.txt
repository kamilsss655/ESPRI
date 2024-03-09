# include source files
set(srcs
    "board.c"
    "main.c"
    "app/button.c"
    "app/uvk5.c"
    "external/printf/printf.c"
    "hardware/button.c"
    "hardware/led.c"
    "hardware/http_server.c"
    "hardware/uart.c"
    "hardware/wifi.c"
    "hardware/spiffs.c"
    "web/router.c"
    "web/handlers/root.c"
    "web/handlers/static_files.c"
    "web/handlers/api/event.c"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")

# mount /storage folder at /storage partition
spiffs_create_partition_image(storage storage FLASH_IN_PROJECT)