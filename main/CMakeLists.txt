set(srcs
    "main.c"
    "app/button.c"
    "hardware/button.c"
    "hardware/led.c"
    "hardware/uart.c"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")