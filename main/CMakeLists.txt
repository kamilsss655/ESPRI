set(srcs
    "main.c"
    "app/button.c"
    "external/printf/printf.c"
    "hardware/button.c"
    "hardware/led.c"
    "hardware/uart.c"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")