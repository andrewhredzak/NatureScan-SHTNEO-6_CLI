#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "NEO6.h"







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
    printf("                      ░██░░░░░░▒█▓                       \n");
    printf("                    ░██░░░░░░░░░░░░                    \n");
    printf("                ▒█▒░░░░░░░▓████░▓▓▓░█░                \n");
    printf("               ▒█▒░░░░░░░▒██░███░▒▒▓▒░░▒               \n");
    printf("             ░▒█▒░░░░░░░▒██░░░███▓▓░░░░░▓▒              \n");
    printf("         ░██░░░▒▓▓▓▒▒██▓░░░░░░░░▒███▒░░░░░█░         \n");
    printf("        ░█▓░░░▒▓▓▓▓▒▓▓▓░▒▒▒▒▒▒▒▒▒▓▓██░░░░░░░▓░        \n");
    printf("       ▒█▓░░░░▒▓▓▓▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▓░░░░░░░░░       \n");
    printf("      ░█▓░░░░░░▒▒▓▓▓▒▒░░░░░░░░░░░░░▒▒▓▓▓░░░░░░░░      \n");
    printf("      ▒█▒░░░░░▓▓▒▒░░░░░░░░░░░░░░░░░░░░░▒▒▓▒░░░░░░      \n");
    printf("      ░▓█▒░░░▒░░░██▒▒█▒█▓▒█▒▒██░▓█▒█▒█▓▒░░▒░░░░▒░░░█▒       \n");
    printf("         ▒█▓░░░░█░░█░▒▒▓▒░▓▓▒▓░▓░▓░▓▒▓▓▒░░░░░░░░░░░░▒█▓░         \n");
    printf("          ░██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░▒▒▒▒▓░          \n");
    printf("            ▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░            \n");
    printf("                                                          \n");
    printf("      ✴       ✴ <<<MADMANINDUSTRIES>>> ✴     ✴          \n");

    // Entry POINT:


    const uart_port_t uart_num = UART_NUM;
    uart_init(uart_num);


    //while(1)
    //vTask(free rtos call )
    







}
