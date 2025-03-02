#include <stdio.h>
#include <string.h>
#include "NEO6.h"
#include "esp_log.h"
#include "driver/uart.h"






void uart_init(handle *uarthandle);{

const uart_port_t uart_num = UART_NUM_2;
uart_config_t uart_config = {
    .baud_rate = CONFIG BAUD****,
    .data_bits = CONFIG****,
    .parity = CONFIG****,
    .stop_bits = CONFIG****,
    .flow_ctrl = CONFIG****,
    .rx_flow_ctrl_thresh = CONFIG****,
};
// Configure UART parameters
ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    
}



