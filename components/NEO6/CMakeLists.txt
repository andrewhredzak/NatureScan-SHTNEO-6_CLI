idf_component_register(SRCS "NEO6.c"
                    INCLUDE_DIRS "." "../unicast"
                    PRIV_REQUIRES esp_driver_uart unicast)