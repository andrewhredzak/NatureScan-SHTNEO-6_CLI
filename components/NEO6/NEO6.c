#include <stdio.h>
#include <string.h>
#include "NEO6.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "NEO6.c";  //tag for source code

void uart_init(uart_port_t uart_num) {
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
    
    ESP_LOGI(TAG, "UART Configuration:");
    ESP_LOGI(TAG, "- Port: UART%d", uart_num);
    ESP_LOGI(TAG, "- Baud Rate: %d", BAUD_RATE);
    ESP_LOGI(TAG, "- Data Bits: %d", DATA_BITS);
    ESP_LOGI(TAG, "- Stop Bits: %d", UART_STOP_BITS);
    ESP_LOGI(TAG, "- Parity: %d", UART_PARITY);
    ESP_LOGI(TAG, "- Flow Control: %d", UART_HW_FLOWCTRL);
    ESP_LOGI(TAG, "- TX Pin: %d", TX_PIN);
    ESP_LOGI(TAG, "- RX Pin: %d", RX_PIN);
    
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_LOGI(TAG, "UART parameter configuration done.");

    // Set UART pins(TX: I15, RX: I14, RTS: IO18, CTS: IO19)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_LOGI(TAG, "UART pin configuration done."); // Setup UART buffered IO with event queue

    ESP_ERROR_CHECK(uart_driver_install(uart_num, RX_BUFFER_SIZE, 0, 0, NULL, 0));
    ESP_LOGI(TAG, "UART driver installed.");
}

void gps_event_handler(uint8_t *data, uint16_t len) {
    // Null-terminate the data for safe printing
    if (len >= RX_BUFFER_SIZE) {
        len = RX_BUFFER_SIZE - 1; // Prevent overflow
    }
    data[len] = '\0';

    // Log the raw NMEA sentence to console (UART1)
    ESP_LOGI(TAG, "GPS Data: %s", (char *)data);
}

void gps_task(uart_port_t uart_num) {
    uint8_t buffer[RX_BUFFER_SIZE];

    while (1) {
        // Read data from UART1
        //ESP_LOGI(TAG, "uart_num:%d", uart_num);
        int len = uart_read_bytes(uart_num, buffer, RX_BUFFER_SIZE - 1, pdMS_TO_TICKS(100));
        //ESP_LOGI(TAG, "len:%d", len);
        if (len > 0) {
            // Check for UART read errors.
            if (len < 0){
                 ESP_LOGE(TAG, "UART read error: %d",len);
            } else {
                //ESP_LOGI(TAG, "in else.");
                gps_event_handler(buffer, len);
            }
          
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Changed to 100ms.
    }
}
