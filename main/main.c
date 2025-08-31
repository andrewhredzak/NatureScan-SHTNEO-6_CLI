#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

// neo6 import
#include "driver/uart.h"
#include "NEO6.h"

//sht31 import 
#include "driver/i2c_master.h"
#include "SHT31.h"  


// unicast import
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "unicast.h"



#define GPS_TASK_STACK_SIZE (4096)  // Increased stack size
#define GPS_TASK_PRIORITY    (5)    // Medium priority
#define SHT31_TASK_PRIORITY  (4)    // Medium priority


static const char *TAG = "MAIN APP-NEO6";   // tag for main code 






void app_main(void){

    printf("successful flash and build!\n");
    ESP_LOGI(TAG, "START NEO-6 SCAN");

    printf("▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒░░░░░░░ ✴ ░░░░░░░▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓                        \n");
    printf("  _   __           _    ✴    ✴   ✴   ✴                                 \n");
    printf(" | \\ | |  ____   | |     ✴   ✴  ✴ ✴  ✴   ✴                           \n");
    printf(" |  \\| | / _` | |_ _|  _   __  ___   _____                               \n");
    printf(" | |\\  || (▒▒ |  | |  | | | | | ,_) / /__//                              \n");
    printf(" |_| \\_|\\__,_|  |_|  \_\_/_/ |_|   \ \___/                              \n");
    printf("                                                                          \n");
    printf("     ___     _     _    _   _ _   _ ___ _______                           \n"); 
    printf("  ///   \\  ______     _____      ____   ____       _ _    _ ___ ___      \n");
    printf("   \\\\    / /  \ \   \\ \\ |   //__ \\ //__ \\   _____  ___              \n");
    printf("  ___\\\\  \ \    __  \ ▒▒\\ |  | | | | | | | |  / /__// | ,_)            \n");
    printf(" [___/___/  \_\__/_/   \\_\\|_| |_| |_| |_| |_|  \ \___/ |_|              \n");
    printf("                                                                          \n");
    printf("      ░██▒▒▒▒▒                   ░██▒▒▒▒▒                 \n");
    printf("           ▒▓░░░               ▒▓░░░                 \n");
    printf("         ▒▒     ▒▒      ▒▒    ▒▒    ▒▒    ▒▒       ▒▒       ▒▒     ▒▒    ▒▒    \n");
    printf("      ✴       ✴ <<<MADMANINDUSTRIES>>> ✴     ✴           \n");

    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    // Entry POINT:
   

    /*

    // sht init
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    i2c_master_init(&bus_handle, &dev_handle);
    float temperature, humidity;
    ESP_LOGI(TAG, "I2C initialized successfully");

 
    // calling sht function:
    sht31_task_params_t* params = malloc(sizeof(sht31_task_params_t));
    params->dev_handle = dev_handle;
    params->temperature = &temperature;
    params->humidity = &humidity;


    const uart_port_t uart_num = UART_NUM;
    uart_init(uart_num);


    // Probe the sensor to check if it is connected to the bus with a 10ms timeout
    esp_err_t err = i2c_master_probe(bus_handle, SHT31_I2C_ADDR_0X44, 200);


    // SHT31 temp sensor task
    if(err == ESP_OK) {
        ESP_LOGI(TAG, "SHT31 sensor found");
        //xTaskCreate(sht4x_read_task, "sht4x_read_task", 4096, NULL, 5, NULL);
        if (xTaskCreate(SHT31TAKEDATA_task, "SHT31_TASK", 2048, (void*)params, 5, NULL) != pdPASS) {
            ESP_LOGE("MAIN", "failed to create sht31 task");
            free(params);
    }

    } else {
        ESP_LOGE(TAG, "SHT31 sensor not found");
        //sht4x_device_delete(sht4x_handle);
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
        ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
        ESP_LOGI(TAG, "I2C de-initialized successfully");
    }


    // launch NEO-6 GPS task
    xTaskCreate(gps_task,"gps_task",GPS_TASK_STACK_SIZE,(void*)UART_NUM,GPS_TASK_PRIORITY,NULL);

    */

    // Initialize WiFi and ESPNOW
    example_wifi_init();
    example_espnow_init();

    // launch unicast 
    unicast_print_test();

    //adding change 1
    // change 2
    


}
