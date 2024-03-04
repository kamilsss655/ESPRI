set(srcs
    "main.c"
    "hardware/led.c"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")