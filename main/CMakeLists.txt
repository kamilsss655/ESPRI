set(srcs
    "main.c"
    "drivers/uart.c"
    "hardware/led.c"
    "hardware/button.c"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")