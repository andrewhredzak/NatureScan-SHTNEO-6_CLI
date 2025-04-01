idf_component_register(SRCS "main.c"
                    PRIV_REQUIRES esp_driver_uart NEO6 esp_driver_i2c SHT31
                    INCLUDE_DIRS ".")
