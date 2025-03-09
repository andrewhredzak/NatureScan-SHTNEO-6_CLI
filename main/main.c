#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "NEO6.h"

#define GPS_TASK_STACK_SIZE (4096)  // Increased stack size
#define GPS_TASK_PRIORITY    (5)    // Medium priority


static const char *TAG = "MAIN APP-NEO6";   // tag for main code 






void app_main(void){

    printf("successful flash and build!\n");
    ESP_LOGI(TAG, "START NEO-6 SCAN");

    printf("                                                         \n");
    printf("                          ✴✴                             \n");
    printf("                           ░                           \n");
    printf("                         ░█▓█░                          \n");
    printf("                         ▓█▒░▒█▒                         \n");
    printf("                       ░▓█▒░░░▒█▒                        \n");
    printf("                      ▒█▒█▓▒█▒▒██░                 \n");
    printf("                       ▒█▓░░░░▓▓▓              \n");
    printf("                        ░██▒▒▒▒▒                 \n");
    printf("                          ▒▓░░░                 \n");
    printf("                            ▒▒                               \n");
    printf("      ✴       ✴ <<<MADMANINDUSTRIES>>> ✴     ✴           \n");

    // Entry POINT:


    const uart_port_t uart_num = UART_NUM;
    uart_init(uart_num);


    // Launch GPS task
    xTaskCreate(
        gps_task,              // Task function
        "gps_task",           // Task name
        GPS_TASK_STACK_SIZE,  // Stack size in words
        (void*)UART_NUM,      // Parameters
        GPS_TASK_PRIORITY,    // Priority
        NULL                  // Task handle
    );
    







}
