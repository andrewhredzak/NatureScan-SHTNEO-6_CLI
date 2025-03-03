#include <stdio.h>
#include <string.h>
#include "NEO6.h"
#include "esp_log.h"
#include "driver/uart.h"


static const char *TAG = "NEO6.c";  //tag for source code



void uart_init(uart_port_t uart_num){
/* **brief this function initializes UART configuration parameters. 
    Key configuration constants (e.g., data bits, parity) are provided by uart.h,
     while some values (e.g., baud rate) are user-defined. */
    uart_config_t uart_config = {
        
        .baud_rate = BAUD_RATE,
        .data_bits = DATA_BITS,
        .parity = UART_PARITY,
        .stop_bits = UART_STOP_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL,
        .rx_flow_ctrl_thresh = FLOW_CTRL_THRESH,
};
// Configure UART parameters
ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
ESP_LOGI(TAG, "UART parameter configuration done.");

// Set UART pins(TX: I15, RX: I14, RTS: IO18, CTS: IO19)
ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
ESP_LOGI(TAG, "UART pin configuration done.");// Setup UART buffered IO with event queue

ESP_ERROR_CHECK(uart_driver_install(uart_num, RX_BUFFER_SIZE, 0, 0, NULL, 0));
ESP_LOGI(TAG, "UART driver installed.");


    
}



